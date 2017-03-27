# Copyright 2016-2017 CodiLime
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

from veles.data.bindata import BinData
from veles.schema.nodeid import NodeID
from veles.schema import fields
from veles.proto.exceptions import SchemaError


class Piwo(object):
    def dump(self):
        return 'piwo'

    @classmethod
    def load(cls, value):
        if value != 'piwo':
            raise SchemaError
        return Piwo()

    def __eq__(self, other):
        return type(self) == type(other)

    def __hash__(self):
        return 13


class Zlew(object):
    def dump(self):
        return 'zlew'

    @classmethod
    def load(cls, value):
        if value != 'zlew':
            raise SchemaError
        return Zlew()

    def __eq__(self, other):
        return type(self) == type(other)

    def __hash__(self):
        return 13


class TestFields(unittest.TestCase):
    def test_field(self):
        a = fields.Any(default='zlew')
        a.__set_name__(None, 'a')
        self.assertEqual(a.name, 'a')
        a.validate(1234)
        a.validate({})
        with self.assertRaises(SchemaError):
            a.validate(None)
        tv = {
            'a': 123,
            'b': 'c',
        }
        self.assertEqual(a.dump(tv), tv)
        self.assertEqual(a.load(tv), tv)
        with self.assertRaises(SchemaError):
            a.load(None)

        with self.assertRaises(TypeError):
            fields.Any(optional='zlew')
        with self.assertRaises(ValueError):
            a = fields.Any(optional=True, default='zlew')

    def test_boolean(self):
        with self.assertRaises(SchemaError):
            fields.Boolean(default='zlew')

        a = fields.Boolean(optional=True)
        a.validate(True)
        a.validate(False)
        a.validate(None)
        with self.assertRaises(SchemaError):
            a.validate(1)
        self.assertEqual(a.dump(None), None)
        self.assertEqual(a.dump(True), True)
        self.assertEqual(a.dump(False), False)
        self.assertEqual(a.load(None), None)
        self.assertEqual(a.load(True), True)
        self.assertEqual(a.load(False), False)
        with self.assertRaises(SchemaError):
            a.load(1)

    def test_float(self):
        a = fields.Float()
        a.validate(1.234)
        with self.assertRaises(SchemaError):
            a.validate(1)
        with self.assertRaises(SchemaError):
            a.validate('1.0')
        self.assertEqual(a.dump(1.234), 1.234)
        self.assertEqual(a.load(1.234), 1.234)
        with self.assertRaises(SchemaError):
            a.load(1)
        with self.assertRaises(SchemaError):
            a.load('1.0')

    def test_string(self):
        a = fields.String()
        a.validate('abcd')
        with self.assertRaises(SchemaError):
            a.validate(b'abcd')
        with self.assertRaises(SchemaError):
            a.validate(1234)
        self.assertEqual(a.dump('abcd'), 'abcd')
        self.assertEqual(a.load('abcd'), 'abcd')
        with self.assertRaises(SchemaError):
            a.load(b'abcd')
        with self.assertRaises(SchemaError):
            a.load(1234)

    def test_binary(self):
        a = fields.Binary()
        a.validate(b'abcd')
        with self.assertRaises(SchemaError):
            a.validate('abcd')
        with self.assertRaises(SchemaError):
            a.validate(1234)
        self.assertEqual(a.dump(b'abcd'), b'abcd')
        self.assertEqual(a.load(b'abcd'), b'abcd')
        with self.assertRaises(SchemaError):
            a.load('abcd')
        with self.assertRaises(SchemaError):
            a.load(1234)

    def test_nodeid(self):
        a = fields.NodeID()
        id = NodeID()
        a.validate(id)
        with self.assertRaises(SchemaError):
            a.validate(b'abcd')
        with self.assertRaises(SchemaError):
            a.validate(1234)
        self.assertEqual(a.dump(id), id)
        self.assertEqual(a.load(id), id)
        with self.assertRaises(SchemaError):
            a.load(b'abcd')
        with self.assertRaises(SchemaError):
            a.load(1234)

    def test_bindata(self):
        a = fields.BinData()
        data = BinData(8, [0x12, 0x34])
        a.validate(data)
        with self.assertRaises(SchemaError):
            a.validate(b'abcd')
        with self.assertRaises(SchemaError):
            a.validate(1234)
        self.assertEqual(a.dump(data), data)
        self.assertEqual(a.load(data), data)
        with self.assertRaises(SchemaError):
            a.load(b'abcd')
        with self.assertRaises(SchemaError):
            a.load(1234)

    def test_integer(self):
        a = fields.Integer()
        a.validate(0)
        a.validate(1)
        a.validate(-1)
        a.validate(0x123456789abcdef123456789abcdef)
        a.validate(-0x123456789abcdef123456789abcdef)
        with self.assertRaises(SchemaError):
            a.validate(False)
        with self.assertRaises(SchemaError):
            a.validate(True)

        a = fields.Integer(minimum=-123, maximum=456)
        a.validate(-123)
        a.validate(123)
        a.validate(234)
        a.validate(456)
        with self.assertRaises(SchemaError):
            a.validate(-0x123456789abcdef123456789abcdef)
        with self.assertRaises(SchemaError):
            a.validate(-124)
        with self.assertRaises(SchemaError):
            a.validate(457)
        with self.assertRaises(SchemaError):
            a.validate(0x123456789abcdef123456789abcdef)

        a = fields.Integer(minimum=123)
        a.validate(123)
        a.validate(234)
        a.validate(456)
        a.validate(0x123456789abcdef123456789abcdef)
        with self.assertRaises(SchemaError):
            a.validate(0)
        with self.assertRaises(SchemaError):
            a.validate(-123)
        with self.assertRaises(SchemaError):
            a.validate(122)

        with self.assertRaises(TypeError):
            fields.Integer(minimum='zlew')
        with self.assertRaises(TypeError):
            fields.Integer(maximum='zlew')
        fields.Integer(minimum=3, maximum=3)
        fields.Integer(minimum=3)
        fields.Integer(maximum=3)
        with self.assertRaises(ValueError):
            fields.Integer(minimum=3, maximum=2)

    def test_unsigned_integer(self):
        a = fields.UnsignedInteger()
        a.validate(0)
        a.validate(1)
        a.validate(123)
        a.validate(0x123456789abcdef123456789abcdef)
        with self.assertRaises(SchemaError):
            a.validate(-122)
        with self.assertRaises(SchemaError):
            a.validate(-1)
        with self.assertRaises(SchemaError):
            a.validate(-0x123456789abcdef123456789abcdef)

        with self.assertRaises(ValueError):
            fields.UnsignedInteger(minimum=-1)
        with self.assertRaises(TypeError):
            fields.UnsignedInteger(minimum=None)
        with self.assertRaises(ValueError):
            fields.UnsignedInteger(maximum=-1)
        fields.UnsignedInteger(maximum=0)
        fields.UnsignedInteger(minimum=123)
        fields.UnsignedInteger(maximum=123)
        fields.UnsignedInteger(maximum=None)

    def test_small_integer(self):
        a = fields.SmallInteger()
        a.validate(0)
        a.validate(-1)
        a.validate(1)
        a.validate(-0x8000000000000000)
        a.validate(0x7fffffffffffffff)
        with self.assertRaises(SchemaError):
            a.validate(-0x8000000000000001)
        with self.assertRaises(SchemaError):
            a.validate(0x8000000000000000)
        fields.SmallInteger(minimum=-1)
        fields.SmallInteger(maximum=-1)
        fields.SmallInteger(maximum=0x7fffffffffffffff)
        fields.SmallInteger(minimum=-0x8000000000000000)
        with self.assertRaises(ValueError):
            fields.SmallInteger(maximum=0x8000000000000000)
        with self.assertRaises(ValueError):
            fields.SmallInteger(minimum=-0x8000000000000001)
        with self.assertRaises(TypeError):
            fields.SmallInteger(maximum=None)
        with self.assertRaises(TypeError):
            fields.SmallInteger(minimum=None)

    def test_small_unsigned_integer(self):
        a = fields.SmallUnsignedInteger()
        a.validate(0)
        a.validate(1)
        a.validate(0xffffffffffffffff)
        with self.assertRaises(SchemaError):
            a.validate(-1)
        with self.assertRaises(SchemaError):
            a.validate(0x10000000000000000)
        fields.SmallUnsignedInteger(minimum=1)
        fields.SmallUnsignedInteger(maximum=1)
        fields.SmallUnsignedInteger(maximum=0xffffffffffffffff)
        fields.SmallUnsignedInteger(minimum=0)
        with self.assertRaises(ValueError):
            fields.SmallUnsignedInteger(maximum=0x10000000000000000)
        with self.assertRaises(ValueError):
            fields.SmallUnsignedInteger(minimum=-1)
        with self.assertRaises(TypeError):
            fields.SmallUnsignedInteger(maximum=None)
        with self.assertRaises(TypeError):
            fields.SmallUnsignedInteger(minimum=None)

    def test_object(self):
        a = fields.Object(Piwo, optional=True)
        a.validate(None)
        a.validate(Piwo())
        with self.assertRaises(SchemaError):
            a.validate('piwo')
        with self.assertRaises(SchemaError):
            a.validate(Zlew())
        self.assertIsInstance(a.load('piwo'), Piwo)
        with self.assertRaises(SchemaError):
            a.load('zlew')
        self.assertEqual(a.load(None), None)
        self.assertEqual(a.dump(Piwo()), 'piwo')
        self.assertEqual(a.dump(None), None)

        a = fields.Object(Zlew)
        a.validate(Zlew())
        with self.assertRaises(SchemaError):
            a.validate('zlew')
        with self.assertRaises(SchemaError):
            a.validate(Piwo())
        with self.assertRaises(SchemaError):
            a.validate(None)
        self.assertIsInstance(a.load('zlew'), Zlew)
        with self.assertRaises(SchemaError):
            a.load('piwo')
        with self.assertRaises(SchemaError):
            a.load(None)
        self.assertEqual(a.dump(Zlew()), 'zlew')

        fields.Object(Zlew, default=Zlew())
        with self.assertRaises(SchemaError):
            fields.Object(Zlew, default=Piwo())

    def test_list(self):
        a = fields.List(fields.Object(Piwo))
        a.validate([])
        a.validate([Piwo()])
        a.validate([Piwo(), Piwo(), Piwo()])
        with self.assertRaises(SchemaError):
            a.validate(Piwo())
        with self.assertRaises(SchemaError):
            a.validate(set())
        with self.assertRaises(SchemaError):
            a.validate(None)
        with self.assertRaises(SchemaError):
            a.validate([Piwo(), Zlew(), Piwo()])
        with self.assertRaises(SchemaError):
            a.validate([Piwo(), None])
        self.assertEqual(a.load([]), [])
        self.assertEqual(a.load(['piwo', 'piwo']), [Piwo(), Piwo()])
        with self.assertRaises(SchemaError):
            a.validate(a.load({}))
        with self.assertRaises(SchemaError):
            a.validate(a.load(None))
        with self.assertRaises(SchemaError):
            a.validate(a.load('piwo'))
        with self.assertRaises(SchemaError):
            a.validate(a.load(['zlew']))
        with self.assertRaises(SchemaError):
            a.validate(a.load(['piwo', None]))
        self.assertEqual(a.dump([]), [])
        self.assertEqual(a.dump([Piwo(), Piwo()]), ['piwo', 'piwo'])
        fields.List(fields.Integer(), default=[1, 2, 3])

    def test_set(self):
        a = fields.Set(fields.Object(Piwo))
        a.validate(set())
        a.validate({Piwo()})
        with self.assertRaises(SchemaError):
            a.validate(Piwo())
        with self.assertRaises(SchemaError):
            a.validate([])
        with self.assertRaises(SchemaError):
            a.validate(None)
        with self.assertRaises(SchemaError):
            a.validate({Piwo(), Zlew()})
        with self.assertRaises(SchemaError):
            a.validate({Piwo(), None})
        self.assertEqual(a.load([]), set())
        self.assertEqual(a.load(['piwo']), {Piwo()})
        with self.assertRaises(SchemaError):
            a.validate(a.load({}))
        with self.assertRaises(SchemaError):
            a.validate(a.load(None))
        with self.assertRaises(SchemaError):
            a.validate(a.load('piwo'))
        with self.assertRaises(SchemaError):
            a.validate(a.load(['zlew']))
        with self.assertRaises(SchemaError):
            a.validate(a.load(['piwo', None]))
        self.assertEqual(a.dump({}), [])
        self.assertEqual(a.dump({Piwo()}), ['piwo'])
        fields.Set(fields.Integer(), default={1, 2, 3})

    def test_map(self):
        a = fields.Map(fields.Object(Piwo), fields.Object(Zlew))
        a.validate({})
        a.validate({Piwo(): Zlew()})
        with self.assertRaises(SchemaError):
            a.validate(Piwo())
        with self.assertRaises(SchemaError):
            a.validate(Zlew())
        with self.assertRaises(SchemaError):
            a.validate([])
        with self.assertRaises(SchemaError):
            a.validate(None)
        with self.assertRaises(SchemaError):
            a.validate({Piwo(): Piwo()})
        with self.assertRaises(SchemaError):
            a.validate({Zlew(): Zlew()})
        with self.assertRaises(SchemaError):
            a.validate({Zlew(): Piwo()})
        with self.assertRaises(SchemaError):
            a.validate({Piwo(): None})
        with self.assertRaises(SchemaError):
            a.validate({None: Zlew()})
        self.assertEqual(a.load({}), {})
        self.assertEqual(a.load({'piwo': 'zlew'}), {Piwo(): Zlew()})
        with self.assertRaises(SchemaError):
            a.validate(a.load([]))
        with self.assertRaises(SchemaError):
            a.validate(a.load(None))
        with self.assertRaises(SchemaError):
            a.validate(a.load('piwo'))
        with self.assertRaises(SchemaError):
            a.validate(a.load({'piwo': 'piwo'}))
        with self.assertRaises(SchemaError):
            a.validate(a.load({'piwo', 'zlew'}))
        self.assertEqual(a.dump({}), {})
        self.assertEqual(a.dump({Piwo(): Zlew()}), {'piwo': 'zlew'})
        fields.Map(fields.Integer(), fields.String(), default={1: 'a'})
