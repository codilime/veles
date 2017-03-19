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
from .conn import BaseLister
from veles.async_conn.subscriber import (
    BaseSubscriber,
)

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


class GetDataSub:
    def __init__(self, proto, qid, obj, key):
        self.proto = proto
        self.qid = qid
        self.obj = obj
        self.key = key

    def cancel(self):
        self.obj.remove_data_sub(self)

    def data_changed(self, data):
        self.proto.send_obj_data_reply(self.qid, data)

    def obj_gone(self):
        if self.qid in self.proto.subs:
            del self.proto.subs[self.qid]
        self.proto.send_obj_gone(self.qid)


class Lister(BaseLister):
    def __init__(self, proto, qid, conn, obj, pos, tags):
        self.proto = proto
        self.qid = qid
        super().__init__(conn, obj, pos, tags)

    def list_changed(self, new, gone):
        self.proto.send_list_reply(self.qid, new, gone)

    def obj_gone(self):
        if self.qid in self.proto.subs:
            del self.proto.subs[self.qid]
        self.proto.send_obj_gone(self.qid)


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
            'modify': self.msg_modify,
            'delete': self.msg_delete,
            'list': self.msg_list,
            'get': self.msg_get,
            'get_data': self.msg_get_data,
            'get_bin': self.msg_get_bin,
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

    async def msg_create(self, msg):
        self.conn.create(
            msg.id,
            msg.parent,
            tags=msg.tags,
            attr=msg.attr,
            data=msg.data,
            bindata=msg.bindata,
            pos=(msg.pos_start, msg.pos_end),
        )
        if msg.rid is not None:
            self.send_msg(messages.MsgAck(
                rid=msg.rid,
            ))

    async def msg_modify(self, msg):
        # XXX
        raise NotImplementedError

    async def msg_delete(self, msg):
        objs = msg.ids
        for obj in objs:
            self.conn.delete(obj)
        if msg.rid is not None:
            self.send_msg(messages.MsgAck(
                rid=msg.rid,
            ))

    async def msg_list(self, msg):
        qid = msg.qid
        if qid in self.subs:
            raise ProtocolError('qid_in_use', 'qid already in use')
        lister = Lister(self, qid, self.conn, msg.parent,
                        msg.pos_filter, msg.tags)
        self.conn.run_lister(lister, msg.sub)
        if msg.sub:
            self.subs[qid] = lister

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
        obj = msg.id
        sub = msg.sub
        qid = msg.qid
        key = msg.key
        if qid in self.subs:
            raise ProtocolError('qid_in_use', 'qid already in use')
        obj = self.conn.get_node_norefresh(obj)
        if obj is None:
            self.send_msg(messages.MsgObjGone(
                qid=qid,
            ))
        else:
            data = self.conn.get_data(obj, key)
            self.send_obj_data_reply(qid, data)
            if sub:
                sub = GetDataSub(self, qid, obj, key)
                self.subs[qid] = sub
                obj.add_data_sub(sub)

    async def msg_get_bin(self, msg):
        # XXX
        raise NotImplementedError

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

    def send_obj_data_reply(self, qid, data):
        self.send_msg(messages.MsgGetDataReply(qid=qid, data=data))

    def send_obj_reply(self, qid, obj):
        self.send_msg(messages.MsgGetReply(
            qid=qid,
            obj=obj.node,
        ))

    def send_list_reply(self, qid, new, gone):
        self.send_msg(messages.MsgListReply(
            objs=[obj.node for obj in new],
            qid=qid,
            gone=gone
        ))

    def send_obj_gone(self, qid):
        self.send_msg(messages.MsgObjGone(
            qid=qid,
        ))


async def create_unix_server(conn, path):
    return await conn.loop.create_unix_server(lambda: ServerProto(conn), path)


async def create_tcp_server(conn, ip, port):
    return await conn.loop.create_server(lambda: ServerProto(conn), ip, port)
