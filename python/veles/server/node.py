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

from veles.util.future import done_future, bad_future
from veles.async_conn.node import AsyncNode
from veles.proto.exceptions import ObjectGoneError


class AsyncLocalNode(AsyncNode):
    def __init__(self, conn, id, node, parent):
        super().__init__(conn, id, node)
        self.parent = parent
        self.subs = set()
        self.data_subs = {}
        self.listers = set()

    # getters

    def refresh(self):
        if self.node is None:
            return bad_future(ObjectGoneError())
        return done_future(self)

    # subscriptions

    def _add_sub(self, sub):
        self.subs.add(sub)
        self.conn.all_subs.add(sub)
        if self.node is not None:
            sub.object_changed()
        else:
            sub.error(ObjectGoneError())

    def _del_sub(self, sub):
        self.subs.remove(sub)
        self.conn.all_subs.remove(sub)

    # misc

    def send_subs(self):
        for sub in self.subs:
            sub.object_changed(self)

    def send_data_subs(self, key, data):
        for sub in self.data_subs.get(key, ()):
            sub.data_changed(self, data)

    def clear_subs(self):
        for sub in self.subs:
            sub.error(ObjectGoneError())
        for k, v in self.data_subs.items():
            for sub in v:
                sub.obj_gone()
        for lister in self.listers:
            lister.obj_gone()

    def remove_data_sub(self, sub):
        self.data_subs[sub.key].remove(sub)

    def add_data_sub(self, sub):
        if sub.key not in self.data_subs:
            self.data_subs[sub.key] = set()
        self.data_subs[sub.key].add(sub)
