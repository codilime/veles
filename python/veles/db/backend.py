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
from veles.util.bigint import bigint_encode, bigint_decode


DB_APP_ID = int('veles', 36)
DB_VERSION = 2
DB_BINDATA_PAGE_SIZE = 0x10000

DB_SCHEMA = [
    'pragma application_id = {}'.format(DB_APP_ID),
    'pragma user_version = {}'.format(DB_VERSION),
    """
        CREATE TABLE node(
            id BLOB NOT NULL PRIMARY KEY,
            parent REFERENCES node(id),
            pos_start BLOB,
            pos_end BLOB
        )
    """, """
        CREATE INDEX node_list
        ON node(parent, pos_start)
    """, """
        CREATE TABLE node_tag(
            id BLOB NOT NULL REFERENCES node(id),
            name VARCHAR NOT NULL,
            PRIMARY KEY (id, name)
        )
    """, """
        CREATE TABLE node_attr(
            id BLOB NOT NULL REFERENCES node(id),
            name VARCHAR NOT NULL,
            data BLOB NOT NULL,
            PRIMARY KEY (id, name)
        )
    """, """
        CREATE TABLE node_data(
            id BLOB NOT NULL REFERENCES node(id),
            name VARCHAR NOT NULL,
            data BLOB NOT NULL,
            PRIMARY KEY (id, name)
        )
    """, """
        CREATE TABLE node_bindata(
            id BLOB NOT NULL REFERENCES node(id),
            name VARCHAR NOT NULL,
            page INTEGER NOT NULL,
            data BLOB NOT NULL,
            PRIMARY KEY (id, name, page)
        )
    """
]


# TODO:
#
# - link support
# - xref support
# - trigger model

if six.PY3:
    def buffer(x):
        return x


def db_bigint_encode(val):
    return None if val is None else buffer(bigint_encode(val))


def db_bigint_decode(val):
    return None if val is None else bigint_decode(bytes(val))


class DbBackend:
    def __init__(self, path):
        if path is None:
            path = ':memory:'
        elif path.startswith(':'):
            path = './' + path
        self.db = sqlite3.connect(path)
        appid = self.db.execute('pragma application_id').fetchone()[0]
        if appid == 0:
            # No application ID set - either we've just created a new database
            # (and need to init it), or we've opened an existing non-Veles
            # sqlite database.  Let's just check if it's empty.
            tables = self.db.execute("""
                SELECT * FROM sqlite_master
            """).fetchall()
            if tables:
                raise ValueError('non-Veles database found')
            # It's empty, let's initialize it.
            c = self.db.cursor()
            for x in DB_SCHEMA:
                c.execute(x)
            self.db.commit()
        elif appid == DB_APP_ID:
            # This is a Veles database, but is it the right version?
            version = self.db.execute('pragma user_version').fetchone()[0]
            if version != DB_VERSION:
                raise ValueError('unknown database schema version')
        else:
            raise ValueError('invalid application ID')
        self.db.execute('pragma foreign_keys = on')
        fk = self.db.execute('pragma foreign_keys').fetchone()[0]
        if not fk:
            raise ValueError('foreign keys not supported by sqlite')
        wrapper = msgpackwrap.MsgpackWrapper()
        self.unpacker = wrapper.unpacker
        self.packer = wrapper.packer

    def _load(self, data):
        self.unpacker.feed(data)
        return self.unpacker.unpack()

    def get(self, id):
        if not isinstance(id, NodeID):
            raise TypeError('node id has wrong type')
        raw_id = buffer(id.bytes)
        c = self.db.cursor()
        c.execute("""
            SELECT parent, pos_start, pos_end FROM node WHERE id = ?
        """, (raw_id,))
        rows = c.fetchall()
        if not rows:
            return None
        (raw_parent, pos_start, pos_end), = rows
        parent = NodeID(bytes(raw_parent)) if raw_parent else NodeID.root_id
        c.execute("""
            SELECT name FROM node_tag WHERE id = ?
        """, (raw_id,))
        tags = {x for x, in c.fetchall()}
        c.execute("""
            SELECT name, data FROM node_attr WHERE id = ?
        """, (raw_id,))
        attr = {k: self._load(v) for k, v in c.fetchall()}
        c.execute("""
            SELECT name FROM node_data WHERE id = ?
        """, (raw_id,))
        data = {x for x, in c.fetchall()}
        # ATTENTION: sqlite dependency here - length(data) will be evaluated
        # for the max page for a given key (see
        # https://www.sqlite.org/lang_select.html#bareagg).
        assert isinstance(c, sqlite3.Cursor)
        c.execute("""
            SELECT name, MAX(page), length(data)
            FROM node_bindata
            WHERE id = ?
            GROUP BY name
        """, (raw_id,))
        bindata = {
            key: page * DB_BINDATA_PAGE_SIZE + lastlen
            for key, page, lastlen in c.fetchall()
        }
        return Node(id=id, parent=parent,
                    pos_start=db_bigint_decode(pos_start),
                    pos_end=db_bigint_decode(pos_end), tags=tags, attr=attr,
                    data=data, bindata=bindata)

    def create(self, node, commit=True):
        if not isinstance(node, Node):
            raise TypeError('node has wrong type')
        if node.id == NodeID.root_id:
            raise ValueError('cannot create root')
        raw_id = buffer(node.id.bytes)
        if node.parent == NodeID.root_id:
            raw_parent = None
        else:
            raw_parent = buffer(node.parent.bytes)
        c = self.db.cursor()
        c.execute("""
            INSERT INTO node (id, parent, pos_start, pos_end)
            VALUES (?, ?, ?, ?)
        """, (
            raw_id, raw_parent,
            db_bigint_encode(node.pos_start),
            db_bigint_encode(node.pos_end)
        ))
        c.executemany("""
            INSERT INTO node_tag (id, name) VALUES (?, ?)
        """, [
            (raw_id, tag)
            for tag in node.tags
        ])
        c.executemany("""
            INSERT INTO node_attr (id, name, data) VALUES (?, ?, ?)
        """, [
            (raw_id, key, buffer(self.packer.pack(val)))
            for key, val in node.attr.items()
        ])
        if commit:
            self.commit()

    def set_pos(self, id, pos_start, pos_end, commit=True):
        if not isinstance(id, NodeID):
            raise TypeError('node id has wrong type')
        if (not isinstance(pos_start, six.integer_types)
                and pos_start is not None):
            raise TypeError('pos_start has to be an int')
        if (not isinstance(pos_end, six.integer_types)
                and pos_end is not None):
            raise TypeError('pos_end has to be an int')
        raw_id = buffer(id.bytes)
        c = self.db.cursor()
        c.execute("""
            UPDATE node
            SET pos_start = ?, pos_end = ?
            WHERE id = ?
        """, (
            db_bigint_encode(pos_start),
            db_bigint_encode(pos_end),
            raw_id
        ))
        if commit:
            self.commit()

    def set_parent(self, id, parent_id, commit=True):
        if not isinstance(id, NodeID):
            raise TypeError('node id has wrong type')
        if not isinstance(parent_id, NodeID):
            raise TypeError('parent id has wrong type')
        raw_id = buffer(id.bytes)
        if parent_id == NodeID.root_id:
            raw_parent = None
        else:
            raw_parent = buffer(parent_id.bytes)
        c = self.db.cursor()
        c.execute("""
            UPDATE node
            SET parent = ?
            WHERE id = ?
        """, (raw_parent, raw_id))
        if commit:
            self.commit()

    def add_tag(self, id, tag, commit=True):
        if not isinstance(id, NodeID):
            raise TypeError('node id has wrong type')
        if not isinstance(tag, six.text_type):
            raise TypeError('tag is not a string')
        raw_id = buffer(id.bytes)
        c = self.db.cursor()
        c.execute("""
            DELETE FROM node_tag
            WHERE id = ? AND name = ?
        """, (raw_id, tag))
        c.execute("""
            INSERT INTO node_tag (id, name) VALUES (?, ?)
        """, (raw_id, tag))
        if commit:
            self.commit()

    def del_tag(self, id, tag, commit=True):
        if not isinstance(id, NodeID):
            raise TypeError('node id has wrong type')
        if not isinstance(tag, six.text_type):
            raise TypeError('tag is not a string')
        raw_id = buffer(id.bytes)
        c = self.db.cursor()
        c.execute("""
            DELETE FROM node_tag
            WHERE id = ? AND name = ?
        """, (raw_id, tag))
        if commit:
            self.commit()

    def set_attr(self, id, key, val, commit=True):
        if not isinstance(id, NodeID):
            raise TypeError('node id has wrong type')
        if not isinstance(key, six.text_type):
            raise TypeError('key is not a string')
        raw_id = buffer(id.bytes)
        c = self.db.cursor()
        c.execute("""
            DELETE FROM node_attr WHERE id = ? AND name = ?
        """, (raw_id, key))
        if val is not None:
            c.execute("""
                INSERT INTO node_attr (id, name, data) VALUES (?, ?, ?)
            """, (raw_id, key, buffer(self.packer.pack(val))))
        if commit:
            self.commit()

    def get_data(self, id, key):
        if not isinstance(id, NodeID):
            raise TypeError('node id has wrong type')
        if not isinstance(key, six.text_type):
            raise TypeError('key is not a string')
        raw_id = buffer(id.bytes)
        c = self.db.cursor()
        c.execute("""
            SELECT data FROM node_data WHERE id = ? AND name = ?
        """, (raw_id, key))
        rows = c.fetchall()
        if not rows:
            return None
        (data,), = rows
        return self._load(data)

    def set_data(self, id, key, data, commit=True):
        if not isinstance(id, NodeID):
            raise TypeError('node id has wrong type')
        if not isinstance(key, six.text_type):
            raise TypeError('key is not a string')
        raw_id = buffer(id.bytes)
        c = self.db.cursor()
        c.execute("""
            DELETE FROM node_data WHERE id = ? AND name = ?
        """, (raw_id, key))
        if data is not None:
            c.execute("""
                INSERT INTO node_data (id, name, data) VALUES (?, ?, ?)
            """, (raw_id, key, buffer(self.packer.pack(data))))
        if commit:
            self.commit()

    def get_bindata(self, id, key, start=0, end=None):
        if not isinstance(id, NodeID):
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
        if start == end:
            return b''
        page_first = start // DB_BINDATA_PAGE_SIZE
        offset = page_first * DB_BINDATA_PAGE_SIZE
        start -= offset
        c = self.db.cursor()
        if end is None:
            c.execute("""
                SELECT data
                FROM node_bindata
                WHERE id = ? AND name = ? AND page >= ?
                ORDER BY page
            """, (buffer(id.bytes), key, page_first))
        else:
            page_last = (end - 1) // DB_BINDATA_PAGE_SIZE
            c.execute("""
                SELECT data
                FROM node_bindata
                WHERE id = ? AND name = ? AND page BETWEEN ? AND ?
                ORDER BY page
            """, (buffer(id.bytes), key, page_first, page_last))
            end -= offset
        data = b''.join(bytes(x) for x, in c.fetchall())
        return data[start:end]

    def set_bindata(self, id, key, start, data, truncate=False, commit=True):
        if not isinstance(id, NodeID):
            raise TypeError('node id has wrong type')
        start = operator.index(start)
        if start < 0:
            raise ValueError('start must not be negative')
        if not isinstance(key, six.text_type):
            raise TypeError('key is not a string')
        if not isinstance(truncate, bool):
            raise TypeError('truncate is not a bool')
        if not isinstance(data, bytes):
            raise TypeError('data must be bytes')
        raw_id = buffer(id.bytes)

        # First, determine current length.
        c = self.db.cursor()
        # ATTENTION: sqlite dependency here - length(data) will be evaluated
        # for the max page (see
        # https://www.sqlite.org/lang_select.html#bareagg).
        assert isinstance(c, sqlite3.Cursor)
        c.execute("""
            SELECT MAX(page), length(data)
            FROM node_bindata
            WHERE id = ? AND name = ?
        """, (raw_id, key))
        (page, lastlen), = c.fetchall()
        if page is not None:
            cur_len = page * DB_BINDATA_PAGE_SIZE + lastlen
        else:
            cur_len = 0
        if start > cur_len:
            raise WritePastEndError()

        # Some calculations.
        end = start + len(data)
        page_first = start // DB_BINDATA_PAGE_SIZE
        page_end = (end + DB_BINDATA_PAGE_SIZE - 1) // DB_BINDATA_PAGE_SIZE
        offset = page_first * DB_BINDATA_PAGE_SIZE
        real_end = page_end * DB_BINDATA_PAGE_SIZE

        # Fetch partial first page.
        if start != offset:
            assert offset < start
            data = self.get_bindata(id, key, offset, start) + data
            start = offset

        # Fetch partial last page.
        if end != real_end and not truncate:
            assert end < real_end
            data = data + self.get_bindata(id, key, end, real_end)
            end = real_end

        # Remove pages that will be overwritten or truncated.
        if truncate:
            c.execute("""
                DELETE FROM node_bindata
                WHERE id = ? AND name = ? AND page >= ?
            """, (raw_id, key, page_first))
        elif page_first == page_end:
            # Nothing to do.
            assert len(data) == 0
            return
        else:
            c.execute("""
                DELETE FROM node_bindata
                WHERE id = ? AND name = ? AND page BETWEEN ? AND ?
            """, (raw_id, key, page_first, page_end - 1))

        # Write new pages.
        c.executemany("""
            INSERT INTO node_bindata (id, name, page, data)
            VALUES (?, ?, ?, ?)
        """, [
            (
                raw_id, key, page,
                buffer(data[
                    (page - page_first) * DB_BINDATA_PAGE_SIZE:
                    (page - page_first + 1) * DB_BINDATA_PAGE_SIZE
                ])
            ) for page in six.moves.range(page_first, page_end)
        ])

        # We're done here.
        if commit:
            self.commit()

    def delete(self, id, commit=True):
        if not isinstance(id, NodeID):
            raise TypeError('node id has wrong type')
        raw_id = buffer(id.bytes)
        c = self.db.cursor()
        c.execute("""
            DELETE FROM node_tag WHERE id = ?
        """, (raw_id,))
        c.execute("""
            DELETE FROM node_attr WHERE id = ?
        """, (raw_id,))
        c.execute("""
            DELETE FROM node_data WHERE id = ?
        """, (raw_id,))
        c.execute("""
            DELETE FROM node_bindata WHERE id = ?
        """, (raw_id,))
        c.execute("""
            DELETE FROM node WHERE id = ?
        """, (raw_id,))
        if commit:
            self.commit()

    def list(self, parent, tags=frozenset(), pos_filter=PosFilter()):
        if not isinstance(parent, NodeID):
            raise TypeError('parent must be a NodeID')
        if not isinstance(tags, (set, frozenset)):
            raise TypeError('tags must be a set')
        if not isinstance(pos_filter, PosFilter):
            raise TypeError('pos_filter must be a PosFilter')
        if parent == NodeID.root_id:
            stmt = """
                SELECT id FROM node WHERE parent IS NULL
            """
            args = ()
        else:
            stmt = """
                SELECT id FROM node WHERE parent = ?
            """
            args = (buffer(parent.bytes), )
        for tag in tags:
            if not isinstance(tag, six.text_type):
                raise TypeError('tag is not a string')
            stmt += """ AND EXISTS (
                SELECT 1 FROM node_tag
                WHERE node_tag.id = node.id AND name = ?
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

    def begin(self):
        if six.PY3:
            assert not self.db.in_transaction

    def commit(self):
        self.db.commit()

    def rollback(self):
        self.db.rollback()

    def close(self):
        self.db.close()
