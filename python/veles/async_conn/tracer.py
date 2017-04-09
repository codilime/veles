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

from veles.proto.node import PosFilter
from veles.proto import check
from veles.proto.exceptions import ObjectGoneError
from veles.util.future import done_future


class AsyncTracer:
    def __init__(self, conn):
        self.conn = conn
        self.checks = []
        self.nodes = {}
        self.node_data = {}

    def _get_node(self, id):
        if id not in self.nodes:
            self.nodes[id] = self.conn.get(id)
        return self.nodes[id]

    def _inject_node(self, node):
        if node.id not in self.nodes:
            self.nodes[node.id] = done_future(node)

    def _get_from_node(self, id, func):
        anode = self._get_node(id)

        async def inner():
            try:
                node = await anode
            except ObjectGoneError:
                self.checks.append(check.CheckGone(
                    node=id,
                ))
                raise
            else:
                return func(node)

        loop = asyncio.get_event_loop()
        return loop.create_task(inner())

    def _get_parent(self, node):
        self.checks.append(check.CheckParent(
            node=node.id,
            parent=node.parent
        ))
        return node.parent

    def get_parent(self, id):
        return self._get_from_node(id, self._get_parent)

    def _get_pos(self, node):
        self.checks.append(check.CheckPos(
            node=node.id,
            pos_start=node.pos_start,
            pos_end=node.pos_end,
        ))
        return node.pos_start, node.pos_end

    def get_pos(self, id):
        return self._get_from_node(id, self._get_pos)

    def _get_tags(self, node):
        self.checks.append(check.CheckTags(
            node=node.id,
            tags=node.tags,
        ))
        return node.tags

    def get_tags(self, id):
        return self._get_from_node(id, self._get_tags)

    def _has_tag(self, node, tag):
        res = tag in node.tags
        self.checks.append(check.CheckTag(
            node=node.id,
            tag=tag,
            present=res,
        ))
        return res

    def has_tag(self, id, tag):
        return self._get_from_node(id, lambda node: self._has_tag(node, tag))

    def _get_attr(self, node, key):
        res = node.attr.get(key)
        self.checks.append(check.CheckAttr(
            node=node.id,
            key=key,
            data=res,
        ))
        return res

    def get_attr(self, id, key):
        return self._get_from_node(id, lambda node: self._get_attr(node, key))

    def _get_bindata_size(self, node, key):
        res = node.bindata.get(key, 0)
        self.checks.append(check.CheckBinDataSize(
            node=node.id,
            key=key,
            size=res,
        ))
        return res

    def get_bindata_size(self, id, key):
        return self._get_from_node(
            id, lambda node: self._get_bindata_size(node, key))

    def _get_trigger(self, node, key):
        res = node.triggers.get(key)
        self.checks.append(check.CheckTrigger(
            node=node.id,
            key=key,
            state=res,
        ))
        return res

    def get_trigger(self, id, key):
        return self._get_from_node(
            id, lambda node: self._get_trigger(node, key))

    async def _get_data(self, node, key, adata):
        try:
            res = await adata
        except ObjectGoneError:
            self.checks.append(check.CheckGone(
                node=node,
            ))
            raise
        else:
            self.checks.append(check.CheckData(
                node=node,
                key=key,
                data=res,
            ))
            return res

    def get_data(self, node, key):
        if (node, key) not in self.node_data:
            adata = self.conn.get_data(node, key)
            loop = asyncio.get_event_loop()
            task = loop.create_task(self._get_data(node, key, adata))
            self.node_data[node, key] = task
        return self.node_data[node, key]

    async def _get_bindata(self, node, key, start, end, adata):
        try:
            res = await adata
        except ObjectGoneError:
            self.checks.append(check.CheckGone(
                node=node,
            ))
            raise
        else:
            self.checks.append(check.CheckBinData(
                node=node,
                key=key,
                start=start,
                end=end,
                data=res,
            ))
            return res

    def get_bindata(self, node, key, start=0, end=None):
        adata = self.conn.get_bindata(node, key, start, end)
        loop = asyncio.get_event_loop()
        return loop.create_task(
            self._get_bindata(node, key, start, end, adata))

    async def _get_list(self, parent, tags, pos_filter, ares):
        try:
            res = await ares
        except ObjectGoneError:
            self.checks.append(check.CheckGone(
                node=parent,
            ))
            raise
        else:
            self.checks.append(check.CheckList(
                parent=parent,
                tags=tags,
                pos_filter=pos_filter,
                nodes={x.id for x in res},
            ))
            for n in res:
                self._inject_node(n)
            return [x.id for x in res]

    def get_list(self, parent, tags=set(), pos_filter=PosFilter()):
        ares = self.conn.get_list(parent, tags, pos_filter)
        loop = asyncio.get_event_loop()
        return loop.create_task(self._get_list(parent, tags, pos_filter, ares))

    def get_query(self, node, sig, params):
        return self.conn.get_query(node, sig, params, self.checks)
