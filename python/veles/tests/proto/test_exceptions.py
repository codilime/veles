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


import unittest

from veles.proto.exceptions import VelesException, ObjectGoneError


class TestExceptions(unittest.TestCase):
    def test_exceptions(self):
        a = VelesException('zlew', 'Something went wrong.')
        self.assertEqual(type(a), VelesException)
        self.assertIsInstance(a, VelesException)
        self.assertNotIsInstance(a, ObjectGoneError)
        self.assertEqual(a.code, 'zlew')
        self.assertEqual(a.msg, 'Something went wrong.')
        self.assertEqual(a.args, (a.code, a.msg))
        a = VelesException('object_gone', 'Object gone.')
        self.assertEqual(type(a), ObjectGoneError)
        self.assertIsInstance(a, VelesException)
        self.assertIsInstance(a, ObjectGoneError)
        self.assertEqual(a.code, 'object_gone')
        self.assertEqual(a.msg, 'Object gone.')
        self.assertEqual(a.args, (a.msg,))
        a = ObjectGoneError('Whoops.')
        self.assertEqual(type(a), ObjectGoneError)
        self.assertIsInstance(a, VelesException)
        self.assertIsInstance(a, ObjectGoneError)
        self.assertEqual(a.code, 'object_gone')
        self.assertEqual(a.msg, 'Whoops.')
        self.assertEqual(a.args, (a.msg,))
        a = ObjectGoneError()
        self.assertEqual(type(a), ObjectGoneError)
        self.assertIsInstance(a, VelesException)
        self.assertIsInstance(a, ObjectGoneError)
        self.assertEqual(a.code, 'object_gone')
        self.assertEqual(a.msg, ObjectGoneError.msg)
        self.assertEqual(a.args, ())
