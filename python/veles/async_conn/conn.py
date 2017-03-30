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

from veles.schema.nodeid import NodeID
from .plugin import MethodHandler


class AsyncConnection:
    """
    A proxy class used to access a server's nodes asynchronously.
    """

    def __init__(self):
        self.root = self.get_node_norefresh(NodeID.root_id)

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

    def register_plugin_handler(self, handler):
        raise NotImplementedError

    def unregister_plugin_handler(self, handler):
        raise NotImplementedError

    def register_plugin(self, plugin):
        for v in plugin.__dict__.values():
            if isinstance(v, MethodHandler):
                self.register_plugin_handler(v)

    def unregister_plugin(self, plugin):
        for v in plugin.__dict__.values():
            if isinstance(v, MethodHandler):
                self.unregister_plugin_handler(v)
