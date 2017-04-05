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

from copy import deepcopy

from veles.proto.exceptions import (
    ObjectGoneError,
)


class Transaction:
    def __init__(self, conn):
        self.conn = conn
        self.undo = {}
        self.list_changes = {}
        self.data_subs = {}
        self.bindata_subs = set()

    def __enter__(self):
        self.conn.db.begin()
        return self

    def __exit__(self, exc, val, tb):
        if exc is None:
            # Everything alright, let's commit.
            self.conn.db.commit()
            for k, (node, parent) in self.undo.items():
                if k.node == node:
                    continue
                # Send all normal subs.  Also handle all subs for created
                # and removed parents.
                for sub in k.subs:
                    k._send_sub(sub)
                if node is None:
                    # Node has been created.
                    for sub in k.list_subs:
                        self.list_ensure(sub)
                    for key, subs in k.data_subs.items():
                        for sub in subs:
                            if sub not in self.data_subs:
                                sub.data_changed(None)
                    for key, subs in k.bindata_subs.items():
                        for sub in subs:
                            if sub not in self.bindata_subs:
                                sub.bindata_changed(b'')
                if k.node is None:
                    # Node has been deleted.
                    for sub in k.list_subs:
                        sub.error(ObjectGoneError())
                    for key, subs in k.data_subs.items():
                        for sub in subs:
                            sub.error(ObjectGoneError())
                    for key, subs in k.bindata_subs.items():
                        for sub in subs:
                            sub.error(ObjectGoneError())
                # Unlist objects removed from parents.
                if k.parent != parent and parent is not None:
                    for sub, objs in parent.list_subs.items():
                        if k in objs:
                            objs.remove(k)
                            self.list_remove(sub, k)
                # Update (current) parent's listers.
                if k.parent is not None:
                    for sub, objs in k.parent.list_subs.items():
                        if sub.matches(k.node):
                            objs.add(k)
                            self.list_add(sub, k)
                        elif k in objs:
                            objs.remove(k)
                            self.list_remove(sub, k)
            # Now send out all buffered list changes.
            for sub, (objs, gone) in self.list_changes.items():
                # Skip deleted parents - they already got an exception above.
                if sub.parent.node is None and sub.parent != self.conn.root:
                    continue
                sub.list_changed(list(objs.values()), list(gone))
            # Send out data subs.
            for sub, data in self.data_subs.items():
                if sub.obj.node is None:
                    continue
                sub.data_changed(data)
            # Send out bindata subs.
            for sub in self.bindata_subs:
                if sub.obj.node is None:
                    continue
                data = self.conn.db.get_bindata(
                    sub.node, sub.key, sub.start, sub.end)
                sub.bindata_changed(data)
        else:
            # Whoops.  Undo changes.
            self.conn.db.rollback()
            for k, (node, parent) in self.undo.items():
                k.node = node
                k.parent = parent

    def list_ensure(self, sub):
        if sub not in self.list_changes:
            self.list_changes[sub] = ({}, set())

    def list_add(self, sub, node):
        self.list_ensure(sub)
        self.list_changes[sub][0][node.id] = node

    def list_remove(self, sub, node):
        self.list_ensure(sub)
        self.list_changes[sub][1].add(node.id)

    def set_data(self, node, key, data):
        for sub in node.data_subs.get(key, ()):
            self.data_subs[sub] = data

    def bindata_changed(self, sub):
        self.bindata_subs.add(sub)

    def save(self, node):
        if node not in self.undo:
            self.undo[node] = deepcopy(node.node), node.parent
