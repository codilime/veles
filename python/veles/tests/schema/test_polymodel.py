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

import six

from veles.schema import fields
from veles.schema.model import PolymorphicModel
from veles.proto.exceptions import SchemaError


class BaseNieZlew(PolymorphicModel):
    pass


class NieZlew(BaseNieZlew):
    object_type = 'nie_zlew'
    pole = fields.String(optional=True)


class BaseZlew(PolymorphicModel):
    imie = fields.String(optional=True)


class Zlew(BaseZlew):
    object_type = 'zlew'
    odplyw = fields.String()


class TurboZlew(Zlew):
    object_type = 'turbozlew'
    dopalacz = fields.Binary()


class WieloZlew(BaseZlew):
    przeplyw = fields.Integer(default=13)


class DwuZlew(WieloZlew):
    object_type = 'dwuzlew'
    lewy = fields.Object(Zlew)
    prawy = fields.Object(Zlew)


class PietroZlew(WieloZlew):
    object_type = 'pietrozlew'
    pietra = fields.List(fields.Object(BaseZlew))


class TestModel(unittest.TestCase):
    def test_fields(self):
        self.assertEqual(set(BaseZlew.fields), {
            BaseZlew.imie
        })
        self.assertEqual(set(Zlew.fields), {
            BaseZlew.imie, Zlew.odplyw
        })
        self.assertEqual(set(TurboZlew.fields), {
            BaseZlew.imie, Zlew.odplyw, TurboZlew.dopalacz
        })
        self.assertEqual(set(WieloZlew.fields), {
            BaseZlew.imie, WieloZlew.przeplyw
        })
        self.assertEqual(set(DwuZlew.fields), {
            BaseZlew.imie, WieloZlew.przeplyw, DwuZlew.lewy, DwuZlew.prawy
        })
        self.assertEqual(set(PietroZlew.fields), {
            BaseZlew.imie, WieloZlew.przeplyw, PietroZlew.pietra
        })

    def test_object_types(self):
        self.assertEqual(set(BaseZlew.object_types), {
            'zlew', 'turbozlew', 'dwuzlew', 'pietrozlew',
        })

    def test_init(self):
        a = Zlew(odplyw='o')
        b = TurboZlew(odplyw='wzium', dopalacz=b'\xf3\x90')
        c = DwuZlew(lewy=a, prawy=b)
        d = PietroZlew(imie='Jasiu', pietra=[c], przeplyw=1)
        with self.assertRaises(TypeError):
            BaseZlew(imie='Sid')
        with self.assertRaises(TypeError):
            WieloZlew(imie='Legion')
        with self.assertRaises(SchemaError):
            DwuZlew(lewy=a, prawy=d)

    def test_dump(self):
        a = Zlew(odplyw='o')
        b = TurboZlew(odplyw='wzium', dopalacz=b'\xf3\x90')
        c = DwuZlew(lewy=a, prawy=b)
        d = PietroZlew(imie='Jasiu', pietra=[c], przeplyw=1)
        da = a.dump()
        db = b.dump()
        dc = c.dump()
        dd = d.dump()
        for x in da:
            self.assertIsInstance(x, six.text_type)
        for x in db:
            self.assertIsInstance(x, six.text_type)
        for x in dc:
            self.assertIsInstance(x, six.text_type)
        for x in dd:
            self.assertIsInstance(x, six.text_type)
        self.assertEqual(da, {
            'object_type': 'zlew',
            'imie': None,
            'odplyw': 'o',
        })
        self.assertEqual(db, {
            'object_type': 'turbozlew',
            'imie': None,
            'odplyw': 'wzium',
            'dopalacz': b'\xf3\x90',
        })
        self.assertEqual(dc, {
            'object_type': 'dwuzlew',
            'imie': None,
            'lewy': da,
            'prawy': db,
            'przeplyw': 13,
        })
        self.assertEqual(dd, {
            'object_type': 'pietrozlew',
            'imie': 'Jasiu',
            'pietra': [dc],
            'przeplyw': 1,
        })

    def test_load(self):
        a = Zlew(odplyw='o')
        b = TurboZlew(odplyw='wzium', dopalacz=b'\xf3\x90')
        c = DwuZlew(lewy=a, prawy=b)
        d = PietroZlew(imie='Jasiu', pietra=[c], przeplyw=1)
        da = a.dump()
        db = b.dump()
        dc = c.dump()
        dd = d.dump()
        self.assertEqual(BaseZlew.load(da), a)
        self.assertEqual(Zlew.load(da), a)
        with self.assertRaises(SchemaError):
            TurboZlew.load(da)
        with self.assertRaises(SchemaError):
            WieloZlew.load(da)
        with self.assertRaises(SchemaError):
            DwuZlew.load(da)
        with self.assertRaises(SchemaError):
            PietroZlew.load(da)
        with self.assertRaises(SchemaError):
            NieZlew.load(da)
        with self.assertRaises(SchemaError):
            BaseNieZlew.load(da)

        self.assertEqual(BaseZlew.load(db), b)
        self.assertEqual(Zlew.load(db), b)
        self.assertEqual(TurboZlew.load(db), b)
        with self.assertRaises(SchemaError):
            WieloZlew.load(db)
        with self.assertRaises(SchemaError):
            DwuZlew.load(db)
        with self.assertRaises(SchemaError):
            PietroZlew.load(db)
        with self.assertRaises(SchemaError):
            NieZlew.load(db)
        with self.assertRaises(SchemaError):
            BaseNieZlew.load(db)

        self.assertEqual(BaseZlew.load(dc), c)
        self.assertEqual(WieloZlew.load(dc), c)
        self.assertEqual(DwuZlew.load(dc), c)
        with self.assertRaises(SchemaError):
            Zlew.load(dc)
        with self.assertRaises(SchemaError):
            TurboZlew.load(dc)
        with self.assertRaises(SchemaError):
            PietroZlew.load(dc)
        with self.assertRaises(SchemaError):
            NieZlew.load(dc)
        with self.assertRaises(SchemaError):
            BaseNieZlew.load(dc)

        self.assertEqual(BaseZlew.load(dd), d)
        self.assertEqual(WieloZlew.load(dd), d)
        self.assertEqual(PietroZlew.load(dd), d)
        with self.assertRaises(SchemaError):
            Zlew.load(dd)
        with self.assertRaises(SchemaError):
            TurboZlew.load(dd)
        with self.assertRaises(SchemaError):
            DwuZlew.load(dd)
        with self.assertRaises(SchemaError):
            NieZlew.load(dd)
        with self.assertRaises(SchemaError):
            BaseNieZlew.load(dd)

        with self.assertRaises(SchemaError):
            BaseZlew.load({})
        with self.assertRaises(SchemaError):
            BaseZlew.load({'object_type': 'nie_zlew'})
