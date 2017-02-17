# Standing in the rain
# The cold and angry rain
# In a long white dress
# A girl without a name

import asyncio
import msgpack
from asrv.srv import BaseLister
from messages import messages


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
        self.packer = msgpack.Packer(use_bin_type=True)
        self.unpacker = msgpack.Unpacker(encoding='utf-8')

    def data_received(self, data):
        self.unpacker.feed(data)
        while True:
            try:
                msg = messages.MsgpackMsg.loads(self.unpacker)
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
            handlers[msg.msg_type](msg)
        except ProtocolError as e:
            self.protoerr(e.args[0], e.args[1])

    def protoerr(self, code, msg):
        self.send_msg({
            'type': 'protoerr',
            'code': code,
            'error': msg,
        })

    def connection_lost(self, ex):
        for qid, sub in self.subs.items():
            sub.kill()
        self.srv.remove_conn(self)

    def send_msg(self, msg):
        self.transport.write(self.packer.pack(msg))

    def getnint(self, msg, field):
        val = msg.get(field, None)
        if not isinstance(val, int) and val is not None:
            raise ProtocolError('type_error',
                                '{} is not int or None'.format(field))
        return val

    def getid(self, msg, field):
        val = msg.get(field, None)
        if val is None:
            val = bytes(24)
        if not isinstance(val, bytes) or len(val) != 24:
            raise ProtocolError('type_error',
                                '{} is not a valid id'.format(field))
        if val == bytes(24):
            val = None
        return val

    def getfbool(self, msg, field):
        val = msg.get(field, False)
        if not isinstance(val, bool):
            raise ProtocolError('type_error', '{} is not bool'.format(field))
        return val

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
            self.send_msg({
                'type': 'ack',
                'rid': msg.rid,
            })

    def msg_modify(self, msg):
        # XXX
        raise NotImplementedError

    def msg_delete(self, msg):
        objs = msg.get('ids', [])
        if not isinstance(objs, (list, tuple)):
            raise ProtocolError('type_error', 'ids is not a list')
        for obj in objs:
            if not isinstance(obj, bytes) or len(obj) != 24:
                raise ProtocolError('invalid_id', 'obj is not a valid id')
        for obj in objs:
            self.srv.delete(obj)
        aid = self.getnint(msg, 'aid')
        if aid is not None:
            self.send_msg({
                'type': 'ack',
                'aid': aid,
            })

    def msg_list(self, msg):
        qid = self.getnint(msg, 'qid')
        if qid in self.subs:
            raise ProtocolError('qid_in_use', 'qid already in use')
        parent = self.getid(msg, 'parent')
        pos_start = self.getnint(msg, 'pos_start')
        pos_end = self.getnint(msg, 'pos_end')
        sub = self.getfbool(msg, 'sub')
        tagsets = msg.get('tags', [{}])
        if not isinstance(tagsets, list):
            raise ProtocolError('type_error', 'tags is not list')
        for tags in tagsets:
            if not isinstance(tags, dict):
                raise ProtocolError('type_error', 'tags is not dict')
            for k, v in tags.items():
                if not isinstance(k, str):
                    raise ProtocolError('type_error', 'tag is not string')
                if not isinstance(v, bool):
                    raise ProtocolError('type_error', 'tag state is not bool')
        lister = Lister(self, qid, self.srv, parent,
                        (pos_start, pos_end), tagsets)
        self.srv.run_lister(lister, sub)
        if sub:
            self.subs[qid] = lister

    def msg_get(self, msg):
        obj = self.getid(msg, 'id')
        sub = self.getfbool(msg, 'sub')
        qid = self.getnint(msg, 'qid')
        if qid in self.subs:
            raise ProtocolError('qid_in_use', 'qid already in use')
        obj = self.srv.get(obj)
        if obj is None:
            self.send_msg({
                'type': 'obj_gone',
                'qid': qid,
            })
        else:
            self.send_obj_reply(qid, obj)
            if sub:
                sub = GetSub(self, qid, obj)
                self.subs[qid] = sub
                obj.add_sub(sub)

    def msg_get_data(self, msg):
        obj = self.getid(msg, 'id')
        sub = self.getfbool(msg, 'sub')
        qid = self.getnint(msg, 'qid')
        key = self.getstr(msg, 'key')
        if qid in self.subs:
            raise ProtocolError('qid_in_use', 'qid already in use')
        obj = self.srv.get(obj)
        if obj is None:
            self.send_msg({
                'type': 'obj_gone',
                'qid': qid,
            })
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
        qid = self.getnint(msg, 'qid')
        if qid in self.subs:
            self.subs[qid].kill()
            del self.subs[qid]
        self.send_msg({
            'type': 'sub_gone',
            'qid': qid,
        })

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

    def send_obj_reply(self, qid, obj):
        self.send_msg({
            'type': 'get_reply',
            'qid': qid,
            'id': obj.id,
            'parent': obj.parent.id if obj.parent is not None else None,
            'pos_start': obj.pos[0],
            'pos_end': obj.pos[1],
            'tags': list(obj.tags),
            'attr': obj.attr,
            'data': list(obj.data),
            'bindata': obj.bindata,
        })

    def send_list_reply(self, qid, new, gone):
        res = []
        for obj in new:
            res.append({
                'id': obj.id,
                'gone': False,
                'parent': obj.parent.id if obj.parent is not None else None,
                'pos_start': obj.pos[0],
                'pos_end': obj.pos[1],
                'tags': list(obj.tags),
                'attr': obj.attr,
                'data': list(obj.data),
                'bindata': obj.bindata,
            })
        for id in gone:
            res.append({
                'id': id,
                'gone': True,
            })
        self.send_msg({
            'type': 'list_reply',
            'objs': res,
            'qid': qid,
        })

    def send_obj_gone(self, qid):
        self.send_msg({
            'type': 'obj_gone',
            'qid': qid,
        })


@asyncio.coroutine
def unix_server(srv, path):
    return (yield from srv.loop.create_unix_server(lambda: Proto(srv), path))


@asyncio.coroutine
def tcp_server(srv, ip, port):
    return (yield from srv.loop.create_server(lambda: Proto(srv), ip, port))
