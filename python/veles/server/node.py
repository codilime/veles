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

from veles.async_conn.node import AsyncNode
from veles.proto.exceptions import (
    ObjectGoneError,
)
from veles.schema.nodeid import NodeID


class AsyncLocalNode(AsyncNode):
    def __init__(self, conn, id, node, parent):
        super().__init__(conn, id, node)
        self.parent = parent
        self.subs = set()
        self.data_subs = {}
        self.bindata_subs = {}
        self.list_subs = {}

    # subscriptions

    def _add_sub(self, sub):
        self.subs.add(sub)
        self._send_sub(sub)

    def _del_sub(self, sub):
        self.subs.remove(sub)

    def _send_sub(self, sub):
        if self.node is not None:
            sub.node_changed(self.node)
        else:
            sub.error(ObjectGoneError())

    def _add_sub_list(self, sub):
        obj_ids = self.conn.db.list(self.id, sub.tags, sub.pos_filter)
        objs = {self.conn.get_node_norefresh(x) for x in obj_ids}
        self.list_subs[sub] = objs
        if self.node is not None or self.id == NodeID.root_id:
            sub.list_changed([obj.node for obj in objs], [])
        else:
            sub.error(ObjectGoneError())

    def _del_sub_list(self, sub):
        del self.list_subs[sub]

    # data sub

    def _add_sub_data(self, sub):
        if sub.key not in self.data_subs:
            self.data_subs[sub.key] = set()
        self.data_subs[sub.key].add(sub)
        if self.node is None:
            sub.error(ObjectGoneError())
        else:
            sub.data_changed(self.conn.db.get_data(self.id, sub.key))

    def _del_sub_data(self, sub):
        self.data_subs[sub.key].remove(sub)
        if not self.data_subs[sub.key]:
            del self.data_subs[sub.key]

    # bindata sub

    def _add_sub_bindata(self, sub):
        if sub.key not in self.bindata_subs:
            self.bindata_subs[sub.key] = set()
        self.bindata_subs[sub.key].add(sub)
        if self.node is None:
            sub.error(ObjectGoneError())
        else:
            sub.bindata_changed(self.conn.db.get_bindata(
                self.id, sub.key, sub.start, sub.end))

    def _del_sub_bindata(self, sub):
        self.bindata_subs[sub.key].remove(sub)
        if not self.bindata_subs[sub.key]:
            del self.bindata_subs[sub.key]
