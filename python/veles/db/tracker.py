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

import weakref

try:
    from functools import lru_cache
except ImportError:
    from backports.functools_lru_cache import lru_cache

from veles.schema.nodeid import NodeID
from veles.proto import operation, check
from veles.proto.node import TriggerState, Node, PosFilter
from veles.proto.exceptions import (
    ObjectGoneError,
    ObjectExistsError,
    ParentCycleError,
    PreconditionFailedError,
)

from .subscriber import (
    BaseSubscriberNode,
    BaseSubscriberData,
    BaseSubscriberBinData,
    BaseSubscriberList,
)
from .backend import DbBackend
from .transaction import Transaction
from .node import DbNode


DB_CACHE_SIZE = 128


class DbTracker(object):
    def __init__(self, db):
        if not isinstance(db, DbBackend):
            db = DbBackend(db)
        self.db = db
        # all_subs is a dict mapping all non-query subscriptions in existence
        # to their DbNodes - this exists
        # so that all involved objects are strongly referenced (otherwise,
        # being only reacheble through the weak dict, they could be GCd along
        # with all their subscriptions).
        self.all_subs = {}
        # WTF H4X: this strange construction does two things:
        #
        # - the self.nodes dictionary ensures there can be at most one DbNode
        #   for every node, so there is only one place to invalidate.
        # - the lru_cache on get_cached_node ensures at least 128 last nodes
        #   are actually kept alive.
        self.nodes = weakref.WeakValueDictionary()
        self.get_cached_node = lru_cache(maxsize=DB_CACHE_SIZE)(
            self._get_cached_node)
        self.triggers_gone_callbacks = set()

    def _get_cached_node(self, nid):
        try:
            return self.nodes[nid]
        except KeyError:
            node = self.db.get(nid)
            if not node:
                res = DbNode(self, nid, None, None)
            else:
                parent = self.get_cached_node(node.parent)
                assert parent.node is not None or parent.id == NodeID.root_id
                res = DbNode(self, nid, node, parent)
            self.nodes[nid] = res
            return res

    def get(self, nid):
        dbnode = self.get_cached_node(nid)
        if dbnode.node is None:
            raise ObjectGoneError()
        return dbnode.node

    def get_data(self, nid, key):
        dbnode = self.get_cached_node(nid)
        if dbnode.node is None:
            raise ObjectGoneError()
        return self.db.get_data(nid, key)

    def get_bindata(self, nid, key, start=0, end=None):
        dbnode = self.get_cached_node(nid)
        if dbnode.node is None:
            raise ObjectGoneError()
        return self.db.get_bindata(nid, key, start, end)

    def get_list_raw(self, parent, tags=frozenset(), pos_filter=PosFilter()):
        if parent != NodeID.root_id:
            dbnode = self.get_cached_node(parent)
            if dbnode.node is None:
                raise ObjectGoneError()
        return self.db.list(parent, tags, pos_filter)

    def get_list(self, parent, tags=frozenset(), pos_filter=PosFilter()):
        return [
            self.get_cached_node(nid).node
            for nid in self.get_list_raw(parent, tags, pos_filter)
        ]

    def _check_ok_gone(self, el):
        dbnode = self.get_cached_node(el.node)
        return dbnode.node is None

    def _check_ok_parent(self, el):
        node = self.get(el.node)
        return node.parent == el.parent

    def _check_ok_pos(self, el):
        node = self.get(el.node)
        return node.pos_start == el.pos_start and node.pos_end == el.pos_end

    def _check_ok_tags(self, el):
        node = self.get(el.node)
        return node.tags == el.tags

    def _check_ok_tag(self, el):
        node = self.get(el.node)
        return (el.tag in node.tags) == el.present

    def _check_ok_attr(self, el):
        node = self.get(el.node)
        return node.attr.get(el.key) == el.data

    def _check_ok_data(self, el):
        return self.get_data(el.node, el.key) == el.data

    def _check_ok_bindata_size(self, el):
        node = self.get(el.node)
        return node.bindata.get(el.key, 0) == el.size

    def _check_ok_bindata(self, el):
        data = self.get_bindata(el.node, el.key, el.start, el.end)
        return data == el.data

    def _check_ok_trigger(self, el):
        node = self.get(el.node)
        return node.triggers.get(el.key) == el.state

    def _check_ok_list(self, el):
        nodes = self.get_list_raw(el.parent, el.tags, el.pos_filter)
        return el.nodes == nodes

    def checks_ok(self, checks):
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
            check.CheckTrigger: self._check_ok_trigger,
            check.CheckList: self._check_ok_list,
        }
        try:
            for ch in checks:
                if not handlers[type(ch)](ch):
                    return False
        except ObjectGoneError:
            return False
        return True

    def _op_create(self, xact, op, dbnode):
        if dbnode.node is not None:
            raise ObjectExistsError()
        parent = self.get_cached_node(op.parent)
        if parent.node is None and parent.id != NodeID.root_id:
            raise ObjectGoneError()
        dbnode.node = Node(
            id=dbnode.id, parent=op.parent,
            pos_start=op.pos_start, pos_end=op.pos_end,
            tags=op.tags, attr=op.attr, data=set(op.data),
            bindata={x: len(y) for x, y in op.bindata.items()},
            triggers={x: TriggerState.pending for x in op.triggers},
        )
        self.db.create(dbnode.node)
        dbnode.parent = parent
        for key, val in op.data.items():
            self.db.set_data(dbnode.id, key, val)
            xact.set_data(dbnode, key, val)
        for key, val in op.bindata.items():
            self.db.set_bindata(dbnode.id, key, 0, val)
            for sub in dbnode.bindata_subs.get(key, set()):
                xact.bindata_changed(sub)
        for key in op.triggers:
            self.db.add_trigger(dbnode.id, key)

    def _op_delete(self, xact, op, dbnode):
        if dbnode.node is None:
            return
        for oid in self.db.list(dbnode.id):
            subnode = self.get_cached_node(oid)
            xact.save(subnode)
            self._op_delete(xact, op, subnode)
        xact.triggers_gone |= self.db.delete(dbnode.id)
        dbnode.node = None
        dbnode.parent = None

    def _op_set_parent(self, xact, op, dbnode):
        if dbnode.node is None:
            raise ObjectGoneError()
        parent = self.get_cached_node(op.parent)
        if parent.node is None and parent.id != NodeID.root_id:
            raise ObjectGoneError()
        if parent.id == dbnode.parent.id:
            return
        cur = parent
        while cur.id != NodeID.root_id:
            if cur.id == dbnode.id:
                raise ParentCycleError()
            cur = cur.parent
        self.db.set_parent(dbnode.id, parent.id)
        dbnode.node.parent = parent.id
        dbnode.parent = parent

    def _op_set_pos(self, xact, op, dbnode):
        if dbnode.node is None:
            raise ObjectGoneError()
        if (op.pos_start == dbnode.node.pos_start and
                op.pos_end == dbnode.node.pos_end):
            return
        self.db.set_pos(dbnode.id, op.pos_start, op.pos_end)
        dbnode.node.pos_start = op.pos_start
        dbnode.node.pos_end = op.pos_end

    def _op_add_tag(self, xact, op, dbnode):
        if dbnode.node is None:
            raise ObjectGoneError()
        if op.tag in dbnode.node.tags:
            return
        self.db.add_tag(dbnode.id, op.tag)
        dbnode.node.tags.add(op.tag)

    def _op_del_tag(self, xact, op, dbnode):
        if dbnode.node is None:
            raise ObjectGoneError()
        if op.tag not in dbnode.node.tags:
            return
        self.db.del_tag(dbnode.id, op.tag)
        dbnode.node.tags.remove(op.tag)

    def _op_set_attr(self, xact, op, dbnode):
        if dbnode.node is None:
            raise ObjectGoneError()
        if dbnode.node.attr.get(op.key) == op.data:
            return
        self.db.set_attr(dbnode.id, op.key, op.data)
        if op.data is None:
            del dbnode.node.attr[op.key]
        else:
            dbnode.node.attr[op.key] = op.data

    def _op_set_data(self, xact, op, dbnode):
        if dbnode.node is None:
            raise ObjectGoneError()
        xact.set_data(dbnode, op.key, op.data)
        self.db.set_data(dbnode.id, op.key, op.data)
        if op.data is None and op.key in dbnode.node.data:
            dbnode.node.data.remove(op.key)
        elif op.data is not None and op.key not in dbnode.node.data:
            dbnode.node.data.add(op.key)

    def _op_set_bindata(self, xact, op, dbnode):
        if dbnode.node is None:
            raise ObjectGoneError()
        self.db.set_bindata(dbnode.id, op.key, op.start, op.data,
                            op.truncate)
        old_len = dbnode.node.bindata.get(op.key, 0)
        if op.truncate:
            new_len = op.start + len(op.data)
            change_end = max(old_len, new_len)
        else:
            new_len = max(op.start + len(op.data), old_len)
            change_end = op.start + len(op.data)
        for sub in dbnode.bindata_subs.get(op.key, ()):
            if sub.end is not None and sub.end <= op.start:
                continue
            if change_end <= sub.start:
                continue
            xact.bindata_changed(sub)
        if old_len != new_len:
            if new_len:
                dbnode.node.bindata[op.key] = new_len
            else:
                del dbnode.node.bindata[op.key]

    def _op_add_trigger(self, xact, op, dbnode):
        if dbnode.node is None:
            raise ObjectGoneError()
        if op.trigger in dbnode.node.triggers:
            return
        self.db.add_trigger(dbnode.id, op.trigger)
        dbnode.node.triggers[op.trigger] = TriggerState.pending

    def _op_del_trigger(self, xact, op, dbnode):
        if dbnode.node is None:
            raise ObjectGoneError()
        if op.trigger not in dbnode.node.triggers:
            return
        tid = self.db.del_trigger(dbnode.id, op.trigger)
        assert tid is not None
        del dbnode.node.triggers[op.trigger]
        xact.triggers_gone.add(tid)

    def transaction(self, checks, ops):
        if not self.checks_ok(checks):
            raise PreconditionFailedError()
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
                operation.OperationAddTrigger: self._op_add_trigger,
                operation.OperationDelTrigger: self._op_del_trigger,
            }
            for op in ops:
                dbnode = self.get_cached_node(op.node)
                xact.save(dbnode)
                handlers[type(op)](xact, op, dbnode)

    def _triggers_gone(self, tids):
        if not tids:
            return
        for cb in self.triggers_gone_callbacks:
            cb(tids)

    def register_triggers_gone_callback(self, cb):
        self.triggers_gone_callbacks.add(cb)

    def unregister_triggers_gone_callback(self, cb):
        self.triggers_gone_callbacks.remove(cb)

    # subscribers

    def register_subscriber(self, sub):
        if isinstance(sub, BaseSubscriberNode):
            dbnode = self.get_cached_node(sub.node)
            dbnode._add_sub(sub)
            self.all_subs[sub] = dbnode
        elif isinstance(sub, BaseSubscriberData):
            dbnode = self.get_cached_node(sub.node)
            dbnode._add_sub_data(sub)
            self.all_subs[sub] = dbnode
        elif isinstance(sub, BaseSubscriberBinData):
            dbnode = self.get_cached_node(sub.node)
            dbnode._add_sub_bindata(sub)
            self.all_subs[sub] = dbnode
        elif isinstance(sub, BaseSubscriberList):
            dbnode = self.get_cached_node(sub.parent)
            dbnode._add_sub_list(sub)
            self.all_subs[sub] = dbnode
        else:
            raise TypeError('unknown type of subscription')

    def unregister_subscriber(self, sub):
        if isinstance(sub, BaseSubscriberNode):
            dbnode = self.get_cached_node(sub.node)
            dbnode._del_sub(sub)
            del self.all_subs[sub]
        elif isinstance(sub, BaseSubscriberData):
            dbnode = self.get_cached_node(sub.node)
            dbnode._del_sub_data(sub)
            del self.all_subs[sub]
        elif isinstance(sub, BaseSubscriberBinData):
            dbnode = self.get_cached_node(sub.node)
            dbnode._del_sub_bindata(sub)
            del self.all_subs[sub]
        elif isinstance(sub, BaseSubscriberList):
            dbnode = self.get_cached_node(sub.parent)
            dbnode._del_sub_list(sub)
            del self.all_subs[sub]
        else:
            raise TypeError('unknown type of subscription')
