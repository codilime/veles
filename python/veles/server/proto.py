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
import hmac
import logging
from os import path
import ssl

import msgpack
from OpenSSL import crypto

from veles.proto import messages, msgpackwrap
from veles.proto.messages import PROTO_VERSION
from veles.util.helpers import prepare_auth_key
from veles.db.subscriber import (
    BaseSubscriberNode,
    BaseSubscriberData,
    BaseSubscriberBinData,
    BaseSubscriberList,
)
from veles.async_conn.subscriber import (
    BaseSubscriberQueryRaw,
    BaseSubscriberConnections,
)
from veles.async_conn.plugin import (
    MethodHandler,
    QueryHandler,
    BroadcastHandler,
    TriggerHandler,
)
from veles.proto.exceptions import (
    VelesException,
    UnknownSubscriptionError,
    SubscriptionInUseError,
    ConnectionLostError,
    SchemaError,
    ProtocolMismatchError,
    AuthenticationError,
)
from veles.util import helpers

logger = logging.getLogger(__name__)

class SubscriberNode(BaseSubscriberNode):
    def __init__(self, tracker, node, proto, qid):
        self.proto = proto
        self.qid = qid
        super().__init__(tracker, node)

    def node_changed(self, node):
        self.proto.send_msg(messages.MsgGetReply(
            qid=self.qid,
            obj=node
        ))

    def error(self, err):
        self.proto.send_msg(messages.MsgQueryError(
            qid=self.qid,
            err=err,
        ))


class SubscriberData(BaseSubscriberData):
    def __init__(self, tracker, node, key, proto, qid):
        self.proto = proto
        self.qid = qid
        super().__init__(tracker, node, key)

    def data_changed(self, data):
        self.proto.send_msg(messages.MsgGetDataReply(
            qid=self.qid,
            data=data
        ))

    def error(self, err):
        self.proto.send_msg(messages.MsgQueryError(
            qid=self.qid,
            err=err,
        ))


class SubscriberBinData(BaseSubscriberBinData):
    def __init__(self, tracker, node, key, start, end, proto, qid):
        self.proto = proto
        self.qid = qid
        super().__init__(tracker, node, key, start, end)

    def bindata_changed(self, data):
        self.proto.send_msg(messages.MsgGetBinDataReply(
            qid=self.qid,
            data=data
        ))

    def error(self, err):
        self.proto.send_msg(messages.MsgQueryError(
            qid=self.qid,
            err=err,
        ))


class SubscriberList(BaseSubscriberList):
    def __init__(self, tracker, parent, tags, pos_filter, proto, qid):
        self.proto = proto
        self.qid = qid
        super().__init__(tracker, parent, tags, pos_filter)

    def list_changed(self, changed, gone):
        self.proto.send_msg(messages.MsgGetListReply(
            qid=self.qid,
            objs=changed,
            gone=gone
        ))

    def error(self, err):
        self.proto.send_msg(messages.MsgQueryError(
            qid=self.qid,
            err=err,
        ))


class SubscriberQuery(BaseSubscriberQueryRaw):
    def __init__(self, tracker, node, name, params, trace, proto, qid):
        self.proto = proto
        self.qid = qid
        super().__init__(tracker, node, name, params, trace)

    def raw_result_changed(self, result, checks):
        self.proto.send_msg(messages.MsgGetQueryReply(
            qid=self.qid,
            result=result,
            checks=checks
        ))

    def error(self, err, checks):
        self.proto.send_msg(messages.MsgQueryError(
            qid=self.qid,
            err=err,
            checks=checks,
        ))


class SubscriberConnections(BaseSubscriberConnections):
    def __init__(self, tracker, proto, qid):
        self.proto = proto
        self.qid = qid
        super().__init__(tracker)

    def connections_changed(self, connections):
        self.proto.send_msg(messages.MsgConnectionsReply(
            qid=self.qid,
            connections=connections,
        ))

    def error(self, err):
        self.proto.send_msg(messages.MsgQueryError(
            qid=self.qid,
            err=err,
        ))


class RemoteMethodRunner:
    def __init__(self):
        self.future = asyncio.Future()


class RemoteQueryRunner:
    def __init__(self, checks):
        self.checks = checks
        self.future = asyncio.Future()


class RemoteBroadcastRunner:
    def __init__(self):
        self.future = asyncio.Future()


class RemoteTriggerRunner:
    def __init__(self, checks):
        self.checks = checks
        self.future = asyncio.Future()


class RemoteMethodHandler(MethodHandler):
    def __init__(self, method, tags, proto, phid):
        self.proto = proto
        self.phid = phid
        super().__init__(method, tags)

    def run_method(self, conn, node, params):
        runner = RemoteMethodRunner()
        self.proto.pmids[id(runner)] = runner
        self.proto.send_msg(messages.MsgPluginMethodRun(
            pmid=id(runner),
            phid=self.phid,
            node=node,
            params=params,
        ))
        return runner.future


class RemoteQueryHandler(QueryHandler):
    def __init__(self, query, tags, proto, phid):
        self.proto = proto
        self.phid = phid
        super().__init__(query, tags)

    def get_query(self, conn, node, params, checks):
        runner = RemoteQueryRunner(checks)
        self.proto.pqids[id(runner)] = runner
        self.proto.send_msg(messages.MsgPluginQueryGet(
            pqid=id(runner),
            phid=self.phid,
            node=node,
            params=params,
        ))
        return runner.future


class RemoteBroadcastHandler(BroadcastHandler):
    def __init__(self, broadcast, proto, phid):
        self.proto = proto
        self.phid = phid
        super().__init__(broadcast)

    def run_broadcast(self, conn, params):
        runner = RemoteBroadcastRunner()
        self.proto.pbids[id(runner)] = runner
        self.proto.send_msg(messages.MsgPluginBroadcastRun(
            pbid=id(runner),
            phid=self.phid,
            params=params,
        ))
        return runner.future


class RemoteTriggerHandler(TriggerHandler):
    def __init__(self, trigger, tags, proto, phid):
        self.proto = proto
        self.phid = phid
        super().__init__(trigger, tags)

    def run_trigger(self, conn, anode, checks):
        runner = RemoteTriggerRunner(checks)
        self.proto.ptids[id(runner)] = runner
        self.proto.send_msg(messages.MsgPluginTriggerRun(
            ptid=id(runner),
            phid=self.phid,
            node=anode.node,
        ))
        return runner.future


class ServerProto(asyncio.Protocol):
    def __init__(self, conn, key):
        self.conn = conn
        self.subs = {}
        self.phids = {}
        self.pmids = {}
        self.pqids = {}
        self.pbids = {}
        self.ptids = {}
        self.server_key = key
        self.client_key = b''
        self.authorized = False
        self.connected = False
        self.client_name = None
        self.client_version = None
        self.client_description = None
        self.client_type = None
        self.quit_on_close = False
        self.cid = None

    def connection_made(self, transport):
        self.transport = transport
        wrapper = msgpackwrap.MsgpackWrapper()
        self.unpacker = wrapper.unpacker
        self.packer = wrapper.packer

    def data_received(self, data):
        if not self.authorized:
            tmp = data[:64-len(self.client_key)]
            data = data[len(tmp):]
            self.client_key += tmp
            if len(self.client_key) < 64:
                return
            if not hmac.compare_digest(self.client_key, self.server_key):
                self.send_msg(messages.MsgConnectionError(
                    err=AuthenticationError(),
                ))
                self.transport.close()
                return
            self.authorized = True
        self.unpacker.feed(data)
        while True:
            try:
                msg = messages.MsgpackMsg.load(self.unpacker.unpack())
                loop = asyncio.get_event_loop()
                loop.create_task(self.handle_msg(msg))
            except msgpack.OutOfData:
                return

    async def handle_msg(self, msg):
        if self.connected:
            handlers = {
                'create': self.msg_create,
                'delete': self.msg_delete,
                'set_parent': self.msg_set_parent,
                'set_pos': self.msg_set_pos,
                'add_tag': self.msg_add_tag,
                'del_tag': self.msg_del_tag,
                'set_attr': self.msg_set_attr,
                'set_data': self.msg_set_data,
                'set_bindata': self.msg_set_bindata,
                'transaction': self.msg_transaction,
                'method_run': self.msg_method_run,
                'broadcast_run': self.msg_broadcast_run,
                'get': self.msg_get,
                'get_data': self.msg_get_data,
                'get_bindata': self.msg_get_bindata,
                'get_list': self.msg_get_list,
                'get_query': self.msg_get_query,
                'cancel_subscription': self.msg_cancel_subscription,
                'plugin_method_register': self.msg_plugin_method_register,
                'plugin_method_result': self.msg_plugin_method_result,
                'plugin_method_error': self.msg_plugin_method_error,
                'plugin_query_register': self.msg_plugin_query_register,
                'plugin_query_result': self.msg_plugin_query_result,
                'plugin_query_error': self.msg_plugin_query_error,
                'plugin_broadcast_register':
                    self.msg_plugin_broadcast_register,
                'plugin_broadcast_result': self.msg_plugin_broadcast_result,
                'plugin_trigger_register': self.msg_plugin_trigger_register,
                'plugin_trigger_done': self.msg_plugin_trigger_done,
                'plugin_trigger_error': self.msg_plugin_trigger_error,
                'plugin_handler_unregister':
                    self.msg_plugin_handler_unregister,
                'list_connections': self.msg_list_connections,
            }
        else:
            handlers = {
                'connect': self.msg_connect,
            }
        try:
            if msg.object_type not in handlers:
                raise SchemaError('unhandled message type')
            await handlers[msg.object_type](msg)
        except VelesException as err:
            self.send_msg(messages.MsgProtoError(
                err=err,
            ))

    def connection_lost(self, ex):
        for qid, sub in self.subs.items():
            sub.cancel()
        for handler in self.phids.values():
            self.conn.unregister_plugin_handler(handler)
        for runner in self.pmids.values():
            runner.future.set_exception(ConnectionLostError())
        for runner in self.pqids.values():
            runner.future.set_exception(ConnectionLostError())
        for runner in self.pbids.values():
            runner.future.set_result([])
        for runner in self.ptids.values():
            runner.future.set_exception(ConnectionLostError())
        if self.connected:
            self.conn.remove_conn(self)
        if self.quit_on_close:
            loop = asyncio.get_event_loop()
            loop.stop()

    def send_msg(self, msg):
        self.transport.write(self.packer.pack(msg.dump()))

    async def do_request(self, msg, req):
        try:
            await req
        except VelesException as err:
            self.send_msg(messages.MsgRequestError(
                rid=msg.rid,
                err=err,
            ))
        else:
            self.send_msg(messages.MsgRequestAck(
                rid=msg.rid,
            ))

    async def msg_connect(self, msg):
        # TODO more sophisticated checking if versions are compatible
        if PROTO_VERSION != msg.proto_version:
            self.send_msg(messages.MsgConnectionError(
                err=ProtocolMismatchError(),
            ))
            self.transport.close()
            return
        self.client_name = msg.client_name
        self.client_version = msg.client_version
        self.client_description = msg.client_description
        self.client_type = msg.client_type
        self.quit_on_close = msg.quit_on_close
        self.connected = True
        self.cid = self.conn.new_conn(self)
        self.send_msg(messages.MsgConnected(
            proto_version=PROTO_VERSION,
            server_name='server',
            server_version='server 1.0'
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

    async def msg_set_bindata(self, msg):
        obj = self.conn.get_node_norefresh(msg.id)
        await self.do_request(msg, obj.set_bindata(
            msg.key, msg.start, msg.data, msg.truncate))

    async def msg_transaction(self, msg):
        await self.do_request(msg, self.conn.transaction(
            msg.checks, msg.operations))

    async def msg_method_run(self, msg):
        try:
            result = await self.conn.run_method_raw(
                msg.node, msg.method, msg.params)
        except VelesException as err:
            self.send_msg(messages.MsgMethodError(
                mid=msg.mid,
                err=err,
            ))
        else:
            self.send_msg(messages.MsgMethodResult(
                mid=msg.mid,
                result=result,
            ))

    async def msg_broadcast_run(self, msg):
        results = await self.conn.run_broadcast_raw(msg.broadcast, msg.params)
        self.send_msg(messages.MsgBroadcastResult(
            bid=msg.bid,
            results=results,
        ))

    async def msg_get(self, msg):
        if msg.qid in self.subs:
            raise SubscriptionInUseError()
        if not msg.sub:
            try:
                node = await self.conn.get(msg.id)
            except VelesException as err:
                self.send_msg(messages.MsgQueryError(
                    qid=msg.qid,
                    err=err,
                ))
            else:
                self.send_msg(messages.MsgGetReply(
                    qid=msg.qid,
                    obj=node,
                ))
        else:
            self.subs[msg.qid] = SubscriberNode(
                self.conn, msg.id, self, msg.qid)

    async def msg_get_data(self, msg):
        if msg.qid in self.subs:
            raise SubscriptionInUseError()
        if not msg.sub:
            try:
                data = await self.conn.get_data(msg.id, msg.key)
            except VelesException as err:
                self.send_msg(messages.MsgQueryError(
                    qid=msg.qid,
                    err=err,
                ))
            else:
                self.send_msg(messages.MsgGetDataReply(
                    qid=msg.qid,
                    data=data
                ))
        else:
            self.subs[msg.qid] = SubscriberData(
                self.conn, msg.id, msg.key, self, msg.qid)

    async def msg_get_bindata(self, msg):
        if msg.qid in self.subs:
            raise SubscriptionInUseError()
        if not msg.sub:
            try:
                data = await self.conn.get_bindata(
                    msg.id, msg.key, msg.start, msg.end)
            except VelesException as err:
                self.send_msg(messages.MsgQueryError(
                    qid=msg.qid,
                    err=err,
                ))
            else:
                self.send_msg(messages.MsgGetBinDataReply(
                    qid=msg.qid,
                    data=data
                ))
        else:
            self.subs[msg.qid] = SubscriberBinData(
                self.conn, msg.id, msg.key, msg.start, msg.end, self, msg.qid)

    async def msg_get_list(self, msg):
        if msg.qid in self.subs:
            raise SubscriptionInUseError()
        if not msg.sub:
            try:
                objs = await self.conn.get_list(
                    msg.parent, msg.tags, msg.pos_filter)
            except VelesException as err:
                self.send_msg(messages.MsgQueryError(
                    qid=msg.qid,
                    err=err,
                ))
            else:
                self.send_msg(messages.MsgGetListReply(
                    qid=msg.qid,
                    objs=objs,
                ))
        else:
            self.subs[msg.qid] = SubscriberList(
                self.conn, msg.parent, msg.tags, msg.pos_filter, self, msg.qid)

    async def msg_get_query(self, msg):
        if msg.qid in self.subs:
            raise SubscriptionInUseError()
        if not msg.sub:
            checks = [] if msg.trace else None
            try:
                result = await self.conn.get_query_raw(
                    msg.node, msg.query, msg.params, checks)
            except VelesException as err:
                self.send_msg(messages.MsgQueryError(
                    qid=msg.qid,
                    err=err,
                    checks=checks or [],
                ))
            else:
                self.send_msg(messages.MsgGetQueryReply(
                    qid=msg.qid,
                    result=result,
                    checks=checks or [],
                ))
        else:
            self.subs[msg.qid] = SubscriberQuery(
                self.conn, msg.node, msg.query, msg.params, msg.trace,
                self, msg.qid)

    async def msg_cancel_subscription(self, msg):
        if msg.qid in self.subs:
            self.subs[msg.qid].cancel()
            del self.subs[msg.qid]
            self.send_msg(messages.MsgSubscriptionCancelled(
                qid=msg.qid,
            ))
        else:
            raise UnknownSubscriptionError()

    async def msg_plugin_method_register(self, msg):
        if msg.phid in self.phids:
            raise SubscriptionInUseError()
        handler = RemoteMethodHandler(msg.name, msg.tags, self, msg.phid)
        self.phids[msg.phid] = handler
        self.conn.register_plugin_handler(handler)

    async def msg_plugin_method_result(self, msg):
        if msg.pmid not in self.pmids:
            raise UnknownSubscriptionError()
        self.pmids[msg.pmid].future.set_result(msg.result)
        del self.pmids[msg.pmid]

    async def msg_plugin_method_error(self, msg):
        if msg.pmid not in self.pmids:
            raise UnknownSubscriptionError()
        self.pmids[msg.pmid].future.set_exception(msg.err)
        del self.pmids[msg.pmid]

    async def msg_plugin_query_register(self, msg):
        if msg.phid in self.phids:
            raise SubscriptionInUseError()
        handler = RemoteQueryHandler(msg.name, msg.tags, self, msg.phid)
        self.phids[msg.phid] = handler
        self.conn.register_plugin_handler(handler)

    async def msg_plugin_query_result(self, msg):
        if msg.pqid not in self.pqids:
            raise UnknownSubscriptionError()
        runner = self.pqids[msg.pqid]
        runner.checks += msg.checks
        runner.future.set_result(msg.result)
        del self.pqids[msg.pqid]

    async def msg_plugin_query_error(self, msg):
        if msg.pqid not in self.pqids:
            raise UnknownSubscriptionError()
        runner = self.pqids[msg.pqid]
        runner.checks += msg.checks
        runner.future.set_exception(msg.err)
        del self.pqids[msg.pqid]

    async def msg_plugin_broadcast_register(self, msg):
        if msg.phid in self.phids:
            raise SubscriptionInUseError()
        handler = RemoteBroadcastHandler(msg.name, self, msg.phid)
        self.phids[msg.phid] = handler
        self.conn.register_plugin_handler(handler)

    async def msg_plugin_broadcast_result(self, msg):
        if msg.pbid not in self.pbids:
            raise UnknownSubscriptionError()
        self.pbids[msg.pbid].future.set_result(msg.results)
        del self.pbids[msg.pbid]

    async def msg_plugin_trigger_register(self, msg):
        if msg.phid in self.phids:
            raise SubscriptionInUseError()
        handler = RemoteTriggerHandler(msg.name, msg.tags, self, msg.phid)
        self.phids[msg.phid] = handler
        self.conn.register_trigger_handler(handler)

    async def msg_plugin_trigger_done(self, msg):
        if msg.ptid not in self.ptids:
            raise UnknownSubscriptionError()
        runner = self.ptids[msg.ptid]
        runner.checks += msg.checks
        runner.future.set_result(None)
        del self.ptids[msg.ptid]

    async def msg_plugin_trigger_error(self, msg):
        if msg.ptid not in self.ptids:
            raise UnknownSubscriptionError()
        runner = self.ptids[msg.ptid]
        runner.checks += msg.checks
        runner.future.set_exception(msg.err)
        del self.ptids[msg.ptid]

    async def msg_plugin_handler_unregister(self, msg):
        if msg.phid not in self.phids:
            raise UnknownSubscriptionError()
        self.conn.unregister_plugin_handler(self.phids[msg.phid])
        del self.phids[msg.phid]
        self.send_msg(messages.MsgPluginHandlerUnregistered(phid=msg.phid))

    async def msg_list_connections(self, msg):
        if msg.qid in self.subs:
            raise SubscriptionInUseError()
        if not msg.sub:
            try:
                connections = await self.conn.get_connections()
            except VelesException as err:
                self.send_msg(messages.MsgQueryError(
                    qid=msg.qid,
                    err=err,
                ))
            else:
                self.send_msg(messages.MsgConnectionsReply(
                    qid=msg.qid,
                    connections=connections,
                ))
        else:
            self.subs[msg.qid] = SubscriberConnections(
                self.conn, self, msg.qid)


async def create_unix_server(conn, key, path):
    logger.info('Client url: VELES+UNIX://%s@%s', key, path)
    key = prepare_auth_key(key)
    return await conn.loop.create_unix_server(
        lambda: ServerProto(conn, key), path)


async def create_tcp_server(conn, key, ip, port):
    logger.info('Client url: VELES://%s@%s:%s', key, ip, port)
    key = prepare_auth_key(key)
    return await conn.loop.create_server(
        lambda: ServerProto(conn, key), ip, port)


async def create_ssl_server(conn, key, ip, port, cert_dir):
    cert_path = path.join(cert_dir, 'veles.cert')
    key_path = path.join(cert_dir, 'veles.key')
    if not path.isfile(cert_path) or not path.isfile(key_path):
        helpers.generate_ssl_cert(cert_path, key_path)
    with open(cert_path) as f:
        cert = f.read()
    cert = crypto.load_certificate(crypto.FILETYPE_PEM, cert)
    fingerprint = cert.digest('sha256').decode()

    logger.info('Client url: VELES+SSL://%s:%s@%s:%s',
                key, fingerprint, ip, port)
    key = prepare_auth_key(key)
    sc = ssl.create_default_context(ssl.Purpose.CLIENT_AUTH)
    sc.load_cert_chain(cert_path, key_path)
    return await conn.loop.create_server(
        lambda: ServerProto(conn, key), ip, port, ssl=sc)
