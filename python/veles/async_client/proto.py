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

# Through the darkness of future past
# The magician longs to see
# One chants out between two worlds
# Fire, walk with me

import asyncio

import msgpack

from veles.proto import messages, msgpackwrap
from veles.proto.exceptions import VelesException, SchemaError


class ClientProto(asyncio.Protocol):
    def __init__(self):
        wrapper = msgpackwrap.MsgpackWrapper()
        self.unpacker = wrapper.unpacker
        self.packer = wrapper.packer
        self.qids = {}
        self.mids = {}
        self.bids = {}
        self.rids = {}
        self.handlers = {}

    def connection_made(self, transport):
        self.transport = transport

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
            'get_reply': self.msg_get_reply,
            'get_data_reply': self.msg_get_reply,
            'get_bindata_reply': self.msg_get_reply,
            'get_list_reply': self.msg_get_reply,
            'get_query_reply': self.msg_get_reply,
            'query_error': self.msg_query_error,
            'subscription_cancelled': self.msg_subscription_cancelled,
            'proto_error': self.msg_proto_error,
            'request_ack': self.msg_request_ack,
            'request_error': self.msg_request_error,
            'method_result': self.msg_method_result,
            'method_error': self.msg_method_error,
            'broadcast_result': self.msg_broadcast_result,
            'plugin_method_run': self.msg_plugin_method_run,
            'plugin_query_get': self.msg_plugin_query_get,
            'plugin_broadcast_run': self.msg_plugin_broadcast_run,
            'plugin_handler_unregistered':
                self.msg_plugin_handler_unregistered,
        }
        if msg.object_type not in handlers:
            raise SchemaError('unhandled message type: {}'.format(
                msg.object_type))
        await handlers[msg.object_type](msg)

    def connection_lost(self, ex):
        raise RuntimeError("Connection lost")

    def send_msg(self, msg):
        self.transport.write(self.packer.pack(msg.dump()))

    async def msg_get_reply(self, msg):
        self.qids[msg.qid].handle_reply(msg)

    async def msg_query_error(self, msg):
        self.qids[msg.qid].handle_error(msg.err, msg.checks)

    async def msg_subscription_cancelled(self, msg):
        del self.qids[msg.qid]

    async def msg_request_ack(self, msg):
        self.rids[msg.rid].handle_ack()

    async def msg_request_error(self, msg):
        self.rids[msg.rid].handle_error(msg.err)

    async def msg_method_result(self, msg):
        self.mids[msg.mid].handle_result(msg.result)

    async def msg_method_error(self, msg):
        self.mids[msg.mid].handle_error(msg.err)

    async def msg_broadcast_result(self, msg):
        self.bids[msg.bid].handle_result(msg.results)

    async def msg_proto_error(self, msg):
        raise msg.err

    async def msg_plugin_method_run(self, msg):
        handler = self.handlers[msg.phid]
        aresult = handler.run_method(self.conn, msg.node, msg.params)
        try:
            result = await aresult
        except VelesException as e:
            self.send_msg(messages.MsgPluginMethodError(
                pmid=msg.pmid,
                err=e,
            ))
        else:
            self.send_msg(messages.MsgPluginMethodResult(
                pmid=msg.pmid,
                result=result
            ))

    async def msg_plugin_query_get(self, msg):
        handler = self.handlers[msg.phid]
        checks = []
        anode = self.conn.get_node_norefresh(msg.node.id)
        anode.node = msg.node
        aresult = handler.get_query(self.conn, anode, msg.params, checks)
        try:
            result = await aresult
        except VelesException as e:
            self.send_msg(messages.MsgPluginQueryError(
                pqid=msg.pqid,
                err=e,
                checks=checks,
            ))
        else:
            self.send_msg(messages.MsgPluginQueryResult(
                pqid=msg.pqid,
                result=result,
                checks=checks,
            ))

    async def msg_plugin_broadcast_run(self, msg):
        handler = self.handlers[msg.phid]
        aresults = handler.run_broadcast(self.conn, msg.params)
        results = await aresults
        self.send_msg(messages.MsgPluginBroadcastResult(
            pbid=msg.pbid,
            results=results
        ))

    async def msg_plugin_handler_unregistered(self, msg):
        del self.handlers[msg.phid]


async def create_unix_client(loop, path):
    return await loop.create_unix_connection(ClientProto, path)


async def create_tcp_client(loop, ip, port):
    return await loop.create_connection(ClientProto, ip, port)
