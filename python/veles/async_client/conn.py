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

from veles.proto import messages
from veles.proto.node import PosFilter
from veles.async_conn.conn import AsyncConnection
from veles.async_conn.plugin import (
    MethodHandler,
    QueryHandler,
    BroadcastHandler,
)
from veles.db.subscriber import (
    BaseSubscriberNode,
    BaseSubscriberData,
    BaseSubscriberBinData,
    BaseSubscriberList,
)
from veles.async_conn.subscriber import (
    BaseSubscriberQuery,
)
from .runner import (
    NodeGetter,
    DataGetter,
    BinDataGetter,
    ListGetter,
    QueryGetter,
    Request,
    MethodRunner,
    BroadcastRunner,
)
from .subscriber import (
    NodeSubManager,
    DataSubManager,
    BinDataSubManager,
    ListSubManager,
    QuerySubManager,
)


class AsyncRemoteConnection(AsyncConnection):
    def __init__(self, loop, proto):
        self.loop = loop
        self.proto = proto
        self.conns = {}
        self.next_cid = 0
        self.proto.conn = self
        self.subs = {}
        super().__init__()

    def new_conn(self, conn):
        cid = self.next_cid
        self.next_cid += 1
        self.conns[cid] = conn
        print("Conn {} started.".format(cid))
        return cid

    def remove_conn(self, conn):
        print("Conn {} gone.".format(conn.cid))
        self.conns[conn.cid] = None

    # getters

    def get(self, node):
        return NodeGetter(self.proto, node).future

    def get_data(self, node, key):
        return DataGetter(self.proto, node, key).future

    def get_bindata(self, node, key, start, end):
        return BinDataGetter(self.proto, node, key, start, end).future

    def get_list(self, parent, tags=frozenset(), pos_filter=PosFilter()):
        return ListGetter(self.proto, parent, set(tags), pos_filter).future

    def get_query_raw(self, node, name, params, checks=None):
        return QueryGetter(self.proto, node, name, params, checks).future

    # mutators

    def transaction(self, checks, operations):
        req = Request(self.proto)
        self.proto.send_msg(messages.MsgTransaction(
            rid=id(req),
            checks=checks,
            operations=operations,
        ))
        return req.future

    def run_method_raw(self, node, method, params):
        mr = MethodRunner(self.proto)
        self.proto.send_msg(messages.MsgMethodRun(
            mid=id(mr),
            node=node,
            method=method,
            params=params,
        ))
        return mr.future

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

    # subscribers

    def register_subscriber(self, sub):
        if isinstance(sub, BaseSubscriberNode):
            manager = NodeSubManager(sub)
        elif isinstance(sub, BaseSubscriberData):
            manager = DataSubManager(sub)
        elif isinstance(sub, BaseSubscriberBinData):
            manager = BinDataSubManager(sub)
        elif isinstance(sub, BaseSubscriberList):
            manager = ListSubManager(sub)
        elif isinstance(sub, BaseSubscriberQuery):
            manager = QuerySubManager(sub)
        else:
            raise TypeError('unknown type of subscription')
        self.subs[sub] = manager

    def unregister_subscriber(self, sub):
        self.subs[sub].cancel()
        del self.subs[sub]
