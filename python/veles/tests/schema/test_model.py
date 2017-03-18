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

import six

from veles.data.bindata import BinData
from veles.schema import fields
from veles.schema.model import Model
from veles.proto.exceptions import SchemaError


class Piwo(Model):
    a = fields.Boolean()


class Zlew(Model):
    b = fields.List(fields.Map(fields.String(), fields.BinData()))


class TurboZlew(Zlew):
    c = fields.Set(fields.Binary())
    d = fields.Object(Piwo)
    e = fields.Integer(optional=True, default=3)


class TestModel(unittest.TestCase):
    def test_field_name(self):
        self.assertEqual(Piwo.a.name, 'a')
        self.assertEqual(Zlew.b.name, 'b')
        self.assertEqual(Zlew.b.element.name, 'b.element')
        self.assertEqual(Zlew.b.element.key.name, 'b.element.key')
        self.assertEqual(Zlew.b.element.value.name, 'b.element.value')
        self.assertEqual(TurboZlew.c.name, 'c')
        self.assertEqual(TurboZlew.c.element.name, 'c.element')
        self.assertEqual(set(Piwo.fields), {Piwo.a})
        self.assertEqual(set(Zlew.fields), {Zlew.b})
        self.assertEqual(set(TurboZlew.fields), {
            Zlew.b, TurboZlew.c, TurboZlew.d, TurboZlew.e})

    def test_init(self):
        a = Piwo(a=True)
        b = Piwo(a=True)
        c = Piwo(a=False)
        d = Zlew()
        e = TurboZlew(d=a)
        self.assertEqual(a.a, True)
        self.assertEqual(c.a, False)
        self.assertEqual(d.b, [])
        self.assertEqual(e.b, [])
        self.assertEqual(e.c, set())
        self.assertEqual(e.d, a)
        self.assertEqual(e.e, 3)
        self.assertEqual(a, a)
        self.assertEqual(d, d)
        self.assertEqual(e, e)
        self.assertEqual(a, b)
        self.assertNotEqual(a, c)
        self.assertNotEqual(a, d)
        self.assertNotEqual(a, e)
        self.assertNotEqual(d, e)
        with self.assertRaises(SchemaError):
            Piwo(a=None)
        with self.assertRaises(SchemaError):
            Piwo()
        with self.assertRaises(TypeError):
            Piwo(a=True, b='zlew')
        with self.assertRaises(SchemaError):
            TurboZlew()
        with self.assertRaises(SchemaError):
            TurboZlew(d=d)

    def test_dump(self):
        a = Piwo(a=True)
        da = a.dump()
        self.assertEqual(da, {'a': True})
        for x in da:
            self.assertIsInstance(x, six.text_type)

        b = Zlew(b=[{'a': BinData(8, []), 'b': BinData(12, [0x123])}, {}])
        db = b.dump()
        self.assertEqual(db, {
            'b': [
                {
                    'a': BinData(8, []),
                    'b': BinData(12, [0x123]),
                },
                {},
            ]
        })
        for x in db:
            self.assertIsInstance(x, six.text_type)

        c = TurboZlew(b=[{}], c={b'abc', b'def'}, d=Piwo(a=False), e=7)
        dc = c.dump()
        self.assertEqual(dc, {
            'b': [{}],
            'c': dc['c'],
            'd': {'a': False},
            'e': 7,
        })
        self.assertIsInstance(dc['c'], list)
        self.assertEqual(set(dc['c']), {b'abc', b'def'})

        d = TurboZlew(d=a)
        dd = d.dump()
        self.assertEqual(dd, {
            'b': [],
            'c': [],
            'd': {'a': True},
            'e': 3,
        })

    def test_load(self):
        a = Piwo.load({
            'a': True,
        })
        self.assertEqual(a, Piwo(a=True))
        b = Zlew.load({})
        self.assertEqual(b, Zlew())
        c = Zlew.load({'b': [{}]})
        self.assertEqual(c, Zlew(b=[{}]))
        d = TurboZlew.load({
            'd': {'a': True}
        })
        self.assertEqual(d, TurboZlew(d=a))
        with self.assertRaises(SchemaError):
            Piwo.load({})
        with self.assertRaises(SchemaError):
            Piwo.load({'a': None})
        with self.assertRaises(SchemaError):
            Piwo.load({'a': True, 'b': True})
        with self.assertRaises(SchemaError):
            Piwo.load('piwo')
        with self.assertRaises(SchemaError):
            Zlew.load([])
        with self.assertRaises(SchemaError):
            Zlew.load({'b': {}})
        with self.assertRaises(SchemaError):
            Zlew.load({'d': {'a': False}})
