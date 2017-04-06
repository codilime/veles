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

from veles.proto import check
from veles.proto.exceptions import VelesException
from veles.async_conn.subscriber import (
    BaseSubscriber,
    BaseSubscriberData,
    BaseSubscriberBinData,
    BaseSubscriberList,
)


class CheckGoneSubscriber(BaseSubscriber):
    def __init__(self, obj, manager):
        self.manager = manager
        super().__init__(obj)

    def object_changed(self):
        self.manager.invalidate()

    def error(self, err):
        pass


class CheckNodeSubscriber(BaseSubscriber):
    def __init__(self, obj, manager):
        self.manager = manager
        self.checks = []
        super().__init__(obj)

    def object_changed(self):
        if not self.obj.conn._checks_ok(self.checks):
            self.manager.invalidate()

    def error(self, err):
        self.manager.invalidate()


class CheckSubscriberData(BaseSubscriberData):
    def __init__(self, obj, check, manager):
        self.check = check
        self.manager = manager
        super().__init__(obj, check.key)

    def data_changed(self, data):
        if data != self.check.data:
            self.manager.invalidate()

    def error(self, err):
        self.manager.invalidate()


class CheckSubscriberBinData(BaseSubscriberBinData):
    def __init__(self, obj, check, manager):
        self.check = check
        self.manager = manager
        super().__init__(obj, check.key, check.start, check.end)

    def bindata_changed(self, data):
        if data != self.check.data:
            self.manager.invalidate()

    def error(self, err):
        self.manager.invalidate()


class CheckSubscriberList(BaseSubscriberList):
    def __init__(self, obj, check, manager):
        self.check = check
        self.manager = manager
        super().__init__(obj, check.tags, check.pos_filter)

    def list_changed(self, changed, gone):
        if gone or any(node.id not in self.check.nodes for node in changed):
            self.manager.invalidate()

    def error(self, err):
        self.manager.invalidate()


class QueryManager:
    def __init__(self, sub):
        self.sub = sub
        self.id = sub.obj.id
        self.conn = sub.obj.conn
        self.idle = True
        self.cancelled = False
        self.check_subs = set()
        self.invalidate()

    def invalidate(self):
        if self.cancelled or not self.idle:
            return
        self.idle = False
        for sub in self.check_subs:
            sub.cancel()
        self.check_subs = set()
        loop = asyncio.get_event_loop()
        return loop.create_task(self.refresh())

    def cancel(self):
        self.cancelled = True
        for sub in self.check_subs:
            sub.cancel()
        self.check_subs = set()

    def rearm(self, checks):
        self.idle = True
        checks_gone = {}
        checks_node = {}
        checks_data = {}
        for ch in checks:
            if isinstance(ch, check.CheckGone):
                if ch.node not in checks_gone:
                    anode = self.conn.get_node_norefresh(ch.node)
                    sub = CheckGoneSubscriber(anode, self)
                    self.check_subs.add(sub)
                    checks_gone[ch.node] = sub
            elif isinstance(ch, (
                check.CheckParent,
                check.CheckPos,
                check.CheckTags,
                check.CheckTag,
                check.CheckAttr,
                check.CheckBinDataSize,
            )):
                if ch.node not in checks_node:
                    anode = self.conn.get_node_norefresh(ch.node)
                    sub = CheckNodeSubscriber(anode, self)
                    self.check_subs.add(sub)
                    checks_node[ch.node] = sub
                checks_node[ch.node].checks.append(ch)
            elif isinstance(ch, check.CheckData):
                if (ch.node, ch.key) not in checks_data:
                    anode = self.conn.get_node_norefresh(ch.node)
                    sub = CheckSubscriberData(anode, ch, self)
                    self.check_subs.add(sub)
                    checks_data[ch.node, ch.key] = sub
            elif isinstance(ch, check.CheckBinData):
                anode = self.conn.get_node_norefresh(ch.node)
                sub = CheckSubscriberBinData(anode, ch, self)
                self.check_subs.add(sub)
            elif isinstance(ch, check.CheckList):
                anode = self.conn.get_node_norefresh(ch.parent)
                sub = CheckSubscriberList(anode, ch, self)
                self.check_subs.add(sub)
            else:
                assert False

    async def refresh(self):
        checks = []
        try:
            result = await self.conn._get_query_raw(
                self.id, self.sub.name, self.sub.params, checks)
        except VelesException as e:
            if self.cancelled:
                return
            self.rearm(checks)
            self.sub.error(e, checks)
        else:
            if self.cancelled:
                return
            self.rearm(checks)
            self.sub.raw_result_changed(result, checks)
