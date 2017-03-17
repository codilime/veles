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

from __future__ import unicode_literals

import unittest
import tempfile
import os.path

import six

from veles.db import Database
from veles.data.bindata import BinData
from veles.proto.node import Node
from veles.schema.nodeid import NodeID
from veles.proto.exceptions import WritePastEndError


class TestDatabase(unittest.TestCase):
    def test_simple(self):
        db = Database(None)
        node = Node(id=NodeID(),
                    tags={'my_tag'},
                    attr={'my_attr': 'my_val'},
                    pos_start=0x123,
                    pos_end=0x456789abcdef1122334456789abcdef,
                    data={'my_key'},
                    bindata={'my_bindata': 12345})
        db.create(node)
        n1 = db.get(NodeID())
        self.assertEqual(n1, None)
        n2 = db.get(node.id)
        self.assertIsInstance(n2, Node)
        self.assertEqual(n2.id, node.id)
        self.assertEqual(n2.parent, node.parent)
        self.assertEqual(n2.tags, node.tags)
        self.assertEqual(n2.attr, node.attr)
        self.assertEqual(n2.pos_start, node.pos_start)
        self.assertEqual(n2.pos_end, node.pos_end)
        self.assertEqual(n2.data, set())
        self.assertEqual(n2.bindata, {})
        with self.assertRaises(TypeError):
            db.get('zlew')
        with self.assertRaises(TypeError):
            db.get(node.id.bytes)
        with self.assertRaises(TypeError):
            db.create({'id': node.id})

    def test_persist(self):
        d = tempfile.mkdtemp()
        path = os.path.join(d, 'x.db')
        try:
            db1 = Database(path)
            node = Node(id=NodeID(),
                        tags={'my_tag'},
                        attr={'my_attr': 'my_val'},
                        pos_start=0x123,
                        pos_end=0x456,
                        data={'my_key'},
                        bindata={'my_bindata': 12345})
            db1.create(node)
            db1.close()
            db2 = Database(path)
            n1 = db2.get(NodeID())
            self.assertEqual(n1, None)
            n2 = db2.get(node.id)
            self.assertIsInstance(n2, Node)
            self.assertEqual(n2.id, node.id)
            self.assertEqual(n2.parent, node.parent)
            self.assertEqual(n2.tags, node.tags)
            self.assertEqual(n2.attr, node.attr)
            self.assertEqual(n2.pos_start, node.pos_start)
            self.assertEqual(n2.pos_end, node.pos_end)
            self.assertEqual(n2.data, set())
            self.assertEqual(n2.bindata, {})
            db2.close()
        finally:
            try:
                os.unlink(path)
            except Exception:
                pass
            os.rmdir(d)

    def test_delete(self):
        db = Database(None)
        node = Node(id=NodeID(),
                    tags={'my_node'},
                    attr={'a': 'b'},
                    pos_start=None,
                    pos_end=None,
                    data=set(),
                    bindata={})
        db.create(node)
        db.set_data(node.id, 'c', 'd')
        db.set_bindata(node.id, 'e', start=0, data=b'f', truncate=False)
        n2 = db.get(node.id)
        self.assertEqual(n2.tags, {'my_node'})
        self.assertEqual(db.get_data(node.id, 'c'), 'd')
        self.assertEqual(db.get_bindata(node.id, 'e'), b'f')
        db.delete(node.id)
        self.assertEqual(db.get(node.id), None)
        self.assertEqual(db.get_data(node.id, 'c'), None)
        self.assertEqual(db.get_bindata(node.id, 'e'), b'')
        id2 = NodeID()
        id3 = NodeID()
        self.assertEqual(db.get(id2), None)
        db.delete(id3)
        self.assertEqual(db.get(id3), None)
        with self.assertRaises(TypeError):
            db.delete(b'zlew')

    def test_parent(self):
        db = Database(None)
        id1 = NodeID()
        id2 = NodeID()
        n1 = Node(id=id1, attr={'name': 'parent'}, data=set(), tags=set(),
                  bindata={})
        db.create(n1)
        n2 = Node(id=id2, parent=id1, attr={'name': 'child'}, tags=set(),
                  data=set(), bindata={})
        db.create(n2)
        n1 = db.get(id1)
        self.assertIsInstance(n1, Node)
        self.assertEqual(n1.id, id1)
        self.assertEqual(n1.parent, None)
        self.assertEqual(n1.attr['name'], 'parent')
        n2 = db.get(id2)
        self.assertIsInstance(n2, Node)
        self.assertEqual(n2.id, id2)
        self.assertEqual(n2.parent, id1)
        self.assertEqual(n2.attr['name'], 'child')
        l0 = db.list(None)
        l1 = db.list(id1)
        l2 = db.list(id2)
        self.assertEqual(l0, {id1})
        self.assertEqual(l1, {id2})
        self.assertEqual(l2, set())
        # reparent n2 to root
        db.set_parent(id2, None)
        n1 = db.get(id1)
        n2 = db.get(id2)
        self.assertEqual(n1.parent, None)
        self.assertEqual(n2.parent, None)
        l0 = db.list(None)
        l1 = db.list(id1)
        l2 = db.list(id2)
        self.assertEqual(l0, {id1, id2})
        self.assertEqual(l1, set())
        self.assertEqual(l2, set())
        # reparent n1 to n2
        db.set_parent(id1, id2)
        n1 = db.get(id1)
        n2 = db.get(id2)
        self.assertEqual(n1.parent, id2)
        self.assertEqual(n2.parent, None)
        l0 = db.list(None)
        l1 = db.list(id1)
        l2 = db.list(id2)
        self.assertEqual(l0, {id2})
        self.assertEqual(l1, set())
        self.assertEqual(l2, {id1})
        with self.assertRaises(TypeError):
            db.set_parent(id1, id2.bytes)

    def test_pos(self):
        db = Database(None)
        node = Node(id=NodeID(),
                    tags=set(),
                    attr={},
                    pos_start=None,
                    pos_end=None,
                    data=set(),
                    bindata={})
        db.create(node)
        n2 = db.get(node.id)
        self.assertEqual(n2.pos_start, None)
        self.assertEqual(n2.pos_end, None)
        db.set_pos(node.id, 123, None)
        n2 = db.get(node.id)
        self.assertEqual(n2.pos_start, 123)
        self.assertEqual(n2.pos_end, None)
        db.set_pos(node.id, 0x11111111111111111111111111111111111,
                   0x222222222222222222222222222222222222222)
        n2 = db.get(node.id)
        self.assertEqual(n2.pos_start, 0x11111111111111111111111111111111111)
        self.assertEqual(n2.pos_end, 0x222222222222222222222222222222222222222)
        with self.assertRaises(TypeError):
            db.set_pos(node.id, 123, b'zlew')
        with self.assertRaises(TypeError):
            db.set_pos(node.id, b'zlew', None)

    def test_tags(self):
        db = Database(None)
        node = Node(id=NodeID(),
                    tags={'abc', 'def', 'ghi'},
                    attr={},
                    pos_start=0x123,
                    pos_end=0x456,
                    data={'my_key'},
                    bindata={'my_bindata': 12345})
        db.create(node)
        n2 = db.get(node.id)
        self.assertEqual(n2.tags, {'abc', 'def', 'ghi'})
        db.add_tag(node.id, 'abc')
        db.add_tag(node.id, 'jkl')
        db.del_tag(node.id, 'def')
        db.del_tag(node.id, 'mno')
        n3 = db.get(node.id)
        self.assertEqual(n3.tags, {'abc', 'ghi', 'jkl'})
        with self.assertRaises(TypeError):
            db.add_tag(node.id, b'zlew')
        with self.assertRaises(TypeError):
            db.add_tag(node.id, 123)
        with self.assertRaises(TypeError):
            db.del_tag(node.id, 123)

    def test_attr(self):
        db = Database(None)
        id2 = NodeID()
        node = Node(id=NodeID(),
                    tags={'my_tag'},
                    attr={
                        'int': 123,
                        'bool': False,
                        'bytes': b'\x01\x02\x03',
                        'str': 'abc',
                        'list': (1, 2, 3),
                        'dict': {'a': 'b', 'c': 'd'},
                        'id': id2,
                        'bindata': BinData.from_spaced_hex(12, '123 456'),
                        'long': 0x123456789abcdef123456789abcdef,
                        'neglong': -0x123456789abcdef123456789abcdef,
                    },
                    pos_start=0x123,
                    pos_end=0x456,
                    data={'my_key'},
                    bindata={'my_bindata': 12345})
        db.create(node)
        n2 = db.get(node.id)
        self.assertEqual(set(n2.attr.keys()), {
            'int', 'bool', 'bytes', 'str', 'list', 'dict', 'id', 'bindata',
            'long', 'neglong',
        })
        self.assertEqual(n2.attr['int'], 123)
        self.assertEqual(n2.attr['bool'], False)
        self.assertNotIn('none', n2.attr)
        self.assertNotIn('meh', n2.attr)
        b = n2.attr['bytes']
        s = n2.attr['str']
        self.assertEqual(b, b'\x01\x02\x03')
        self.assertEqual(s, 'abc')
        self.assertIsInstance(b, bytes)
        self.assertIsInstance(s, six.text_type)
        self.assertEqual(n2.attr['list'], [1, 2, 3])
        self.assertEqual(n2.attr['dict'], {'a': 'b', 'c': 'd'})
        self.assertEqual(n2.attr['id'], id2)
        self.assertEqual(n2.attr['bindata'],
                         BinData.from_spaced_hex(12, '123 456'))
        self.assertEqual(n2.attr['long'],
                         0x123456789abcdef123456789abcdef)
        self.assertEqual(n2.attr['neglong'],
                         -0x123456789abcdef123456789abcdef)
        db.set_attr(node.id, 'int', None)
        db.set_attr(node.id, 'bool', True)
        db.set_attr(node.id, 'meh', 'meh')
        n3 = db.get(node.id)
        self.assertEqual(set(n3.attr.keys()), {
            'meh', 'bool', 'bytes', 'str', 'list', 'dict', 'id', 'bindata',
            'long', 'neglong',
        })
        self.assertEqual(n3.attr['bool'], True)
        self.assertEqual(n3.attr['meh'], 'meh')
        with self.assertRaises(TypeError):
            db.set_attr(node.id, b'zlew', 'zlew')
        with self.assertRaises(TypeError):
            db.set_attr(node.id, 123, 456)
        with self.assertRaises(TypeError):
            db.set_attr(node.id, 123, 456)

    def test_data(self):
        db = Database(None)
        node = Node(id=NodeID(),
                    tags={'my_tag'},
                    attr={'my_attr': 'my_val'},
                    pos_start=0x123,
                    pos_end=0x456,
                    data={'my_key'},
                    bindata={'my_bindata': 12345})
        id2 = NodeID()
        db.create(node)
        db.set_data(node.id, 'int', 123)
        db.set_data(node.id, 'bool', False)
        db.set_data(node.id, 'none', None)
        db.set_data(node.id, 'bytes', b'\x01\x02\x03')
        db.set_data(node.id, 'str', 'abc')
        db.set_data(node.id, 'list', (1, 2, 3))
        db.set_data(node.id, 'dict', {'a': 'b', 'c': 'd'})
        db.set_data(node.id, 'id', id2)
        db.set_data(node.id, 'bindata', BinData.from_spaced_hex(12, '123 456'))
        db.set_data(node.id, 'long', 0x123456789abcdef123456789abcdef)
        db.set_data(node.id, 'neglong', -0x123456789abcdef123456789abcdef)
        n2 = db.get(node.id)
        self.assertEqual(n2.data, {
            'int', 'bool', 'bytes', 'str', 'list', 'dict', 'id', 'bindata',
            'long', 'neglong',
        })
        self.assertEqual(db.get_data(node.id, 'int'), 123)
        self.assertEqual(db.get_data(node.id, 'bool'), False)
        self.assertEqual(db.get_data(node.id, 'none'), None)
        self.assertEqual(db.get_data(node.id, 'meh'), None)
        b = db.get_data(node.id, 'bytes')
        s = db.get_data(node.id, 'str')
        self.assertEqual(b, b'\x01\x02\x03')
        self.assertEqual(s, 'abc')
        self.assertIsInstance(b, bytes)
        self.assertIsInstance(s, six.text_type)
        self.assertEqual(db.get_data(node.id, 'list'), [1, 2, 3])
        self.assertEqual(db.get_data(node.id, 'dict'), {'a': 'b', 'c': 'd'})
        self.assertEqual(db.get_data(node.id, 'id'), id2)
        self.assertEqual(db.get_data(node.id, 'bindata'),
                         BinData.from_spaced_hex(12, '123 456'))
        self.assertEqual(db.get_data(node.id, 'long'),
                         0x123456789abcdef123456789abcdef)
        self.assertEqual(db.get_data(node.id, 'neglong'),
                         -0x123456789abcdef123456789abcdef)
        with self.assertRaises(TypeError):
            db.set_data(node.id, b'zlew', 'zlew')
        with self.assertRaises(TypeError):
            db.set_data(node.id, 123, 456)
        with self.assertRaises(TypeError):
            db.get_data(node.id, b'zlew')
        with self.assertRaises(TypeError):
            db.get_data(node.id, 123)

    def test_bindata(self):
        db = Database(None)
        node = Node(id=NodeID(),
                    tags=set(),
                    attr={},
                    pos_start=None,
                    pos_end=None,
                    data=set(),
                    bindata={'my_bindata': 12345})
        db.create(node)
        db.set_bindata(node.id, 'one', start=0, data=b'')
        db.set_bindata(node.id, 'two', start=0, data=b'\x12\x34\x56')
        db.set_bindata(node.id, 'three', start=0, data=b'\x11' * 0x123456)
        n2 = db.get(node.id)
        self.assertEqual(n2.bindata, {
            'two': 3,
            'three': 0x123456,
        })
        self.assertEqual(db.get_bindata(node.id, 'one'), b'')
        self.assertEqual(db.get_bindata(node.id, 'one', start=3), b'')
        self.assertEqual(db.get_bindata(node.id, 'one', start=3, end=15), b'')
        self.assertEqual(db.get_bindata(node.id, 'two'), b'\x12\x34\x56')
        self.assertEqual(db.get_bindata(node.id, 'two', start=2), b'\x56')
        self.assertEqual(db.get_bindata(node.id, 'two', start=4), b'')
        self.assertEqual(db.get_bindata(node.id, 'two', start=1, end=2),
                         b'\x34')
        self.assertEqual(db.get_bindata(node.id, 'two', start=1, end=6),
                         b'\x34\x56')
        self.assertEqual(db.get_bindata(node.id, 'three'), b'\x11' * 0x123456)
        db.set_bindata(node.id, 'one', start=0, data=b'\x11\x22')
        self.assertEqual(db.get_bindata(node.id, 'one'), b'\x11\x22')
        db.set_bindata(node.id, 'one', start=1, data=b'\x33\x44\x55')
        self.assertEqual(db.get_bindata(node.id, 'one'), b'\x11\x33\x44\x55')
        db.set_bindata(node.id, 'one', start=2, data=b'\x66')
        self.assertEqual(db.get_bindata(node.id, 'one'), b'\x11\x33\x66\x55')
        db.set_bindata(node.id, 'one', start=2, data=b'\x77', truncate=True)
        self.assertEqual(db.get_bindata(node.id, 'one'), b'\x11\x33\x77')
        n2 = db.get(node.id)
        self.assertIn('one', n2.bindata)
        db.set_bindata(node.id, 'one', start=0, data=b'', truncate=True)
        self.assertEqual(db.get_bindata(node.id, 'one'), b'')
        n2 = db.get(node.id)
        self.assertNotIn('one', n2.bindata)
        db.set_bindata(node.id, 'three', start=0x12345, data=b'\x22' * 0x789ab)
        n2 = db.get(node.id)
        self.assertEqual(n2.bindata['three'], 0x123456)
        correct = (b'\x11' * 0x12345 +
                   b'\x22' * 0x789ab +
                   b'\x11' * (0x123456 - 0x789ab - 0x12345))
        self.assertEqual(db.get_bindata(node.id, 'three'), correct)
        with self.assertRaises(WritePastEndError):
            db.set_bindata(node.id, 'three', start=0x1234567, data=b'\x33')
        with self.assertRaises(ValueError):
            db.set_bindata(node.id, 'three', start=-1, data=b'\x33')
        db.set_bindata(node.id, 'three', start=0x123456,
                       data=b'\x33' * 0x789ab)
        n2 = db.get(node.id)
        self.assertEqual(n2.bindata['three'], 0x123456 + 0x789ab)
        correct = (b'\x11' * 0x12345 +
                   b'\x22' * 0x789ab +
                   b'\x11' * (0x123456 - 0x789ab - 0x12345) +
                   b'\x33' * 0x789ab)
        self.assertEqual(db.get_bindata(node.id, 'three'), correct)
        db.set_bindata(node.id, 'three', start=0x6789a,
                       data=b'\x44' * 0x789ab, truncate=True)
        n2 = db.get(node.id)
        self.assertEqual(n2.bindata['three'], 0x6789a + 0x789ab)
        correct = (b'\x11' * 0x12345 +
                   b'\x22' * (0x6789a - 0x12345) +
                   b'\x44' * 0x789ab)
        self.assertEqual(db.get_bindata(node.id, 'three'), correct)
        self.assertEqual(db.get_bindata(node.id, 'three', start=0x789ab),
                         b'\x44' * 0x6789a)
        self.assertEqual(db.get_bindata(node.id, 'three', start=0x789ab,
                                        end=0xdeadbeef),
                         b'\x44' * 0x6789a)
        self.assertEqual(db.get_bindata(node.id, 'three',
                                        start=0x55555, end=0x88888),
                         b'\x22' * 0x12345 + b'\x44' * (0x33333 - 0x12345))
        db.set_bindata(node.id, 'three', start=0x1234, data=b'', truncate=True)
        n2 = db.get(node.id)
        self.assertEqual(n2.bindata['three'], 0x1234)
        correct = b'\x11' * 0x1234
        self.assertEqual(db.get_bindata(node.id, 'three'), correct)
        with self.assertRaises(TypeError):
            db.set_bindata(node.id, b'zlew', 0, b'zlew')
        with self.assertRaises(TypeError):
            db.set_bindata(node.id, 'abc', 0, 'zlew')
        with self.assertRaises(TypeError):
            db.set_bindata(node.id, 'abc', 0, 1234)
        with self.assertRaises(TypeError):
            db.set_bindata(node.id, 'zlew', b'zlew', b'zlew')
        with self.assertRaises(TypeError):
            db.set_bindata(node.id, 'zlew', 0, b'zlew', truncate='zlew')
        with self.assertRaises(TypeError):
            db.get_bindata(node.id, b'zlew')
        with self.assertRaises(TypeError):
            db.get_bindata(node.id, 123)

    # XXX list
