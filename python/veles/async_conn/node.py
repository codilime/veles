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


from veles.proto.node import PosFilter


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
        return self.conn.get_node(self.node.parent_id)

    # queries

    def refresh(self):
        """
        Refetches the node from connection and returns an awaitable of self.
        """
        raise NotImplementedError

    def get_data(self, key):
        """
        Fetches data with the given key, returns an awaitable of the data
        value.
        """
        raise NotImplementedError

    def get_bindata(self, key, start, end):
        """
        Fetches a range of bindata with the given key, returns an awaitable
        of BinData.
        """
        raise NotImplementedError

    def get_list(self, tags=frozenset(), pos_filter=PosFilter()):
        """
        Fetches a list of children of this node, containing given
        tags, and with pos_start/pos_end in the given ranges.
        Returns an awaitable of list of AsyncNodes.
        """
        raise NotImplementedError

    def get_query(self, name, params, tracer=None):
        """
        Runs a query, returns an awaitable of the result.  ``tracer`` should
        be a Tracer instance that will record the data used, or None.
        """
        raise NotImplementedError

    # subscriptions - only to be used by BaseSubscriber*.

    def _add_sub(self, sub):
        raise NotImplementedError

    def _del_sub(self, sub):
        raise NotImplementedError

    def _add_sub_data(self, sub):
        raise NotImplementedError

    def _del_sub_data(self, sub):
        raise NotImplementedError

    def _add_sub_bindata(self, sub):
        raise NotImplementedError

    def _del_sub_bindata(self, sub):
        raise NotImplementedError

    def _add_sub_list(self, sub):
        raise NotImplementedError

    def _del_sub_list(self, sub):
        raise NotImplementedError

    def _add_sub_query(self, sub):
        raise NotImplementedError

    def _del_sub_query(self, sub):
        raise NotImplementedError

    # mutators

    def _create(self, parent, pos=(None, None), tags=set(), attr={},
                data={}, bindata={}):
        """
        Fills this object with data and creates it on the server.
        Returns an awaitable of self.
        """
        raise NotImplementedError

    def delete(self):
        """
        Deletes this object on the server.  Returns an awaitable of None.
        """
        raise NotImplementedError

    def set_parent(self, parent):
        """
        Changes the parent of this object.  Parent may be an AsyncNode
        or NodeID.  Returns an awaitable of None.
        """
        raise NotImplementedError

    def set_pos(self, start, end):
        """
        Changes the position of this object.  Returns an awaitable of None.
        """
        raise NotImplementedError

    def add_tag(self, tag):
        """
        Adds a tag to this object.  Returns an awaitable of None.
        """
        raise NotImplementedError

    def del_tag(self, tag):
        """
        Removes a tag from this object.  Returns an awaitable of None.
        """
        raise NotImplementedError

    def set_attr(self, key, value):
        """
        Sets the value of an attribute of this object, or deletes an attribute
        if the value is None.  Returns an awaitable of None.
        """
        raise NotImplementedError

    def set_data(self, key, value):
        """
        Sets a data value associated with this object, or deletes it
        if the value is None.  Returns an awaitable of None.
        """
        raise NotImplementedError

    def set_bindata(self, key, start, value, truncate=False):
        """
        Sets a range of binary data associated with this object.  If truncate
        is set, all data after the modified range is deleted.  If this results
        in a zero-sized binary data, the key is deleted.  Returns an awaitable
        of None.
        """
        raise NotImplementedError

    def run_mthd(self, name, params):
        """
        Runs a method on the object.  Returns an awaitable of the result.
        """
        raise NotImplementedError
