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

from .node import AsyncLocalNode


class BaseLister:
    def __init__(self, conn, parent, pos, tags):
        self.conn = conn
        self.pos = pos
        self.tags = tags
        self.subs = set()
        self.objs = set()
        if parent != NodeID.root_id:
            self.parent = conn.get(parent)
            if self.parent is None:
                self.obj_gone()
                return
        else:
            self.parent = None

    def kill(self):
        if self.parent is None:
            self.conn.top_listers.remove(self)
        else:
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


class AsyncLocalConnection:
    def __init__(self, loop, path):
        self.loop = loop
        self.db = Database(path)
        self.conns = {}
        self.objs = weakref.WeakValueDictionary()
        self.next_cid = 0
        self.top_listers = set()

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
        obj = self.get(obj_id)
        if obj.parent is None:
            listers = self.top_listers
        else:
            listers = obj.parent.listers
        for lister in listers:
            if lister.matches(obj.node):
                lister.list_changed([obj], [])
                lister.objs.add(obj)

    def delete(self, obj_id):
        obj = self.get(obj_id)
        if obj is None:
            return
        for oid in self.db.list(obj_id):
            self.delete(oid)
        self.db.delete(obj_id)
        obj.clear_subs()
        if obj.parent is None:
            listers = self.top_listers
        else:
            listers = obj.parent.listers
        for lister in listers:
            if obj in lister.objs:
                lister.list_changed([], [obj_id])
                lister.objs.remove(obj)
        del self.objs[obj.node.id]

    def get(self, obj_id):
        try:
            return self.objs[obj_id]
        except KeyError:
            node = self.db.get(obj_id)
            if not node:
                return None
            if node.parent:
                parent = self.get(node.parent)
            else:
                parent = None
            res = AsyncLocalNode(self, parent, node)
            self.objs[obj_id] = res
            return res

    def get_data(self, obj, key):
        return self.db.get_data(obj.node.id, key)

    def run_lister(self, lister, sub=False):
        if lister.parent is not None:
            parent = lister.parent.node.id
        else:
            parent = NodeID.root_id
        obj_ids = self.db.list(parent, lister.tags, lister.pos)
        objs = [self.get(x) for x in obj_ids]
        lister.list_changed(objs, [])
        if sub:
            lister.objs = set(objs)
            if lister.parent is None:
                self.top_listers.add(lister)
            else:
                lister.parent.listers.add(lister)

    def add_local_plugin(self, plugin):
        # XXX
        pass
