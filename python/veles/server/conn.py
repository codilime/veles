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

# I believe
# Some things can't be explained
# They are hidden in the mist
# And in the silver rain

import weakref

from veles.proto.node import Node
from veles.schema.nodeid import NodeID
from veles.db import Database

from veles.async_conn.conn import AsyncConnection

from .node import AsyncLocalNode


class BaseLister:
    def __init__(self, conn, parent, pos, tags):
        self.conn = conn
        self.pos = pos
        self.tags = tags
        self.subs = set()
        self.objs = set()
        self.parent = conn.get_node_norefresh(parent)
        if parent != NodeID.root_id and self.parent.node is None:
            self.obj_gone()

    def kill(self):
        self.parent.listers.remove(self)

    def matches(self, obj):
        if not self.pos.matches(obj):
            return False
        if not self.tags <= obj.tags:
            return False
        return True

    def list_changed(self):
        raise NotImplementedError

    def obj_gone(self):
        raise NotImplementedError


class AsyncLocalConnection(AsyncConnection):
    def __init__(self, loop, path):
        self.loop = loop
        self.db = Database(path)
        self.conns = {}
        self.objs = weakref.WeakValueDictionary()
        self.next_cid = 0
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

    def create(self, obj_id, parent, *, tags=[], attr={}, data={}, bindata={},
               pos=(None, None)):
        node = Node(id=obj_id, parent=parent, pos_start=pos[0],
                    pos_end=pos[1], tags=tags, attr=attr, data=set(data),
                    bindata={x: len(y) for x, y in bindata.items()})
        self.db.create(node)
        for key, val in data.items():
            self.db.set_data(node.id, key, val)
        for key, val in bindata.items():
            self.db.set_bindata(node.id, key, start=0, data=val,
                                truncate=False)
        obj = self.get_node_norefresh(obj_id)
        for lister in obj.parent.listers:
            if lister.matches(obj.node):
                lister.list_changed([obj], [])
                lister.objs.add(obj)

    def delete(self, obj_id):
        obj = self.get_node_norefresh(obj_id)
        if obj.node is None:
            return
        for oid in self.db.list(obj_id):
            self.delete(oid)
        self.db.delete(obj_id)
        obj.clear_subs()
        for lister in obj.parent.listers:
            if obj in lister.objs:
                lister.list_changed([], [obj_id])
                lister.objs.remove(obj)
        del self.objs[obj.node.id]

    def get_node_norefresh(self, obj_id):
        try:
            return self.objs[obj_id]
        except KeyError:
            node = self.db.get(obj_id)
            if not node:
                return AsyncLocalNode(self, obj_id, None, None)
            parent = self.get_node_norefresh(node.parent)
            assert parent.node is not None or parent.id == NodeID.root_id
            res = AsyncLocalNode(self, obj_id, node, parent)
            self.objs[obj_id] = res
            return res

    def get_data(self, obj, key):
        return self.db.get_data(obj.node.id, key)

    def run_lister(self, lister, sub=False):
        obj_ids = self.db.list(lister.parent.id, lister.tags, lister.pos)
        objs = [self.get_node_norefresh(x) for x in obj_ids]
        lister.list_changed(objs, [])
        if sub:
            lister.objs = set(objs)
            lister.parent.listers.add(lister)

    def add_local_plugin(self, plugin):
        # XXX
        pass
