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

from veles.messages import definitions
from veles.messages import msgpackwrap
from veles.schema import model
from .srv import BaseLister


class ProtocolError(Exception):
    pass


class GetSub:
    def __init__(self, conn, qid, obj):
        self.conn = conn
        self.qid = qid
        self.obj = obj

    def kill(self):
        self.obj.remove_sub(self)

    def obj_changed(self):
        self.conn.send_obj_reply(self.qid, self.obj)

    def obj_gone(self):
        if self.qid in self.conn.subs:
            del self.conn.subs[self.qid]
        self.conn.send_obj_gone(self.qid)


class GetDataSub:
    def __init__(self, conn, qid, obj, key):
        self.conn = conn
        self.qid = qid
        self.obj = obj
        self.key = key

    def kill(self):
        self.obj.remove_data_sub(self)

    def data_changed(self, data):
        self.conn.send_obj_data_reply(self.qid, data)

    def obj_gone(self):
        if self.qid in self.conn.subs:
            del self.conn.subs[self.qid]
        self.conn.send_obj_gone(self.qid)


class Lister(BaseLister):
    def __init__(self, conn, qid, srv, obj, pos, tags):
        self.conn = conn
        self.qid = qid
        super().__init__(srv, obj, pos, tags)

    def list_changed(self, new, gone):
        self.conn.send_list_reply(self.qid, new, gone)

    def obj_gone(self):
        if self.qid in self.conn.subs:
            del self.conn.subs[self.qid]
        self.conn.send_obj_gone(self.qid)


class Proto(asyncio.Protocol):
    def __init__(self, srv):
        self.srv = srv
        self.subs = {}

    def connection_made(self, transport):
        self.transport = transport
        self.cid = self.srv.new_conn(self)
        wrapper = msgpackwrap.MsgpackWrapper()
        self.unpacker = wrapper.unpacker
        self.packer = wrapper.packer

    def data_received(self, data):
        self.unpacker.feed(data)
        while True:
            try:
                msg = model.Model.load(self.unpacker.unpack())
                if not isinstance(msg, definitions.MsgpackMsg):
                    raise ValueError(
                        'received an object that isn\'t a message')
                self.handle_msg(msg)
            except msgpack.OutOfData:
                return

    def handle_msg(self, msg):
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
            handlers[msg.object_type](msg)
        except ProtocolError as e:
            self.protoerr(e.args[0], e.args[1])

    def protoerr(self, code, msg):
        self.send_msg(definitions.MsgProtoError(
            code=code,
            error=msg,
        ))

    def connection_lost(self, ex):
        for qid, sub in self.subs.items():
            sub.kill()
        self.srv.remove_conn(self)

    def send_msg(self, msg):
        self.transport.write(msg.dump(self.packer))

    def msg_create(self, msg):
        self.srv.create(
            msg.id,
            msg.parent,
            tags=msg.tags,
            attr=msg.attr,
            data=msg.data,
            bindata=msg.bindata,
            pos=(msg.pos_start, msg.pos_end),
        )
        if msg.rid is not None:
            self.send_msg(definitions.MsgAck(
                rid=msg.rid,
            ))

    def msg_modify(self, msg):
        # XXX
        raise NotImplementedError

    def msg_delete(self, msg):
        objs = msg.ids
        for obj in objs:
            self.srv.delete(obj)
        if msg.rid is not None:
            self.send_msg(definitions.MsgAck(
                rid=msg.rid,
            ))

    def msg_list(self, msg):
        qid = msg.qid
        if qid in self.subs:
            raise ProtocolError('qid_in_use', 'qid already in use')
        tagsets = msg.tags
        lister = Lister(self, qid, self.srv, msg.parent,
                        (msg.pos_start, msg.pos_end), tagsets)
        self.srv.run_lister(lister, msg.sub)
        if msg.sub:
            self.subs[qid] = lister

    def msg_get(self, msg):
        qid = msg.qid
        if qid in self.subs:
            raise ProtocolError('qid_in_use', 'qid already in use')
        obj = self.srv.get(msg.id)
        if obj is None:
            self.send_msg(definitions.MsgObjGone(
                qid=qid,
            ))
        else:
            self.send_obj_reply(qid, obj)
            if msg.sub:
                sub = GetSub(self, qid, obj)
                self.subs[qid] = sub
                obj.add_sub(sub)

    def msg_get_data(self, msg):
        obj = msg.id
        sub = msg.sub
        qid = msg.qid
        key = msg.key
        if qid in self.subs:
            raise ProtocolError('qid_in_use', 'qid already in use')
        obj = self.srv.get(obj)
        if obj is None:
            self.send_msg(definitions.MsgObjGone(
                qid=qid,
            ))
        else:
            data = self.srv.get_data(obj, key)
            self.send_obj_data_reply(qid, data)
            if sub:
                sub = GetDataSub(self, qid, obj, key)
                self.subs[qid] = sub
                obj.add_data_sub(sub)

    def msg_get_bin(self, msg):
        # XXX
        raise NotImplementedError

    def msg_unsub(self, msg):
        qid = msg.qid
        if qid in self.subs:
            self.subs[qid].kill()
            del self.subs[qid]
        self.send_msg(definitions.MsgSubCancelled(
            qid=qid,
        ))

    def msg_mthd_run(self, msg):
        # XXX
        raise NotImplementedError

    def msg_mthd_done(self, msg):
        # XXX
        raise NotImplementedError

    def msg_proc_done(self, msg):
        # XXX
        raise NotImplementedError

    def msg_mthd_reg(self, msg):
        # XXX
        raise NotImplementedError

    def msg_proc_reg(self, msg):
        # XXX
        raise NotImplementedError

    def send_obj_data_reply(self, qid, data):
        self.send_msg(definitions.MsgGetDataReply(qid=qid, data=data))

    def send_obj_reply(self, qid, obj):
        self.send_msg(definitions.MsgGetReply(
            qid=qid,
            obj=obj.node,
        ))

    def send_list_reply(self, qid, new, gone):
        self.send_msg(definitions.MsgListReply(
            objs=[obj.node for obj in new],
            qid=qid,
            gone=gone
        ))

    def send_obj_gone(self, qid):
        self.send_msg(definitions.MsgObjGone(
            qid=qid,
        ))


@asyncio.coroutine
def unix_server(srv, path):
    return (yield from srv.loop.create_unix_server(lambda: Proto(srv), path))


@asyncio.coroutine
def tcp_server(srv, ip, port):
    return (yield from srv.loop.create_server(lambda: Proto(srv), ip, port))
