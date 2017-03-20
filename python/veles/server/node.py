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
from veles.proto.node import Node, PosFilter
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
        self.list_subs = {}

    # getters

    def refresh(self):
        if self.node is None:
            return bad_future(ObjectGoneError())
        return done_future(self)

    def get_data(self, key):
        if self.node is None:
            return bad_future(ObjectGoneError())
        return done_future(self.conn.db.get_data(self.id, key))

    def get_list(self, tags=frozenset(), pos_filter=PosFilter()):
        if self.node is None and self != self.conn.root:
            return bad_future(ObjectGoneError())
        obj_ids = self.conn.db.list(self.id, tags, pos_filter)
        objs = [self.conn.get_node_norefresh(x) for x in obj_ids]
        return done_future(objs)

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
            sub.object_changed()
        else:
            sub.error(ObjectGoneError())

    def _send_subs_unparent(self):
        if self.parent is not None:
            for sub, objs in self.parent.list_subs.items():
                if self in objs:
                    objs.remove(self)
                    sub.list_changed([], [self.id])

    def _send_subs(self):
        for sub in self.subs:
            self._send_sub(sub)
        if self.parent is not None:
            for sub, objs in self.parent.list_subs.items():
                if sub.matches(self.node):
                    objs.add(self)
                    sub.list_changed([self], [])
                elif self in objs:
                    objs.remove(self)
                    sub.list_changed([], [self.id])

    def _add_sub_list(self, sub):
        obj_ids = self.conn.db.list(self.id, sub.tags, sub.pos_filter)
        objs = {self.conn.get_node_norefresh(x) for x in obj_ids}
        self.list_subs[sub] = objs
        self.conn.all_subs.add(sub)
        sub.list_changed(objs, [])

    def _del_sub_list(self, sub):
        del self.list_subs[sub]
        self.conn.all_subs.remove(sub)

    # data sub

    def _add_sub_data(self, sub):
        if sub.key not in self.data_subs:
            self.data_subs[sub.key] = set()
        self.data_subs[sub.key].add(sub)
        self.conn.all_subs.add(sub)
        self._send_sub_data(sub)

    def _del_sub_data(self, sub):
        self.data_subs[sub.key].remove(sub)
        if not self.data_subs[sub.key]:
            del self.data_subs[sub.key]
        self.conn.all_subs.remove(sub)

    def _send_sub_data(self, sub):
        if self.node is None:
            sub.error(ObjectGoneError())
        else:
            sub.data_changed(self.conn.db.get_data(self.id, sub.key))

    def _send_subs_data(self, key, data):
        for sub in self.data_subs.get(key, ()):
            sub.data_changed(data)

    # mutators

    def _create(self, parent=None, pos=(None, None), tags=set(), attr={},
                data={}, bindata={}):
        if self.node is not None:
            return bad_future(ObjectExistsError())
        parent_id, parent = _resolve_obj(self.conn, parent)
        if parent.node is None and parent != self.conn.root:
            return bad_future(ObjectGoneError())
        node = Node(id=self.id, parent=parent_id, pos_start=pos[0],
                    pos_end=pos[1], tags=tags, attr=attr, data=set(data),
                    bindata={x: len(y) for x, y in bindata.items()})
        self.conn.db.create(node)
        self.node = node
        self.parent = parent
        for key, val in data.items():
            self.conn.db.set_data(node.id, key, val)
        for key, val in bindata.items():
            self.conn.db.set_bindata(node.id, key, start=0, data=val,
                                     truncate=False)
        self._send_subs()
        for sub in self.list_subs:
            sub.list_changed([], [])
        for key, subs in self.data_subs.items():
            data = self.conn.db.get_data(self.id, key)
            for sub in subs:
                sub.data_changed(data)
        return done_future(self)

    def delete(self):
        if self.node is not None:
            for oid in self.conn.db.list(self.id):
                self.conn.get_node_norefresh(oid).delete()
            self.conn.db.delete(self.id)
            self.node = None
            self._send_subs_unparent()
            self.parent = None
            self._send_subs()
            for key, subs in self.data_subs.items():
                for sub in subs:
                    sub.error(ObjectGoneError())
            for sub in self.list_subs:
                sub.error(ObjectGoneError())
        return done_future(None)

    def set_data(self, key, value):
        if self.node is None:
            return bad_future(ObjectGoneError())
        self.conn.db.set_data(self.id, key, value)
        self._send_subs_data(key, value)
        if value is None and key in self.node.data:
            self.node.data.remove(key)
            self._send_subs()
        elif value is not None and key not in self.node.data:
            self.node.data.add(key)
            self._send_subs()
        return done_future(None)
