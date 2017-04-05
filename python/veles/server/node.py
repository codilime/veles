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

import asyncio

from veles.util.future import done_future, bad_future
from veles.async_conn.node import AsyncNode
from veles.proto.exceptions import (
    VelesException,
    ObjectGoneError,
)
from veles.proto.check import CheckGone, CheckTags
from veles.proto.node import PosFilter
from veles.schema.nodeid import NodeID

from .query import QueryManager


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
        self.bindata_subs = {}
        self.list_subs = {}
        self.query_subs = {}

    # getters

    def refresh(self):
        if self.node is None:
            return bad_future(ObjectGoneError())
        return done_future(self)

    def get_data(self, key):
        if self.node is None:
            return bad_future(ObjectGoneError())
        return done_future(self.conn.db.get_data(self.id, key))

    def get_bindata(self, key, start, end):
        if self.node is None:
            return bad_future(ObjectGoneError())
        return done_future(self.conn.db.get_bindata(self.id, key, start, end))

    def get_list(self, tags=frozenset(), pos_filter=PosFilter()):
        if self.node is None and self != self.conn.root:
            return bad_future(ObjectGoneError())
        obj_ids = self.conn.db.list(self.id, tags, pos_filter)
        objs = [self.conn.get_node_norefresh(x) for x in obj_ids]
        return done_future(objs)

    async def _get_query_raw(self, name, params, checks):
        while True:
            if self.node is None:
                if checks is not None:
                    checks.append(CheckGone(
                        node=self.id
                    ))
                raise ObjectGoneError()
            cur_checks = [CheckTags(
                node=self.id,
                tags=self.node.tags,
            )]
            try:
                handler = self.conn.queries.find(name, self.node.tags)
                result = await handler.get_query(
                    self.conn, self, params, cur_checks)
            except VelesException:
                if self.conn._checks_ok(cur_checks):
                    if checks is not None:
                        checks += cur_checks
                    raise
            else:
                if self.conn._checks_ok(cur_checks):
                    if checks is not None:
                        checks += cur_checks
                    return result

    def get_query_raw(self, name, params, checks=None):
        loop = asyncio.get_event_loop()
        return loop.create_task(self._get_query_raw(name, params, checks))

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

    def _add_sub_list(self, sub):
        obj_ids = self.conn.db.list(self.id, sub.tags, sub.pos_filter)
        objs = {self.conn.get_node_norefresh(x) for x in obj_ids}
        self.list_subs[sub] = objs
        self.conn.all_subs.add(sub)
        if self.node is not None or self.id == NodeID.root_id:
            sub.list_changed(objs, [])
        else:
            sub.error(ObjectGoneError())

    def _del_sub_list(self, sub):
        del self.list_subs[sub]
        self.conn.all_subs.remove(sub)

    # data sub

    def _add_sub_data(self, sub):
        if sub.key not in self.data_subs:
            self.data_subs[sub.key] = set()
        self.data_subs[sub.key].add(sub)
        self.conn.all_subs.add(sub)
        if self.node is None:
            sub.error(ObjectGoneError())
        else:
            sub.data_changed(self.conn.db.get_data(self.id, sub.key))

    def _del_sub_data(self, sub):
        self.data_subs[sub.key].remove(sub)
        if not self.data_subs[sub.key]:
            del self.data_subs[sub.key]
        self.conn.all_subs.remove(sub)

    # bindata sub

    def _add_sub_bindata(self, sub):
        if sub.key not in self.bindata_subs:
            self.bindata_subs[sub.key] = set()
        self.bindata_subs[sub.key].add(sub)
        self.conn.all_subs.add(sub)
        if self.node is None:
            sub.error(ObjectGoneError())
        else:
            sub.bindata_changed(self.conn.db.get_bindata(
                self.id, sub.key, sub.start, sub.end))

    def _del_sub_bindata(self, sub):
        self.bindata_subs[sub.key].remove(sub)
        if not self.bindata_subs[sub.key]:
            del self.bindata_subs[sub.key]
        self.conn.all_subs.remove(sub)

    def _add_sub_query(self, sub):
        self.query_subs[sub] = QueryManager(sub)
        self.conn.all_subs.add(sub)

    def _del_sub_query(self, sub):
        self.query_subs[sub].cancel()
        del self.query_subs[sub]
        self.conn.all_subs.remove(sub)

    # mutators

    def run_method_raw(self, method, params):
        if self.node is None:
            return bad_future(ObjectGoneError())
        try:
            handler = self.conn.methods.find(method, self.node.tags)
        except VelesException as e:
            return bad_future(e)
        return handler.run_method(self.conn, self.node, params)
