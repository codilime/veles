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


class AsyncLocalNode:
    def __init__(self, conn, parent, node):
        self.conn = conn
        self.parent = parent
        self.node = node
        self.subs = set()
        self.data_subs = {}
        self.listers = set()

    def send_subs(self):
        for sub in self.subs:
            sub.obj_changed(self)

    def send_data_subs(self, key, data):
        for sub in self.data_subs.get(key, ()):
            sub.data_changed(self, data)

    def clear_subs(self):
        for sub in self.subs:
            sub.obj_gone()
        for k, v in self.data_subs.items():
            for sub in v:
                sub.obj_gone()
        for lister in self.listers:
            lister.obj_gone()

    def remove_sub(self, sub):
        self.subs.remove(sub)

    def add_sub(self, sub):
        self.subs.add(sub)

    def remove_data_sub(self, sub):
        self.data_subs[sub.key].remove(sub)

    def add_data_sub(self, sub):
        if sub.key not in self.data_subs:
            self.data_subs[sub.key] = set()
        self.data_subs[sub.key].add(sub)
