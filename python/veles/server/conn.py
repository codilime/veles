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

import asyncio

import weakref

from collections import defaultdict

from veles.util.future import done_future, bad_future
from veles.schema.nodeid import NodeID
from veles.db import Database
from veles.proto import operation, check
from veles.proto.node import Node
from veles.proto.exceptions import (
    VelesException,
    ObjectGoneError,
    ObjectExistsError,
    ParentCycleError,
    PreconditionFailedError,
    RegistryNoMatchError,
    RegistryMultiMatchError,
)
from veles.async_conn.plugin import (
    MethodHandler, QueryHandler, BroadcastHandler
)
from veles.async_conn.conn import AsyncConnection

from .node import AsyncLocalNode
from .transaction import Transaction


class NameTagsRegistry:
    def __init__(self):
        self.items = defaultdict(set)

    def register(self, name, tags, item):
        self.items[name].add((frozenset(tags), item))

    def unregister(self, name, tags, item):
        self.items[name].remove((frozenset(tags), item))

    def find(self, name, tags):
        res = None
        best = -1
        for itags, item in self.items[name]:
            if itags <= tags and len(itags) >= best:
                if len(itags) > best:
                    best = len(itags)
                    res = []
                res.append(item)
        if res is None:
            raise RegistryNoMatchError()
        if len(res) != 1:
            raise RegistryMultiMatchError()
        return res[0]


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
        self.methods = NameTagsRegistry()
        self.queries = NameTagsRegistry()
        self.broadcasts = {}
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

    def _check_ok_gone(self, el):
        node = self.get_node_norefresh(el.node)
        return node.node is None

    def _check_ok_parent(self, el):
        node = self.get_node_norefresh(el.node)
        if node.node is None:
            return False
        return node.node.parent == el.parent

    def _check_ok_pos(self, el):
        node = self.get_node_norefresh(el.node)
        if node.node is None:
            return False
        return (
            node.node.pos_start == el.pos_start and
            node.node.pos_end == el.pos_end)

    def _check_ok_tags(self, el):
        node = self.get_node_norefresh(el.node)
        if node.node is None:
            return False
        return node.node.tags == el.tags

    def _check_ok_tag(self, el):
        node = self.get_node_norefresh(el.node)
        if node.node is None:
            return False
        return (el.tag in node.node.tags) == el.present

    def _check_ok_attr(self, el):
        node = self.get_node_norefresh(el.node)
        if node.node is None:
            return False
        return node.node.attr.get(el.key) == el.data

    def _check_ok_data(self, el):
        node = self.get_node_norefresh(el.node)
        if node.node is None:
            return False
        return self.db.get_data(node.id, el.key) == el.data

    def _check_ok_bindata_size(self, el):
        node = self.get_node_norefresh(el.node)
        if node.node is None:
            return False
        return node.node.bindata.get(el.key, 0) == el.size

    def _check_ok_bindata(self, el):
        node = self.get_node_norefresh(el.node)
        if node.node is None:
            return False
        data = self.db.get_bindata(node.id, el.key, el.start, el.end)
        return data == el.data

    def _check_ok_list(self, el):
        parent = self.get_node_norefresh(el.parent)
        if parent.node is None and parent != self.root:
            return False
        nodes = self.db.list(el.parent, el.tags, el.pos_filter)
        return el.nodes == nodes

    def _checks_ok(self, checks):
        handlers = {
            check.CheckGone: self._check_ok_gone,
            check.CheckParent: self._check_ok_parent,
            check.CheckPos: self._check_ok_pos,
            check.CheckTags: self._check_ok_tags,
            check.CheckTag: self._check_ok_tag,
            check.CheckAttr: self._check_ok_attr,
            check.CheckData: self._check_ok_data,
            check.CheckBinDataSize: self._check_ok_bindata_size,
            check.CheckBinData: self._check_ok_bindata,
            check.CheckList: self._check_ok_list,
        }
        for ch in checks:
            if not handlers[type(ch)](ch):
                return False
        return True

    def _op_create(self, xact, op, node):
        if node.node is not None:
            raise ObjectExistsError()
        parent = self.get_node_norefresh(node.parent)
        if parent.node is None and parent != self.root:
            raise ObjectGoneError()
        node.node = Node(
            id=node.id, parent=op.parent,
            pos_start=op.pos_start, pos_end=op.pos_end,
            tags=op.tags, attr=op.attr, data=set(op.data),
            bindata={x: len(y) for x, y in op.bindata.items()}
        )
        self.db.create(node.node, commit=False)
        node.parent = parent
        for key, val in op.data.items():
            self.db.set_data(node.id, key, val)
            xact.set_data(node, key, val)
        for key, val in op.bindata.items():
            for sub in node.bindata_subs.get(key, set()):
                xact.bindata_changed(sub)

    def _op_delete(self, xact, op, node):
        if node.node is None:
            return
        for oid in self.db.list(node.id):
            subnode = self.get_node_norefresh(oid)
            xact.save(subnode)
            self._op_delete(xact, op, subnode)
        self.db.delete(self.id, commit=False)
        node.node = None
        node.parent = None

    def _op_set_parent(self, xact, op, node):
        if node.node is None:
            raise ObjectGoneError()
        parent = self.get_node_norefresh(op.parent)
        if parent.node is None and parent != self.root:
            raise ObjectGoneError()
        if parent.id == node.parent.id:
            return
        cur = parent
        while cur.id != NodeID.root_id:
            if cur.id == node.id:
                raise ParentCycleError()
            cur = cur.parent
        self.db.set_parent(node.id, parent.id, commit=False)
        node.node.parent = parent.id
        node.parent = parent

    def _op_set_pos(self, xact, op, node):
        if node.node is None:
            raise ObjectGoneError()
        if op.pos_start == node.pos_start and op.pos_end == node.pos_end:
            return
        self.db.set_pos(node.id, op.pos_start, op.pos_end, commit=False)
        node.node.pos_start = op.pos_start
        node.node.pos_end = op.pos_end

    def _op_add_tag(self, xact, op, node):
        if node.node is None:
            raise ObjectGoneError()
        if op.tag in node.node.tags:
            return
        self.db.add_tag(node.id, op.tag, commit=False)
        node.node.tags.add(op.tag)

    def _op_del_tag(self, xact, op, node):
        if node.node is None:
            raise ObjectGoneError()
        if op.tag not in node.node.tags:
            return
        self.db.del_tag(node.id, op.tag, commit=False)
        node.node.tags.remove(op.tag)

    def _op_set_attr(self, xact, op, node):
        if node.node is None:
            raise ObjectGoneError()
        if node.node.attr.get(op.key) == op.data:
            return
        self.db.set_attr(node.id, op.key, op.data, commit=False)
        if op.data is None:
            del node.node.attr[op.key]
        else:
            node.node.attr[op.key] = op.data

    def _op_set_data(self, xact, op, node):
        if node.node is None:
            raise ObjectGoneError()
        self.db.set_data(node.id, op.key, op.data, commit=False)
        xact.set_data(node, op.key, op.data)
        if op.data is None and op.key in node.node.data:
            node.node.data.remove(op.key)
        elif op.data is not None and op.key not in node.node.data:
            node.node.data.add(op.key)

    def _op_set_bindata(self, xact, op, node):
        if node.node is None:
            raise ObjectGoneError()
        self.db.set_bindata(node.id, op.key, op.start, op.data,
                            op.truncate, commit=False)
        old_len = node.node.bindata.get(op.key, 0)
        if op.truncate:
            new_len = op.start + len(op.data)
            change_end = max(old_len, new_len)
        else:
            new_len = max(op.start + len(op.data), old_len)
            change_end = op.start + len(op.data)
        for sub in node.bindata_subs.get(op.key, ()):
            if sub.end is not None and sub.end <= op.start:
                continue
            if change_end <= sub.start:
                continue
            xact.bindata_changed(sub)
        if old_len != new_len:
            if new_len:
                node.node.bindata[op.key] = new_len
            else:
                del node.node.bindata[op.key]

    def transaction(self, checks, ops):
        if not self._checks_ok(checks):
            return bad_future(PreconditionFailedError())
        try:
            with Transaction(self) as xact:
                handlers = {
                    operation.OperationCreate: self._op_create,
                    operation.OperationDelete: self._op_delete,
                    operation.OperationSetParent: self._op_set_parent,
                    operation.OperationSetPos: self._op_set_pos,
                    operation.OperationAddTag: self._op_add_tag,
                    operation.OperationDelTag: self._op_del_tag,
                    operation.OperationSetAttr: self._op_set_attr,
                    operation.OperationSetData: self._op_set_data,
                    operation.OperationSetBinData: self._op_set_bindata,
                }
                for op in ops:
                    node = self.get_node_norefresh(op.node)
                    xact.save(node)
                    handlers[type(op)](xact, op, node)
        except VelesException as err:
            return bad_future(err)
        else:
            return done_future(None)

    def run_broadcast_raw(self, broadcast, params):
        aresults = []
        for handler in self.broadcasts.get(broadcast, []):
            aresults.append(handler.run_broadcast(self, params))

        async def get_results():
            results = []
            for ares in aresults:
                results += await ares
            return results

        loop = asyncio.get_event_loop()
        return loop.create_task(get_results())

    def register_plugin_handler(self, handler):
        if isinstance(handler, MethodHandler):
            self.methods.register(handler.method, handler.tags, handler)
        elif isinstance(handler, QueryHandler):
            self.queries.register(handler.query, handler.tags, handler)
        elif isinstance(handler, BroadcastHandler):
            self.broadcasts.setdefault(handler.broadcast, set()).add(handler)
        else:
            raise TypeError('unknown type of plugin handler')

    def unregister_plugin_handler(self, handler):
        if isinstance(handler, MethodHandler):
            self.methods.unregister(handler.method, handler.tags, handler)
        elif isinstance(handler, QueryHandler):
            self.queries.unregister(handler.query, handler.tags, handler)
        elif isinstance(handler, BroadcastHandler):
            self.broadcasts[handler.broadcast].remove(handler)
        else:
            raise TypeError('unknown type of plugin handler')
