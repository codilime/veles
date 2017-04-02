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

from collections import defaultdict

from veles.util.future import done_future, bad_future
from veles.db.tracker import DbTracker
from veles.proto import check
from veles.proto.node import PosFilter
from veles.proto.exceptions import (
    VelesException,
    RegistryNoMatchError,
    RegistryMultiMatchError,
)
from veles.async_conn.plugin import (
    MethodHandler, QueryHandler, BroadcastHandler, TriggerHandler
)
from veles.async_conn.subscriber import BaseSubscriberQuery
from veles.async_conn.conn import AsyncConnection

from .query import QueryManager


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
    def __init__(self, loop, tracker):
        self.loop = loop
        if not isinstance(tracker, DbTracker):
            tracker = DbTracker(tracker)
        self.tracker = tracker
        self.conns = {}
        self.next_cid = 0
        self.methods = NameTagsRegistry()
        self.queries = NameTagsRegistry()
        self.triggers = NameTagsRegistry()
        self.broadcasts = {}
        self.query_subs = {}
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

    # getters

    def get(self, nid):
        try:
            return done_future(self.tracker.get(nid))
        except VelesException as e:
            return bad_future(e)

    def get_data(self, nid, key):
        try:
            return done_future(self.tracker.get_data(nid, key))
        except VelesException as e:
            return bad_future(e)

    def get_bindata(self, nid, key, start, end):
        try:
            return done_future(self.tracker.get_bindata(nid, key, start, end))
        except VelesException as e:
            return bad_future(e)

    def get_list(self, parent, tags=frozenset(), pos_filter=PosFilter()):
        try:
            return done_future(self.tracker.get_list(parent, tags, pos_filter))
        except VelesException as e:
            return bad_future(e)

    async def _get_query_raw(self, nid, name, params, checks):
        while True:
            try:
                node = self.tracker.get(nid)
            except VelesException:
                if checks is not None:
                    checks.append(check.CheckGone(
                        node=nid
                    ))
                raise
            cur_checks = [check.CheckTags(
                node=nid,
                tags=node.tags,
            )]
            try:
                handler = self.queries.find(name, node.tags)
                result = await handler.get_query(
                    self, node, params, cur_checks)
            except VelesException:
                if self.tracker.checks_ok(cur_checks):
                    if checks is not None:
                        checks += cur_checks
                    raise
            else:
                if self.tracker.checks_ok(cur_checks):
                    if checks is not None:
                        checks += cur_checks
                    return result

    def get_query_raw(self, nid, name, params, checks=None):
        loop = asyncio.get_event_loop()
        return loop.create_task(self._get_query_raw(
            nid, name, params, checks))

    def transaction(self, checks, ops):
        try:
            self.tracker.transaction(checks, ops)
        except VelesException as e:
            return bad_future(e)
        return done_future(None)

    def run_method_raw(self, nid, method, params):
        try:
            node = self.tracker.get(nid)
            handler = self.methods.find(method, node.tags)
        except VelesException as e:
            return bad_future(e)
        return handler.run_method(self, node, params)

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
        elif isinstance(handler, TriggerHandler):
            self.triggers.register(handler.trigger, handler.tags, handler)
        else:
            raise TypeError('unknown type of plugin handler')

    def unregister_plugin_handler(self, handler):
        if isinstance(handler, MethodHandler):
            self.methods.unregister(handler.method, handler.tags, handler)
        elif isinstance(handler, QueryHandler):
            self.queries.unregister(handler.query, handler.tags, handler)
        elif isinstance(handler, BroadcastHandler):
            self.broadcasts[handler.broadcast].remove(handler)
        elif isinstance(handler, TriggerHandler):
            self.triggers.unregister(handler.trigger, handler.tags, handler)
        else:
            raise TypeError('unknown type of plugin handler')

    # subscribers

    def register_subscriber(self, sub):
        if isinstance(sub, BaseSubscriberQuery):
            self.query_subs[sub] = QueryManager(sub)
        else:
            self.tracker.register_subscriber(sub)

    def unregister_subscriber(self, sub):
        if isinstance(sub, BaseSubscriberQuery):
            self.query_subs[sub].cancel()
            del self.query_subs[sub]
        else:
            self.tracker.unregister_subscriber(sub)
