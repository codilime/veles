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

import sqlite3
import weakref

from veles.messages import msgpackwrap

from veles.common import base


APP_ID = int.from_bytes(b'ml0n', 'big')


class BaseLister:
    def __init__(self, srv, parent, pos, tags):
        self.srv = srv
        self.pos = pos
        self.tags = tags
        self.subs = set()
        self.objs = set()
        if parent is not None:
            self.parent = srv.get(parent)
            if self.parent is None:
                self.obj_gone()
                return
        else:
            self.parent = None

    def kill(self):
        if self.parent is None:
            self.srv.top_listers.remove(self)
        else:
            self.parent.listers.remove(self)

    def matches(self, obj):
        if self.pos[0] is not None:
            if obj.pos[0] is None:
                return False
            if obj.pos[1] is not None and obj.pos[1] <= self.pos[0]:
                return False
        if self.pos[1] is not None:
            if obj.pos[0] is not None and obj.pos[0] >= self.pos[1]:
                return False
        for tags in self.tags:
            for k, v in tags:
                if v != (k in obj.tags):
                    break
            else:
                return True
        return False

    def list_changed(self):
        raise NotImplementedError

    def obj_gone(self):
        raise NotImplementedError


class Object:
    def __init__(self, srv, id, parent, pos, tags, attr, data, bindata):
        self.srv = srv
        self.id = id
        self.parent = parent
        self.pos = pos
        self.tags = tags
        self.attr = attr
        self.data = data
        self.bindata = bindata
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


class Server:
    def __init__(self, loop, path):
        self.loop = loop
        self.db = sqlite3.connect(path)
        appid = self.db.cursor().execute('pragma application_id').fetchone()[0]
        if appid == 0:
            self.new_db()
        elif appid != APP_ID:
            raise ValueError('invalid application ID')
        self.db.cursor().execute('pragma foreign_keys = on')
        fk = self.db.cursor().execute('pragma foreign_keys').fetchone()[0]
        if not fk:
            raise ValueError('foreign keys not supported by sqlite')
        self.conns = {}
        self.objs = weakref.WeakValueDictionary()
        self.next_cid = 0
        self.top_listers = set()
        wrapper = msgpackwrap.MsgpackWrapper()
        self.unpacker = wrapper.unpacker
        self.packer = wrapper.packer

    def _load(self, data):
        self.unpacker.feed(data)
        return self.unpacker.unpack()

    def new_db(self):
        c = self.db.cursor()
        c.execute('pragma application_id = {}'.format(APP_ID))
        c.execute("""
            CREATE TABLE object(
                id BLOB PRIMARY KEY,
                parent REFERENCES object(id),
                pos_start INTEGER,
                pos_end INTEGER
            )
        """)
        c.execute("""
            CREATE TABLE object_tag(
                obj_id BLOB REFERENCES object(id),
                name VARCHAR,
                PRIMARY KEY (obj_id, name)
            )
        """)
        c.execute("""
            CREATE TABLE object_attr(
                obj_id BLOB REFERENCES object(id),
                name VARCHAR,
                data BLOB,
                PRIMARY KEY (obj_id, name)
            )
        """)
        c.execute("""
            CREATE TABLE object_data(
                obj_id BLOB REFERENCES object(id),
                name VARCHAR,
                data BLOB,
                PRIMARY KEY (obj_id, name)
            )
        """)
        c.execute("""
            CREATE TABLE object_bindata(
                obj_id BLOB REFERENCES object(id),
                name VARCHAR,
                data BLOB,
                PRIMARY KEY (obj_id, name)
            )
        """)
        self.db.commit()

    def new_conn(self, conn):
        cid = self.next_cid
        self.next_cid += 1
        self.conns[cid] = conn
        print("Conn {} started.".format(cid))
        return cid

    def remove_conn(self, conn):
        print("Conn {} gone.".format(conn.cid))
        self.conns[conn.cid] = None

    def create(self, obj_id, parent, *, tags=[], attr={}, data={}, bindata={},
               pos=(None, None)):
        raw_obj_id = obj_id.to_bytes()
        raw_parent = parent.to_bytes() if parent else None
        c = self.db.cursor()
        c.execute("""
            INSERT INTO object (id, parent, pos_start, pos_end)
            VALUES (?, ?, ?, ?)
        """, (raw_obj_id, raw_parent, pos[0], pos[1]))
        for tag in tags:
            c.execute("""
                INSERT INTO object_tag (obj_id, name) VALUES (?, ?)
            """, (raw_obj_id, tag))
        for key, val in attr.items():
            c.execute("""
                INSERT INTO object_attr (obj_id, name, data) VALUES (?, ?, ?)
            """, (raw_obj_id, key, self.packer.pack(val)))
        for key, val in data.items():
            c.execute("""
                INSERT INTO object_data (obj_id, name, data) VALUES (?, ?, ?)
            """, (raw_obj_id, key, self.packer.pack(val)))
        for key, val in bindata.items():
            c.execute("""
                INSERT INTO object_bindata (obj_id, name, data)
                VALUES (?, ?, ?)
            """, (raw_obj_id, key, val))
        self.db.commit()
        obj = self.get(obj_id)
        if obj.parent is None:
            listers = self.top_listers
        else:
            listers = obj.parent.listers
        for lister in listers:
            if lister.matches(obj):
                lister.list_changed([obj], [])
                lister.objs.add(obj)

    def delete(self, obj_id):
        obj = self.get(obj_id)
        raw_obj_id = obj_id.to_bytes()
        if obj is None:
            return
        c = self.db.cursor()
        c.execute("""
            SELECT id FROM object WHERE parent = ?
        """, (raw_obj_id,))
        for id, in c.fetchall():
            self.delete(base.ObjectID(id))
        c.execute("""
            DELETE FROM object_tag WHERE obj_id = ?
        """, (raw_obj_id,))
        c.execute("""
            DELETE FROM object_attr WHERE obj_id = ?
        """, (raw_obj_id,))
        c.execute("""
            DELETE FROM object_data WHERE obj_id = ?
        """, (raw_obj_id,))
        c.execute("""
            DELETE FROM object_bindata WHERE obj_id = ?
        """, (raw_obj_id,))
        c.execute("""
            DELETE FROM object WHERE id = ?
        """, (raw_obj_id,))
        self.db.commit()
        obj.clear_subs()
        if obj.parent is None:
            listers = self.top_listers
        else:
            listers = obj.parent.listers
        for lister in listers:
            if obj in lister.objs:
                lister.list_changed([], [obj_id])
                lister.objs.remove(obj)
        del self.objs[obj.id]

    def get(self, obj_id):
        try:
            return self.objs[obj_id]
        except KeyError:
            raw_obj_id = obj_id.to_bytes()
            c = self.db.cursor()
            c.execute("""
                SELECT parent, pos_start, pos_end FROM object WHERE id = ?
            """, (raw_obj_id,))
            rows = c.fetchall()
            if not rows:
                return None
            (parent, pos_start, pos_end), = rows
            if parent is not None:
                parent = self.get(base.ObjectID(parent))
            c.execute("""
                SELECT name FROM object_tag WHERE obj_id = ?
            """, (raw_obj_id,))
            tags = {x for x, in c.fetchall()}
            c.execute("""
                SELECT name, data FROM object_attr WHERE obj_id = ?
            """, (raw_obj_id,))
            attr = {k: self._load(v) for k, v in c.fetchall()}
            c.execute("""
                SELECT name FROM object_data WHERE obj_id = ?
            """, (raw_obj_id,))
            data = {x for x, in c.fetchall()}
            c.execute("""
                SELECT name, length(data) FROM object_bindata WHERE obj_id = ?
            """, (raw_obj_id,))
            bindata = {x: y for x, y in c.fetchall()}
            res = Object(self, obj_id, parent, (pos_start, pos_end),
                         tags, attr, data, bindata)
            self.objs[obj_id] = res
            return res

    def get_data(self, obj, key):
        c = self.db.cursor()
        c.execute("""
            SELECT data FROM object_data WHERE obj_id = ? AND name = ?
        """, (obj.id.to_bytes(), key))
        rows = c.fetchall()
        if not rows:
            return None
        (data,), = rows
        return self._load(data)

    def run_lister(self, lister, sub=False):
        c = self.db.cursor()
        if lister.parent is not None:
            c.execute("""
                SELECT id FROM object WHERE parent = ?
            """, (lister.parent.id.to_bytes(),))
        else:
            c.execute("""
                SELECT id FROM object WHERE parent IS NULL
            """)
        obj_ids = [base.ObjectID(x) for x, in c.fetchall()]
        objs = [self.get(x) for x in obj_ids]
        objs = [x for x in objs if lister.matches(x)]
        lister.list_changed(objs, [])
        if sub:
            lister.objs = set(objs)
            if lister.parent is None:
                self.top_listers.add(lister)
            else:
                lister.parent.listers.add(lister)

    def add_local_plugin(self, plugin):
        # XXX
        pass
