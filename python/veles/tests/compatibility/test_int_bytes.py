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

import unittest

from veles.compatibility.int_bytes import int_to_bytes, int_from_bytes


class TestIntBytes(unittest.TestCase):
    def test_int_from_bytes(self):
        self.assertEqual(int_from_bytes(b'', 'little'), 0)
        self.assertEqual(int_from_bytes(b'', 'big'), 0)
        with self.assertRaises(ValueError):
            int_from_bytes(b'', 'zlew')
        with self.assertRaises(TypeError):
            int_from_bytes(u'zlew', 'little')
        self.assertEqual(int_from_bytes(b'\x12\x34', 'little'), 0x3412)
        self.assertEqual(int_from_bytes(b'\x12\x34', 'big'), 0x1234)
        x = b'\x11\x22\x33\x44\x55\x66\x77\x88\x99\xaa\xbb\xcc\xdd\xee\xff'
        y = 0xffeeddccbbaa998877665544332211
        z = 0x112233445566778899aabbccddeeff
        self.assertEqual(int_from_bytes(x, 'little'), y)
        self.assertEqual(int_from_bytes(x, 'big'), z)

    def test_int_to_bytes(self):
        self.assertEqual(int_to_bytes(0, 0, 'little'), b'')
        self.assertEqual(int_to_bytes(0, 0, 'big'), b'')
        with self.assertRaises(TypeError):
            int_to_bytes('zlew', 4, 'little')
        with self.assertRaises(TypeError):
            int_to_bytes(0x1234, 'zlew', 'little')
        with self.assertRaises(ValueError):
            int_to_bytes(0x1234, 4, 'zlew')
        with self.assertRaises(OverflowError):
            int_to_bytes(1, 0, 'little')
        with self.assertRaises(OverflowError):
            int_to_bytes(-1, 3, 'little')
        self.assertEqual(int_to_bytes(0x1234, 2, 'little'), b'\x34\x12')
        self.assertEqual(int_to_bytes(0x1234, 2, 'big'), b'\x12\x34')
        self.assertEqual(int_to_bytes(0x1234, 3, 'little'), b'\x34\x12\x00')
        self.assertEqual(int_to_bytes(0x1234, 3, 'big'), b'\x00\x12\x34')
        with self.assertRaises(OverflowError):
            int_to_bytes(0x1234, 1, 'big')
        with self.assertRaises(OverflowError):
            int_to_bytes(-0x1234, 2, 'big')
        self.assertEqual(int_to_bytes(0xffff, 2, 'big'), b'\xff\xff')
        with self.assertRaises(OverflowError):
            int_to_bytes(0x10000, 2, 'big')
        x = 0x112233445566778899aabbccddeeff
        y = b'\x11\x22\x33\x44\x55\x66\x77\x88\x99\xaa\xbb\xcc\xdd\xee\xff'
        z = b'\xff\xee\xdd\xcc\xbb\xaa\x99\x88\x77\x66\x55\x44\x33\x22\x11'
        self.assertEqual(int_to_bytes(x, 15, 'big'), y)
        self.assertEqual(int_to_bytes(x, 15, 'little'), z)
