# Copyright 2017 CodiLime
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Standing in the rain
# The cold and angry rain
# In a long white dress
# A girl without a name

import asyncio

import msgpack

from veles.proto import messages, msgpackwrap
from veles.async_conn.subscriber import (
    BaseSubscriber,
    BaseSubscriberData,
    BaseSubscriberList,
)
from veles.proto.exceptions import VelesException


class ProtocolError(Exception):
    pass


class Subscriber(BaseSubscriber):
    def __init__(self, obj, proto, qid):
        self.proto = proto
        self.qid = qid
        super().__init__(obj)

    def object_changed(self):
        self.proto.send_msg(messages.MsgGetReply(
            qid=self.qid,
            obj=self.obj.node
        ))

    def error(self, err):
        self.proto.send_msg(messages.MsgQueryError(
            qid=self.qid,
            code=err.code,
            msg=err.msg,
        ))


class SubscriberData(BaseSubscriberData):
    def __init__(self, obj, key, proto, qid):
        self.proto = proto
        self.qid = qid
        super().__init__(obj, key)

    def data_changed(self, data):
        self.proto.send_msg(messages.MsgGetDataReply(
            qid=self.qid,
            data=data
        ))

    def error(self, err):
        self.proto.send_msg(messages.MsgQueryError(
            qid=self.qid,
            code=err.code,
            msg=err.msg,
        ))


class SubscriberList(BaseSubscriberList):
    def __init__(self, obj, tags, pos_filter, proto, qid):
        self.proto = proto
        self.qid = qid
        super().__init__(obj, tags, pos_filter)

    def list_changed(self, changed, gone):
        self.proto.send_msg(messages.MsgGetListReply(
            objs=[obj.node for obj in changed],
            qid=self.qid,
            gone=gone
        ))

    def error(self, err):
        self.proto.send_msg(messages.MsgQueryError(
            qid=self.qid,
            code=err.code,
            msg=err.msg,
        ))


class ServerProto(asyncio.Protocol):
    def __init__(self, conn):
        self.conn = conn
        self.subs = {}

    def connection_made(self, transport):
        self.transport = transport
        self.cid = self.conn.new_conn(self)
        wrapper = msgpackwrap.MsgpackWrapper()
        self.unpacker = wrapper.unpacker
        self.packer = wrapper.packer

    def data_received(self, data):
        self.unpacker.feed(data)
        while True:
            try:
                msg = messages.MsgpackMsg.load(self.unpacker.unpack())
                loop = asyncio.get_event_loop()
                loop.create_task(self.handle_msg(msg))
            except msgpack.OutOfData:
                return

    async def handle_msg(self, msg):
        handlers = {
            'create': self.msg_create,
            'delete': self.msg_delete,
            'set_parent': self.msg_set_parent,
            'set_pos': self.msg_set_pos,
            'add_tag': self.msg_add_tag,
            'del_tag': self.msg_del_tag,
            'set_attr': self.msg_set_attr,
            'set_data': self.msg_set_data,
            'get': self.msg_get,
            'get_data': self.msg_get_data,
            'get_bin': self.msg_get_bin,
            'get_list': self.msg_get_list,
            'mthd_run': self.msg_mthd_run,
            'unsub': self.msg_unsub,
            'mthd_done': self.msg_mthd_done,
            'proc_done': self.msg_proc_done,
            'mthd_reg': self.msg_mthd_reg,
            'proc_reg': self.msg_proc_reg,
        }
        try:
            await handlers[msg.object_type](msg)
        except ProtocolError as e:
            self.protoerr(e.args[0], e.args[1])

    def protoerr(self, code, msg):
        self.send_msg(messages.MsgProtoError(
            code=code,
            error=msg,
        ))

    def connection_lost(self, ex):
        for qid, sub in self.subs.items():
            sub.cancel()
        self.conn.remove_conn(self)

    def send_msg(self, msg):
        self.transport.write(self.packer.pack(msg.dump()))

    async def do_request(self, msg, req):
        try:
            await req
        except VelesException as e:
            self.send_msg(messages.MsgRequestError(
                rid=msg.rid,
                code=e.code,
                msg=e.msg,
            ))
        else:
            self.send_msg(messages.MsgRequestAck(
                rid=msg.rid,
            ))

    async def msg_create(self, msg):
        await self.do_request(msg, self.conn.create(
            msg.id,
            msg.parent,
            tags=msg.tags,
            attr=msg.attr,
            data=msg.data,
            bindata=msg.bindata,
            pos=(msg.pos_start, msg.pos_end),
        )[1])

    async def msg_delete(self, msg):
        obj = self.conn.get_node_norefresh(msg.id)
        await self.do_request(msg, obj.delete())

    async def msg_set_parent(self, msg):
        obj = self.conn.get_node_norefresh(msg.id)
        await self.do_request(msg, obj.set_parent(msg.parent))

    async def msg_set_pos(self, msg):
        obj = self.conn.get_node_norefresh(msg.id)
        await self.do_request(msg, obj.set_pos(msg.pos_start, msg.pos_end))

    async def msg_add_tag(self, msg):
        obj = self.conn.get_node_norefresh(msg.id)
        await self.do_request(msg, obj.add_tag(msg.tag))

    async def msg_del_tag(self, msg):
        obj = self.conn.get_node_norefresh(msg.id)
        await self.do_request(msg, obj.del_tag(msg.tag))

    async def msg_set_attr(self, msg):
        obj = self.conn.get_node_norefresh(msg.id)
        await self.do_request(msg, obj.set_attr(msg.key, msg.data))

    async def msg_set_data(self, msg):
        obj = self.conn.get_node_norefresh(msg.id)
        await self.do_request(msg, obj.set_data(msg.key, msg.data))

    async def msg_get(self, msg):
        if msg.qid in self.subs:
            raise ProtocolError('qid_in_use', 'qid already in use')
        obj = self.conn.get_node_norefresh(msg.id)
        if not msg.sub:
            try:
                obj = await obj.refresh()
            except VelesException as e:
                self.send_msg(messages.MsgQueryError(
                    qid=msg.qid,
                    code=e.code,
                    msg=e.msg,
                ))
            else:
                self.send_msg(messages.MsgGetReply(
                    qid=msg.qid,
                    obj=obj.node,
                ))
        else:
            self.subs[msg.qid] = Subscriber(obj, self, msg.qid)

    async def msg_get_data(self, msg):
        if msg.qid in self.subs:
            raise ProtocolError('qid_in_use', 'qid already in use')
        obj = self.conn.get_node_norefresh(msg.id)
        if not msg.sub:
            try:
                data = await obj.get_data(msg.key)
            except VelesException as e:
                self.send_msg(messages.MsgQueryError(
                    qid=msg.qid,
                    code=e.code,
                    msg=e.msg,
                ))
            else:
                self.send_msg(messages.MsgGetDataReply(
                    qid=msg.qid,
                    data=data
                ))
        else:
            self.subs[msg.qid] = SubscriberData(obj, msg.key, self, msg.qid)

    async def msg_get_bin(self, msg):
        # XXX
        raise NotImplementedError

    async def msg_get_list(self, msg):
        if msg.qid in self.subs:
            raise ProtocolError('qid_in_use', 'qid already in use')
        parent = self.conn.get_node_norefresh(msg.parent)
        if not msg.sub:
            try:
                objs = await parent.get_list(msg.tags, msg.pos_filter)
            except VelesException as e:
                self.send_msg(messages.MsgQueryError(
                    qid=msg.qid,
                    code=e.code,
                    msg=e.msg,
                ))
            else:
                self.send_msg(messages.MsgGetListReply(
                    qid=msg.qid,
                    objs=[obj.node for obj in objs]
                ))
        else:
            self.subs[msg.qid] = SubscriberList(
                parent, msg.tags, msg.pos_filter, self, msg.qid)

    async def msg_unsub(self, msg):
        qid = msg.qid
        if qid in self.subs:
            self.subs[qid].cancel()
            del self.subs[qid]
        self.send_msg(messages.MsgSubCancelled(
            qid=qid,
        ))

    async def msg_mthd_run(self, msg):
        # XXX
        raise NotImplementedError

    async def msg_mthd_done(self, msg):
        # XXX
        raise NotImplementedError

    async def msg_proc_done(self, msg):
        # XXX
        raise NotImplementedError

    async def msg_mthd_reg(self, msg):
        # XXX
        raise NotImplementedError

    async def msg_proc_reg(self, msg):
        # XXX
        raise NotImplementedError


async def create_unix_server(conn, path):
    return await conn.loop.create_unix_server(lambda: ServerProto(conn), path)


async def create_tcp_server(conn, ip, port):
    return await conn.loop.create_server(lambda: ServerProto(conn), ip, port)
