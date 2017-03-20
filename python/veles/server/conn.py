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

    def cancel(self):
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
        # all_subs is a set of all subscriptions in existence - this exists
        # so that all involved objects are strongly referenced (otherwise,
        # being only reacheble through the weak dict, they could be GCd along
        # with all their subscriptions).
        self.all_subs = set()
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

    def get_node_norefresh(self, obj_id):
        try:
            return self.objs[obj_id]
        except KeyError:
            node = self.db.get(obj_id)
            if not node:
                res = AsyncLocalNode(self, obj_id, None, None)
            else:
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
