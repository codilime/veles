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

from veles.common.base import NodeID


class AsyncConnection:
    """
    A proxy class used to access a server's nodes asynchronously.
    """

    def get_node_norefresh(self, id):
        """
        Returns an AsyncNode with the given id immediately.  Does not fetch it
        from the server - it may contain invalid data until refresh is called.
        Such nodes may still be useful for some operations (eg. listing
        children).
        """
        raise NotImplementedError

    def get_node(self, id):
        """
        Fetches an AsyncNode with the given id from the server.  If this
        node is already available, it is refreshed.  Returns an awaitable
        of AsyncNode.
        """
        return self.get_node_norefresh(id).refresh()

    def get_list(self, parent, tags=frozenset(), pos_start_range=None,
                 pos_end_range=None):
        """
        Fetches a list of nodes with the given parent, containing given
        tags, and with pos_start/pos_end in the given ranges.
        Returns an awaitable of list of AsyncNodes.
        """
        raise NotImplementedError

    def create(self, id=None, parent=None, pos=(None, None), tags=frozenset(),
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

    # subscriptions - only to be used by BaseSubscriberList

    def _add_sub_list(self, sub):
        raise NotImplementedError

    def _del_sub_list(self, sub):
        raise NotImplementedError
