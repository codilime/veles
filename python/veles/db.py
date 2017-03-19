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

import operator

import sqlite3

import six

from veles.proto import msgpackwrap
from veles.schema.nodeid import NodeID
from veles.proto.node import Node, PosFilter
from veles.proto.exceptions import WritePastEndError
from veles.compatibility.int_bytes import int_from_bytes
from veles.util.bigint import bigint_encode, bigint_decode


APP_ID = int_from_bytes(b'mln0', 'big')


# TODO:
#
# - link support
# - xref support
# - paged BinData
# - trigger model

if six.PY3:
    def buffer(x):
        return x


def db_bigint_encode(val):
    return None if val is None else buffer(bigint_encode(val))


def db_bigint_decode(val):
    return None if val is None else bigint_decode(bytes(val))


class Database:
    def __init__(self, path):
        if path is None:
            path = ':memory:'
        elif path.startswith(':'):
            path = './' + path
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
                pos_start BLOB,
                pos_end BLOB
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

    def get(self, obj_id):
        if not isinstance(obj_id, NodeID):
            raise TypeError('node id has wrong type')
        raw_obj_id = buffer(obj_id.bytes)
        c = self.db.cursor()
        c.execute("""
            SELECT parent, pos_start, pos_end FROM object WHERE id = ?
        """, (raw_obj_id,))
        rows = c.fetchall()
        if not rows:
            return None
        (raw_parent, pos_start, pos_end), = rows
        parent = NodeID(bytes(raw_parent)) if raw_parent else NodeID.root_id
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
        return Node(id=obj_id, parent=parent,
                    pos_start=db_bigint_decode(pos_start),
                    pos_end=db_bigint_decode(pos_end), tags=tags, attr=attr,
                    data=data, bindata=bindata)

    def create(self, node):
        if not isinstance(node, Node):
            raise TypeError('node has wrong type')
        if node.id == NodeID.root_id:
            raise ValueError('cannot create root')
        raw_obj_id = buffer(node.id.bytes)
        if node.parent == NodeID.root_id:
            raw_parent = None
        else:
            raw_parent = buffer(node.parent.bytes)
        c = self.db.cursor()
        c.execute("""
            INSERT INTO object (id, parent, pos_start, pos_end)
            VALUES (?, ?, ?, ?)
        """, (
            raw_obj_id, raw_parent,
            db_bigint_encode(node.pos_start),
            db_bigint_encode(node.pos_end)
        ))
        for tag in node.tags:
            c.execute("""
                INSERT INTO object_tag (obj_id, name) VALUES (?, ?)
            """, (raw_obj_id, tag))
        for key, val in node.attr.items():
            c.execute("""
                INSERT INTO object_attr (obj_id, name, data) VALUES (?, ?, ?)
            """, (raw_obj_id, key, buffer(self.packer.pack(val))))
        self.db.commit()

    def set_pos(self, obj_id, pos_start, pos_end):
        if not isinstance(obj_id, NodeID):
            raise TypeError('node id has wrong type')
        if (not isinstance(pos_start, six.integer_types)
                and pos_start is not None):
            raise TypeError('pos_start has to be an int')
        if (not isinstance(pos_end, six.integer_types)
                and pos_end is not None):
            raise TypeError('pos_end has to be an int')
        raw_obj_id = buffer(obj_id.bytes)
        c = self.db.cursor()
        c.execute("""
            UPDATE object
            SET pos_start = ?, pos_end = ?
            WHERE id = ?
        """, (
            db_bigint_encode(pos_start),
            db_bigint_encode(pos_end),
            raw_obj_id
        ))
        self.db.commit()

    def set_parent(self, obj_id, parent_id):
        if not isinstance(obj_id, NodeID):
            raise TypeError('node id has wrong type')
        if not isinstance(parent_id, NodeID):
            raise TypeError('parent id has wrong type')
        raw_obj_id = buffer(obj_id.bytes)
        if parent_id == NodeID.root_id:
            raw_parent = None
        else:
            raw_parent = buffer(parent_id.bytes)
        c = self.db.cursor()
        c.execute("""
            UPDATE object
            SET parent = ?
            WHERE id = ?
        """, (raw_parent, raw_obj_id))
        self.db.commit()

    def add_tag(self, obj_id, tag):
        if not isinstance(obj_id, NodeID):
            raise TypeError('node id has wrong type')
        if not isinstance(tag, six.text_type):
            raise TypeError('tag is not a string')
        raw_obj_id = buffer(obj_id.bytes)
        c = self.db.cursor()
        c.execute("""
            DELETE FROM object_tag
            WHERE obj_id = ? AND name = ?
        """, (raw_obj_id, tag))
        c.execute("""
            INSERT INTO object_tag (obj_id, name) VALUES (?, ?)
        """, (raw_obj_id, tag))
        self.db.commit()

    def del_tag(self, obj_id, tag):
        if not isinstance(obj_id, NodeID):
            raise TypeError('node id has wrong type')
        if not isinstance(tag, six.text_type):
            raise TypeError('tag is not a string')
        raw_obj_id = buffer(obj_id.bytes)
        c = self.db.cursor()
        c.execute("""
            DELETE FROM object_tag
            WHERE obj_id = ? AND name = ?
        """, (raw_obj_id, tag))
        self.db.commit()

    def set_attr(self, obj_id, key, val):
        if not isinstance(obj_id, NodeID):
            raise TypeError('node id has wrong type')
        if not isinstance(key, six.text_type):
            raise TypeError('key is not a string')
        raw_obj_id = buffer(obj_id.bytes)
        c = self.db.cursor()
        c.execute("""
            DELETE FROM object_attr WHERE obj_id = ? AND name = ?
        """, (raw_obj_id, key))
        if val is not None:
            c.execute("""
                INSERT INTO object_attr (obj_id, name, data) VALUES (?, ?, ?)
            """, (raw_obj_id, key, buffer(self.packer.pack(val))))
        self.db.commit()

    def get_data(self, obj_id, key):
        if not isinstance(obj_id, NodeID):
            raise TypeError('node id has wrong type')
        if not isinstance(key, six.text_type):
            raise TypeError('key is not a string')
        raw_obj_id = buffer(obj_id.bytes)
        c = self.db.cursor()
        c.execute("""
            SELECT data FROM object_data WHERE obj_id = ? AND name = ?
        """, (raw_obj_id, key))
        rows = c.fetchall()
        if not rows:
            return None
        (data,), = rows
        return self._load(data)

    def set_data(self, obj_id, key, data):
        if not isinstance(obj_id, NodeID):
            raise TypeError('node id has wrong type')
        if not isinstance(key, six.text_type):
            raise TypeError('key is not a string')
        raw_obj_id = buffer(obj_id.bytes)
        c = self.db.cursor()
        c.execute("""
            DELETE FROM object_data WHERE obj_id = ? AND name = ?
        """, (raw_obj_id, key))
        if data is not None:
            c.execute("""
                INSERT INTO object_data (obj_id, name, data) VALUES (?, ?, ?)
            """, (raw_obj_id, key, buffer(self.packer.pack(data))))
        self.db.commit()

    # XXX: refactor these two to use paging

    def get_bindata(self, obj_id, key, start=0, end=None):
        if not isinstance(obj_id, NodeID):
            raise TypeError('node id has wrong type')
        start = operator.index(start)
        if end is not None:
            end = operator.index(end)
        if start < 0:
            raise ValueError('start must not be negative')
        if end is not None and end < start:
            raise ValueError('end must be >= start')
        if not isinstance(key, six.text_type):
            raise TypeError('key is not a string')
        c = self.db.cursor()
        c.execute("""
            SELECT data FROM object_bindata WHERE obj_id = ? AND name = ?
        """, (buffer(obj_id.bytes), key))
        rows = c.fetchall()
        if not rows:
            return b''
        (data,), = rows
        return data[start:end]

    def set_bindata(self, obj_id, key, start, data, truncate=False):
        if not isinstance(obj_id, NodeID):
            raise TypeError('node id has wrong type')
        start = operator.index(start)
        if start < 0:
            raise ValueError('start must not be negative')
        if not isinstance(key, six.text_type):
            raise TypeError('key is not a string')
        if not isinstance(truncate, bool):
            raise TypeError('truncate is not a bool')
        cur = self.get_bindata(obj_id, key)
        raw_obj_id = buffer(obj_id.bytes)
        new_data = bytearray(cur)
        if len(new_data) < start:
            raise WritePastEndError()
        new_data[start:start+len(data)] = data
        if truncate:
            del new_data[start+len(data):]
        new_data = bytes(new_data)
        c = self.db.cursor()
        c.execute("""
            DELETE FROM object_bindata WHERE obj_id = ? AND name = ?
        """, (raw_obj_id, key))
        if new_data:
            c.execute("""
                INSERT INTO object_bindata (obj_id, name, data)
                VALUES (?, ?, ?)
            """, (raw_obj_id, key, buffer(new_data)))
        self.db.commit()

    def delete(self, obj_id):
        if not isinstance(obj_id, NodeID):
            raise TypeError('node id has wrong type')
        raw_obj_id = buffer(obj_id.bytes)
        c = self.db.cursor()
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

    def list(self, parent, tags=frozenset(), pos_filter=PosFilter()):
        if not isinstance(parent, NodeID):
            raise TypeError('parent must be a NodeID')
        if not isinstance(tags, (set, frozenset)):
            raise TypeError('tags must be a set')
        if not isinstance(pos_filter, PosFilter):
            raise TypeError('pos_filter must be a PosFilter')
        if parent == NodeID.root_id:
            stmt = """
                SELECT id FROM object WHERE parent IS NULL
            """
            args = ()
        else:
            stmt = """
                SELECT id FROM object WHERE parent = ?
            """
            args = (buffer(parent.bytes), )
        for tag in tags:
            if not isinstance(tag, six.text_type):
                raise TypeError('tag is not a string')
            stmt += """ AND EXISTS (
                SELECT 1 FROM object_tag
                WHERE obj_id = id AND name = ?
            )"""
            args += (tag,)
        if pos_filter.start_from is not None:
            stmt += " AND pos_start >= ?"
            args += (db_bigint_encode(pos_filter.start_from),)
        if pos_filter.start_to is not None:
            stmt += " AND (pos_start <= ? OR pos_start IS NULL)"
            args += (db_bigint_encode(pos_filter.start_to),)
        if pos_filter.end_from is not None:
            stmt += " AND (pos_end >= ? OR pos_end is NULL)"
            args += (db_bigint_encode(pos_filter.end_from),)
        if pos_filter.end_to is not None:
            stmt += " AND pos_end <= ?"
            args += (db_bigint_encode(pos_filter.end_to),)
        c = self.db.cursor()
        c.execute(stmt, args)
        return {NodeID(bytes(x)) for x, in c.fetchall()}

    def close(self):
        self.db.close()
