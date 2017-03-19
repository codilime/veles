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

from veles.util.future import done_future, bad_future
from veles.async_conn.node import AsyncNode
from veles.proto.exceptions import ObjectGoneError, ObjectExistsError
from veles.proto.node import Node
from veles.schema.nodeid import NodeID


def _resolve_obj(conn, obj):
    if isinstance(obj, NodeID):
        return obj, conn.get_node_norefresh(obj)
    elif isinstance(obj, AsyncNode):
        return obj.id, obj
    else:
        raise TypeError('expected NodeID or AsyncNode')


class AsyncLocalNode(AsyncNode):
    def __init__(self, conn, id, node, parent):
        super().__init__(conn, id, node)
        self.parent = parent
        self.subs = set()
        self.data_subs = {}
        self.listers = set()

    # getters

    def refresh(self):
        if self.node is None:
            return bad_future(ObjectGoneError())
        return done_future(self)

    # subscriptions

    def _add_sub(self, sub):
        self.subs.add(sub)
        self.conn.all_subs.add(sub)
        self._send_sub(sub)

    def _del_sub(self, sub):
        self.subs.remove(sub)
        self.conn.all_subs.remove(sub)

    def _send_sub(self, sub):
        if self.node is not None:
            sub.object_changed(self)
        else:
            sub.error(ObjectGoneError())

    def _send_subs(self):
        for sub in self.subs:
            self._send_sub(sub)

    # mutators

    def _create(self, parent=None, pos=(None, None), tags=set(), attr={},
                data={}, bindata={}):
        if self.node is not None:
            raise ObjectExistsError()
        parent_id, parent = _resolve_obj(self.conn, parent)
        if parent.node is None and parent != self.conn.root:
            print(parent, parent.node, self.conn.root)
            raise ObjectGoneError()
        node = Node(id=self.id, parent=parent_id, pos_start=pos[0],
                    pos_end=pos[1], tags=tags, attr=attr, data=set(data),
                    bindata={x: len(y) for x, y in bindata.items()})
        self.conn.db.create(node)
        self.node = node
        self.parent = parent
        self._send_subs()
        for key, val in data.items():
            self.conn.db.set_data(node.id, key, val)
        for key, val in bindata.items():
            self.conn.db.set_bindata(node.id, key, start=0, data=val,
                                     truncate=False)
        for lister in self.parent.listers:
            if lister.matches(self.node):
                lister.list_changed([self], [])
                lister.objs.add(self)
        return done_future(self)

    # misc

    def send_data_subs(self, key, data):
        for sub in self.data_subs.get(key, ()):
            sub.data_changed(self, data)

    def clear_subs(self):
        self._send_subs()
        for k, v in self.data_subs.items():
            for sub in v:
                sub.obj_gone()
        for lister in self.listers:
            lister.obj_gone()

    def remove_data_sub(self, sub):
        self.data_subs[sub.key].remove(sub)

    def add_data_sub(self, sub):
        if sub.key not in self.data_subs:
            self.data_subs[sub.key] = set()
        self.data_subs[sub.key].add(sub)
