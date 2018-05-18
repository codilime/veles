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

from veles.proto.exceptions import ObjectGoneError


class Transaction(object):
    def __init__(self, tracker):
        self.tracker = tracker
        self.undo = {}
        self.old_data = {}
        self.list_changes = {}
        self.data_subs = {}
        self.bindata_subs = set()
        self.gone_subs = set()
        self.node_subs = {}
        self.triggers_gone = set()

    def precommit(self):
        for dbnode, (node, parent) in self.undo.items():
            if dbnode.node is None or node is None:
                # Node created or deleted - bust everything.
                self.tracker.db.bust_triggers_all(dbnode.id)
            else:
                if dbnode.node.parent != node.parent:
                    self.tracker.db.bust_triggers_node(dbnode.id, 'parent')
                if (dbnode.node.pos_start != node.pos_start or
                        dbnode.node.pos_end != node.pos_end):
                    self.tracker.db.bust_triggers_node(dbnode.id, 'pos')
                if dbnode.node.tags != node.tags:
                    self.tracker.db.bust_triggers_node(dbnode.id, 'tags')
                for tag in (dbnode.node.tags ^ node.tags):
                    self.tracker.db.bust_triggers_node_name(
                        dbnode.id, 'tag', tag)
                for attr in (set(dbnode.node.attr) | set(node.attr)):
                    if dbnode.node.attr.get(attr) != node.attr.get(attr):
                        self.tracker.db.bust_triggers_node_name(
                            dbnode.id, 'attr', attr)
                all_bindata = set(dbnode.node.bindata) | set(node.bindata)
                for key in all_bindata:
                    if (dbnode.node.bindata.get(key) !=
                            node.bindata.get(key)):
                        self.tracker.db.bust_triggers_node_name(
                            dbnode.id, 'bindata_size', key)
                all_triggers = set(dbnode.node.triggers) | set(node.triggers)
                for trigger in all_triggers:
                    if (dbnode.node.triggers.get(trigger) !=
                            node.triggers.get(trigger)):
                        self.tracker.db.bust_triggers_node_name(
                            dbnode.id, 'trigger', trigger)
        for (dbnode, key), data in self.old_data.items():
            if data != self.tracker.db.get_data(dbnode.id, key):
                self.tracker.db.bust_triggers_node_name(
                    dbnode.id, 'data', key)
                for sub in dbnode.data_subs.get(key, ()):
                    self.data_subs[sub] = data
        # XXX: bust bindata
        # XXX: bust list

    def handle_changed_node(self, dbnode, old_node, old_parent):
        # Queue normal subs.  Also handle all subs for created
        # and removed parents.
        if dbnode.node is None:
            # Node has been deleted.
            self.gone_subs |= dbnode.subs
            self.gone_subs |= dbnode.list_subs.keys()
            for key, subs in dbnode.data_subs.items():
                self.gone_subs |= subs
            for key, subs in dbnode.bindata_subs.items():
                self.gone_subs |= subs
        else:
            for sub in dbnode.subs:
                self.node_subs[sub] = dbnode.node
            if old_node is None:
                # Node has been created.
                for sub in dbnode.list_subs:
                    self.list_ensure(sub)
                for key, subs in dbnode.data_subs.items():
                    for sub in subs:
                        if sub not in self.data_subs:
                            self.data_subs[sub] = None
                for key, subs in dbnode.bindata_subs.items():
                    for sub in subs:
                        self.bindata_subs.add(sub)
        # Unlist objects removed from parents.
        if dbnode.parent != old_parent and old_parent is not None:
            for sub, dbnodes in old_parent.list_subs.items():
                if dbnode in dbnodes:
                    dbnodes.remove(dbnode)
                    self.list_remove(sub, dbnode)
        # Update (current) parent's listers.
        if dbnode.parent is not None:
            for sub, dbnodes in dbnode.parent.list_subs.items():
                if sub.matches(dbnode.node):
                    dbnodes.add(dbnode)
                    self.list_add(sub, dbnode)
                elif dbnode in dbnodes:
                    dbnodes.remove(dbnode)
                    self.list_remove(sub, dbnode)

    def postcommit(self):
        self.tracker._triggers_gone(self.triggers_gone)
        for dbnode, (node, parent) in self.undo.items():
            if dbnode.node != node:
                self.handle_changed_node(dbnode, node, parent)
        # Send out all gone subs.
        for sub in self.gone_subs:
            sub.error(ObjectGoneError())
        # Send out all node subs.
        for sub, node in self.node_subs.items():
            sub.node_changed(node)
        # Now send out all buffered list changes.
        for sub, (changed, gone) in self.list_changes.items():
            if sub in self.gone_subs:
                continue
            sub.list_changed(list(changed.values()), list(gone))
        # Send out data subs.
        for sub, data in self.data_subs.items():
            if sub in self.gone_subs:
                continue
            sub.data_changed(data)
        # Send out bindata subs.
        for sub in self.bindata_subs:
            if sub in self.gone_subs:
                continue
            data = self.tracker.get_bindata(
                sub.node, sub.key, sub.start, sub.end)
            sub.bindata_changed(data)

    def rollback(self):
        self.tracker.db.rollback()
        for dbnode, (node, parent) in self.undo.items():
            dbnode.node = node
            dbnode.parent = parent

    def __enter__(self):
        self.tracker.db.begin()
        return self

    def __exit__(self, exc, val, tb):
        if exc is None:
            # Everything alright, let's commit.
            try:
                self.precommit()
            except:
                # This should never happen, but just in case...
                self.rollback()
                raise
            self.tracker.db.commit()
            self.postcommit()
        else:
            # Whoops.  Undo changes.
            self.rollback()

    def list_ensure(self, sub):
        if sub not in self.list_changes:
            self.list_changes[sub] = ({}, set())

    def list_add(self, sub, dbnode):
        self.list_ensure(sub)
        self.list_changes[sub][0][dbnode.id] = dbnode.node

    def list_remove(self, sub, dbnode):
        self.list_ensure(sub)
        self.list_changes[sub][1].add(dbnode.id)

    def set_data(self, dbnode, key, data):
        if (dbnode, key) not in self.old_data:
            old_data = self.tracker.db.get_data(dbnode.id, key)
            self.old_data[dbnode.id, key] = old_data

    def bindata_changed(self, sub):
        self.bindata_subs.add(sub)

    def save(self, dbnode):
        if dbnode not in self.undo:
            self.undo[dbnode] = deepcopy(dbnode.node), dbnode.parent
