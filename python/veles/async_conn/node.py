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

from veles.proto.node import PosFilter
from veles.proto import operation


class AsyncNode:
    """
    A proxy class for a node available via an asynchronous connection.
    It has the following attributes:

    - ``conn``: the associated AsyncConnection.
    - ``id``: the node id.
    - ``node``: a proto.Node instance with this node's data, or None if the
      node is invalid (does not exist, or hasn't yet been fetched from
      the server).
    """

    def __init__(self, conn, id, node):
        self.conn = conn
        self.id = id
        self.node = node

    def get_parent(self):
        """
        Returns an awaitable of AsyncNode representing the parent.
        """
        return self.conn.get_node(self.node.parent)

    # queries

    def refresh(self):
        """
        Refetches the node from connection and returns an awaitable of self.
        """
        anode = self.conn.get(self.id)

        async def inner():
            try:
                self.node = await anode
            except:
                self.node = None
                raise
            return self

        loop = asyncio.get_event_loop()
        return loop.create_task(inner())

    def get_data(self, key):
        """
        Fetches data with the given key, returns an awaitable of the data
        value.
        """
        return self.conn.get_data(self.id, key)

    def get_bindata(self, key, start, end):
        """
        Fetches a range of bindata with the given key, returns an awaitable
        of bytes.
        """
        return self.conn.get_bindata(self.id, key, start, end)

    def get_list(self, tags=frozenset(), pos_filter=PosFilter()):
        """
        Fetches a list of children of this node, containing given
        tags, and with pos_start/pos_end in the given ranges.
        Returns an awaitable of list of AsyncNodes.
        """
        anodes = self.conn.get_list(self.id, tags, pos_filter)

        async def inner():
            nodes = await anodes
            res = []
            for node in nodes:
                anode = self.conn.get_node_norefresh(node.id)
                anode.node = node
                res.append(anode)
            return res

        loop = asyncio.get_event_loop()
        return loop.create_task(inner())

    def get_query_raw(self, name, params, checks=None):
        """
        Runs a query, returns an awaitable of the result.  If checks is not
        None, it should be a list that query's checks will be appended to.
        """
        return self.conn.get_query_raw(self.id, name, params, checks)

    def get_query(self, sig, params, checks=None):
        """
        Runs a query, translating its params and result according
        to the given signature.  Returns an awaitable of the result.
        """
        return self.conn.get_query(self.id, sig, params, checks)

    # mutators

    def _create(self, parent, pos=(None, None), tags=set(), attr={},
                data={}, bindata={}):
        """
        Fills this object with data and creates it on the server.
        Returns an awaitable of self.
        """
        async def inner():
            await self.conn.transaction([], [
                operation.OperationCreate(
                    node=self.id,
                    parent=parent,
                    pos_start=pos[0],
                    pos_end=pos[1],
                    tags=tags,
                    attr=attr,
                    data=data,
                    bindata=bindata,
                )
            ])
            return self

        loop = asyncio.get_event_loop()
        return loop.create_task(inner())

    def delete(self):
        """
        Deletes this object on the server.  Returns an awaitable of None.
        """
        return self.conn.transaction([], [
            operation.OperationDelete(
                node=self.id
            )
        ])

    def set_parent(self, parent):
        """
        Changes the parent of this object.  Parent should be a NodeID.
        Returns an awaitable of None.
        """
        return self.conn.transaction([], [
            operation.OperationSetParent(
                node=self.id,
                parent=parent
            )
        ])

    def set_pos(self, start, end):
        """
        Changes the position of this object.  Returns an awaitable of None.
        """
        return self.conn.transaction([], [
            operation.OperationSetParent(
                node=self.id,
                pos_start=start,
                pos_end=end,
            )
        ])

    def add_tag(self, tag):
        """
        Adds a tag to this object.  Returns an awaitable of None.
        """
        return self.conn.transaction([], [
            operation.OperationAddTag(
                node=self.id,
                tag=tag,
            )
        ])

    def del_tag(self, tag):
        """
        Removes a tag from this object.  Returns an awaitable of None.
        """
        return self.conn.transaction([], [
            operation.OperationDelTag(
                node=self.id,
                tag=tag,
            )
        ])

    def set_attr(self, key, value):
        """
        Sets the value of an attribute of this object, or deletes an attribute
        if the value is None.  Returns an awaitable of None.
        """
        return self.conn.transaction([], [
            operation.OperationSetAttr(
                node=self.id,
                key=key,
                data=value,
            )
        ])

    def set_data(self, key, value):
        """
        Sets a data value associated with this object, or deletes it
        if the value is None.  Returns an awaitable of None.
        """
        return self.conn.transaction([], [
            operation.OperationSetData(
                node=self.id,
                key=key,
                data=value,
            )
        ])

    def set_bindata(self, key, start, value, truncate=False):
        """
        Sets a range of binary data associated with this object.  If truncate
        is set, all data after the modified range is deleted.  If this results
        in a zero-sized binary data, the key is deleted.  Returns an awaitable
        of None.
        """
        return self.conn.transaction([], [
            operation.OperationSetBinData(
                node=self.id,
                key=key,
                start=start,
                data=value,
                truncate=truncate,
            )
        ])

    def run_method_raw(self, method, params):
        """
        Runs a method on the object.  Returns an awaitable of the result.
        """
        return self.conn.run_method_raw(self.id, method, params)

    def run_method(self, sig, params):
        """
        Runs a method on the object, translating its params and result
        according to the given signature.  Returns an awaitable of the result.
        """
        return self.conn.run_method(self.id, sig, params)
