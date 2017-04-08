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

import weakref

import asyncio

from veles.proto.node import PosFilter
from veles.schema.nodeid import NodeID
from .plugin import MethodHandler, QueryHandler, BroadcastHandler
from .node import AsyncNode


class AsyncConnection:
    """
    A proxy class used to access a server's nodes asynchronously.
    """

    def __init__(self):
        self.anodes = weakref.WeakValueDictionary()
        self.root = self.get_node_norefresh(NodeID.root_id)

    def get_node_norefresh(self, nid):
        """
        Returns an AsyncNode with the given id immediately.  Does not fetch it
        from the server - it may contain invalid data until refresh is called.
        Such nodes may still be useful for some operations (eg. listing
        children).
        """
        try:
            return self.anodes[nid]
        except KeyError:
            res = AsyncNode(self, nid, None)
            self.anodes[nid] = res
            return res

    def get_node(self, id):
        """
        Fetches an AsyncNode with the given id from the server.  If this
        node is already available, it is refreshed.  Returns an awaitable
        of AsyncNode.
        """
        return self.get_node_norefresh(id).refresh()

    def get(self, node):
        """
        Fetches a Node with the given id from the server.  Returns
        an awaitable of Node.
        """
        raise NotImplementedError

    def get_data(self, node, key):
        """
        Fetches data of a given node with the given key, returns an awaitable
        of the data value.
        """
        raise NotImplementedError

    def get_bindata(self, node, key, start, end):
        """
        Fetches a range of bindata of a given node with the given key, returns
        an awaitable of bytes.
        """
        raise NotImplementedError

    def get_list(self, parent, tags=frozenset(), pos_filter=PosFilter()):
        """
        Fetches a list of children of a given node, containing given
        tags, and with pos_start/pos_end in the given ranges.
        Returns an awaitable of list of Nodes.
        """
        raise NotImplementedError

    def get_query_raw(self, node, name, params, checks=None):
        """
        Runs a query, returns an awaitable of the result.  If checks is not
        None, it should be a list that query's checks will be appended to.
        """
        raise NotImplementedError

    def get_query(self, node, sig, params, checks=None):
        """
        Runs a query, translating its params and result according
        to the given signature.  Returns an awaitable of the result.
        """
        params = sig.params.dump(params)
        aresult = self.get_query_raw(node, sig.query, params, checks)

        async def get_result():
            result = await aresult
            return sig.result.load(result)

        loop = asyncio.get_event_loop()
        return loop.create_task(get_result())

    def create(self, id=None, parent=None, pos=(None, None), tags=set(),
               attr={}, data={}, bindata={}):
        """
        Creates a node with the given data.  If ``id`` is None, a random id
        will be generated.  Returns a tuple of (node, awaitable of node) -
        the node is available immediately (and can be linked to other objects,
        or used as a parent), and the awaitable will be resolved once
        the operation completes on the server.
        """
        if id is None:
            id = NodeID()
        obj = self.get_node_norefresh(id)
        return obj, obj._create(parent, pos, tags, attr, data, bindata)

    def transaction(self, checks, operations):
        raise NotImplementedError

    def run_method_raw(self, node, method, params):
        """
        Runs a method on the given node.  Returns an awaitable of the result.
        """
        raise NotImplementedError

    def run_method(self, node, sig, params):
        """
        Runs a method on the given node, translating its params and result
        according to the given signature.  Returns an awaitable of the result.
        """
        params = sig.params.dump(params)
        aresult = self.run_method_raw(node, sig.method, params)

        async def get_result():
            result = await aresult
            return sig.result.load(result)

        loop = asyncio.get_event_loop()
        return loop.create_task(get_result())

    def run_broadcast_raw(self, broadcast, params):
        """
        Runs a broadcast.  Returns an awaitable of the results.
        """
        raise NotImplementedError

    def run_broadcast(self, sig, params):
        """
        Runs a broadcast, translating its params and results
        according to the given signature.  Returns an awaitable of the results.
        """
        params = sig.params.dump(params)
        aresults = self.run_broadcast_raw(sig.broadcast, params)

        async def get_results():
            results = await aresults
            return [sig.result.load(result) for result in results]

        loop = asyncio.get_event_loop()
        return loop.create_task(get_results())

    def register_subscriber(self, sub):
        raise NotImplementedError

    def unregister_subscriber(self, sub):
        raise NotImplementedError

    def register_plugin_handler(self, handler):
        raise NotImplementedError

    def unregister_plugin_handler(self, handler):
        raise NotImplementedError

    def register_plugin(self, plugin):
        for v in plugin.__dict__.values():
            if isinstance(v, (MethodHandler, QueryHandler, BroadcastHandler)):
                self.register_plugin_handler(v)

    def unregister_plugin(self, plugin):
        for v in plugin.__dict__.values():
            if isinstance(v, (MethodHandler, QueryHandler, BroadcastHandler)):
                self.unregister_plugin_handler(v)
