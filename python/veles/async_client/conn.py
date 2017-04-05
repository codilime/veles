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

import asyncio

import weakref

from veles.proto import messages
from veles.async_conn.conn import AsyncConnection
from veles.async_conn.plugin import (
    MethodHandler,
    QueryHandler,
    BroadcastHandler,
)
from .node import AsyncRemoteNode


class Request:
    def __init__(self, proto):
        self.proto = proto
        self.future = asyncio.Future()
        self.proto.rids[id(self)] = self

    def handle_ack(self):
        self.future.set_result(None)
        del self.proto.rids[id(self)]

    def handle_error(self, exc):
        self.future.set_exception(exc)
        del self.proto.rids[id(self)]


class BroadcastRunner:
    def __init__(self, proto):
        self.proto = proto
        self.future = asyncio.Future()
        self.proto.bids[id(self)] = self

    def handle_result(self, result):
        self.future.set_result(result)
        del self.proto.bids[id(self)]


class AsyncRemoteConnection(AsyncConnection):
    def __init__(self, loop, proto):
        self.loop = loop
        self.proto = proto
        self.objs = weakref.WeakValueDictionary()
        self.conns = {}
        self.next_cid = 0
        self.proto.conn = self
        super().__init__()

    def get_node_norefresh(self, obj_id):
        try:
            return self.objs[obj_id]
        except KeyError:
            res = AsyncRemoteNode(self, obj_id, None)
            self.objs[obj_id] = res
            return res

    def new_conn(self, conn):
        cid = self.next_cid
        self.next_cid += 1
        self.conns[cid] = conn
        print("Conn {} started.".format(cid))
        return cid

    def remove_conn(self, conn):
        print("Conn {} gone.".format(conn.cid))
        self.conns[conn.cid] = None

    def transaction(self, checks, operations):
        req = Request(self.proto)
        self.proto.send_msg(messages.MsgTransaction(
            rid=id(req),
            checks=checks,
            operations=operations,
        ))
        return req.future

    def run_broadcast_raw(self, broadcast, params):
        br = BroadcastRunner(self.proto)
        self.proto.send_msg(messages.MsgBroadcastRun(
            bid=id(br),
            broadcast=broadcast,
            params=params,
        ))
        return br.future

    def register_plugin_handler(self, handler):
        if isinstance(handler, MethodHandler):
            self.proto.send_msg(messages.MsgPluginMethodRegister(
                phid=id(handler),
                name=handler.method,
                tags=handler.tags,
            ))
        elif isinstance(handler, QueryHandler):
            self.proto.send_msg(messages.MsgPluginQueryRegister(
                phid=id(handler),
                name=handler.query,
                tags=handler.tags,
            ))
        elif isinstance(handler, BroadcastHandler):
            self.proto.handlers[id(handler)] = handler
            self.proto.send_msg(messages.MsgPluginBroadcastRegister(
                phid=id(handler),
                name=handler.broadcast,
            ))
        else:
            raise TypeError("unknown type of plugin handler")
        self.proto.handlers[id(handler)] = handler

    def unregister_plugin_handler(self, handler):
        self.proto.send_msg(messages.MsgPluginHandlerUnregister(
            phid=id(handler),
        ))
