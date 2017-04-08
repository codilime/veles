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

from veles.proto.exceptions import (
    VelesException,
    ObjectGoneError,
)


class DbNode(object):
    def __init__(self, tracker, id, node, parent):
        self.tracker = tracker
        self.id = id
        self.node = node
        self.parent = parent
        self.subs = set()
        self.data_subs = {}
        self.bindata_subs = {}
        self.list_subs = {}

    # subscriptions

    def _add_sub(self, sub):
        self.subs.add(sub)
        if self.node is not None:
            sub.node_changed(self.node)
        else:
            sub.error(ObjectGoneError())

    def _del_sub(self, sub):
        self.subs.remove(sub)

    def _add_sub_list(self, sub):
        try:
            nids = self.tracker.get_list_raw(self.id, sub.tags, sub.pos_filter)
            dbnodes = {self.tracker.get_cached_node(x) for x in nids}
            self.list_subs[sub] = dbnodes
            sub.list_changed([dbnode.node for dbnode in dbnodes], [])
        except VelesException as e:
            self.list_subs[sub] = None
            sub.error(e)

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
            sub.data_changed(self.tracker.get_data(self.id, sub.key))

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
            sub.bindata_changed(self.tracker.get_bindata(
                self.id, sub.key, sub.start, sub.end))

    def _del_sub_bindata(self, sub):
        self.bindata_subs[sub.key].remove(sub)
        if not self.bindata_subs[sub.key]:
            del self.bindata_subs[sub.key]
