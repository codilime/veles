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

from veles.data.bindata import BinData


class TestBinData(unittest.TestCase):
    def test_simple(self):
        a = BinData(8, [0x12])
        self.assertEqual(a.width, 8)
        self.assertEqual(a.raw_data, b'\x12')
        self.assertEqual(a.octets_per_element(), 1)
        self.assertEqual(a.octets(), 1)
        self.assertEqual(len(a), 1)
        self.assertEqual(a[0], 0x12)
        self.assertEqual(a[-1], 0x12)
        self.assertEqual(list(a), [0x12])
        self.assertEqual(a, a[:])
        self.assertEqual(str(a), '12')
        self.assertEqual(repr(a),
                         'BinData.from_spaced_hex(8, \'12\')')

    def test_fib_8(self):
        a = BinData(8, [1, 2, 3, 5, 8])
        self.assertEqual(a.width, 8)
        self.assertEqual(a.raw_data, b'\x01\x02\x03\x05\x08')
        self.assertEqual(a.octets_per_element(), 1)
        self.assertEqual(a.octets(), 5)
        self.assertEqual(len(a), 5)
        self.assertEqual(a[0], 1)
        self.assertEqual(a[4], 8)
        self.assertEqual(a[-1], 8)
        self.assertEqual(list(a), [1, 2, 3, 5, 8])
        self.assertEqual(a, a[:])
        self.assertEqual(str(a), '01 02 03 05 08')
        self.assertEqual(repr(a),
                         'BinData.from_spaced_hex(8, \'01 02 03 05 08\')')

    def test_fib_7(self):
        a = BinData(7, [1, 2, 3, 5, 8])
        self.assertEqual(a.width, 7)
        self.assertEqual(a.raw_data, b'\x01\x02\x03\x05\x08')
        self.assertEqual(a.octets_per_element(), 1)
        self.assertEqual(a.octets(), 5)
        self.assertEqual(len(a), 5)
        self.assertEqual(a[0], 1)
        self.assertEqual(a[4], 8)
        self.assertEqual(a[-1], 8)
        self.assertEqual(list(a), [1, 2, 3, 5, 8])
        self.assertEqual(a, a[:])
        self.assertEqual(str(a), '01 02 03 05 08')
        self.assertEqual(repr(a),
                         'BinData.from_spaced_hex(7, \'01 02 03 05 08\')')

    def test_fib_9(self):
        a = BinData(9, [1, 2, 3, 5, 8])
        self.assertEqual(a.width, 9)
        self.assertEqual(a.raw_data,
                         b'\x01\x00\x02\x00\x03\x00\x05\x00\x08\x00')
        self.assertEqual(a.octets_per_element(), 2)
        self.assertEqual(a.octets(), 10)
        self.assertEqual(len(a), 5)
        self.assertEqual(a[0], 1)
        self.assertEqual(a[4], 8)
        self.assertEqual(a[-1], 8)
        self.assertEqual(list(a), [1, 2, 3, 5, 8])
        self.assertEqual(a, a[:])
        self.assertEqual(str(a), '001 002 003 005 008')
        self.assertEqual(repr(a),
                         'BinData.from_spaced_hex(9, \'001 002 003 005 008\')')

    def test_19(self):
        a = BinData(19, [0x12345, 0x6789a])
        self.assertEqual(a.width, 19)
        self.assertEqual(a.raw_data, b'\x45\x23\x01\x9a\x78\x06')
        self.assertEqual(a.octets_per_element(), 3)
        self.assertEqual(a.octets(), 6)
        self.assertEqual(len(a), 2)
        self.assertEqual(a[0], 0x12345)
        self.assertEqual(a[1], 0x6789a)
        self.assertEqual(a[-1], 0x6789a)
        self.assertEqual(list(a), [0x12345, 0x6789a])
        self.assertEqual(a, a[:])
        self.assertEqual(str(a), '12345 6789a')
        self.assertEqual(repr(a),
                         'BinData.from_spaced_hex(19, \'12345 6789a\')')

    def test_null(self):
        a = BinData(8)
        self.assertEqual(a.width, 8)
        self.assertEqual(a.raw_data, b'')
        self.assertEqual(a.octets_per_element(), 1)
        self.assertEqual(a.octets(), 0)
        self.assertEqual(len(a), 0)
        self.assertEqual(list(a), [])
        self.assertEqual(a, a[:])
        self.assertEqual(str(a), '')
        self.assertEqual(repr(a),
                         'BinData.from_spaced_hex(8, \'\')')

    def test_init_bytes(self):
        a = BinData(8, bytearray(b'12'))
        self.assertEqual(a.width, 8)
        self.assertEqual(a.raw_data, b'12')
        self.assertEqual(a.octets_per_element(), 1)
        self.assertEqual(a.octets(), 2)
        self.assertEqual(len(a), 2)
        self.assertEqual(list(a), [0x31, 0x32])
        self.assertEqual(a, a[:])
        self.assertEqual(str(a), '31 32')
        self.assertEqual(repr(a),
                         'BinData.from_spaced_hex(8, \'31 32\')')

    def test_init_str(self):
        a = BinData.from_spaced_hex(12, '12')
        self.assertEqual(a.width, 12)
        self.assertEqual(a.raw_data, b'\x12\x00')
        self.assertEqual(a.octets_per_element(), 2)
        self.assertEqual(a.octets(), 2)
        self.assertEqual(len(a), 1)
        self.assertEqual(list(a), [0x12])
        self.assertEqual(a, a[:])
        self.assertEqual(str(a), '012')
        self.assertEqual(repr(a),
                         'BinData.from_spaced_hex(12, \'012\')')

    def test_init_num(self):
        a = BinData(12, 12)
        self.assertEqual(a.width, 12)
        self.assertEqual(a.raw_data, bytes(bytearray(24)))
        self.assertEqual(a.octets_per_element(), 2)
        self.assertEqual(a.octets(), 24)
        self.assertEqual(len(a), 12)
        self.assertEqual(list(a), [0] * 12)
        self.assertEqual(a, a[:])
        s = ' '.join('000' for _ in range(12))
        self.assertEqual(str(a), s)
        self.assertEqual(repr(a),
                         'BinData.from_spaced_hex(12, \'{}\')'.format(s))

    def test_init_crap(self):
        with self.assertRaises(TypeError):
            BinData(8.0)
        with self.assertRaises(ValueError):
            BinData(0)
        with self.assertRaises(ValueError):
            BinData(-8)
        with self.assertRaises(TypeError):
            BinData(8, [3.0])
        with self.assertRaises(TypeError):
            BinData(8, 3.0)
        with self.assertRaises(TypeError):
            BinData(8, u'meh')
        BinData(7, [0x7f])
        with self.assertRaises(ValueError):
            BinData(7, [0x80])
        with self.assertRaises(ValueError):
            BinData(8, [-1])

    def test_from_raw_8(self):
        a = BinData.from_raw_data(8, b'\x01\x02\x03\x05\x08')
        self.assertEqual(a.width, 8)
        self.assertEqual(a.raw_data, b'\x01\x02\x03\x05\x08')
        self.assertEqual(a.octets_per_element(), 1)
        self.assertEqual(a.octets(), 5)
        self.assertEqual(len(a), 5)
        self.assertEqual(a[0], 1)
        self.assertEqual(a[4], 8)
        self.assertEqual(a[-1], 8)
        self.assertEqual(list(a), [1, 2, 3, 5, 8])
        self.assertEqual(a, a[:])
        self.assertEqual(str(a), '01 02 03 05 08')
        self.assertEqual(repr(a),
                         'BinData.from_spaced_hex(8, \'01 02 03 05 08\')')

    def test_from_raw_9(self):
        a = BinData.from_raw_data(9,
                                  b'\x01\x00\x02\x00\x03\x00\x05\x00\x08\x00')
        self.assertEqual(a.width, 9)
        self.assertEqual(a.raw_data,
                         b'\x01\x00\x02\x00\x03\x00\x05\x00\x08\x00')
        self.assertEqual(a.octets_per_element(), 2)
        self.assertEqual(a.octets(), 10)
        self.assertEqual(len(a), 5)
        self.assertEqual(a[0], 1)
        self.assertEqual(a[4], 8)
        self.assertEqual(a[-1], 8)
        self.assertEqual(list(a), [1, 2, 3, 5, 8])
        self.assertEqual(a, a[:])
        self.assertEqual(str(a), '001 002 003 005 008')
        self.assertEqual(repr(a),
                         'BinData.from_spaced_hex(9, \'001 002 003 005 008\')')

    def test_from_raw_crap(self):
        with self.assertRaises(TypeError):
            BinData.from_raw_data(9.0, b'\x12')
        with self.assertRaises(TypeError):
            BinData.from_raw_data(9, u'meh')
        with self.assertRaises(ValueError):
            BinData.from_raw_data(-8, b'\x12')
        with self.assertRaises(ValueError):
            BinData.from_raw_data(0, b'\x12')
        with self.assertRaises(ValueError):
            BinData.from_raw_data(9, b'\x12')
        BinData.from_raw_data(9, b'\x12\x01')
        with self.assertRaises(ValueError):
            BinData.from_raw_data(9, b'\x12\x02')
        BinData.from_raw_data(9, b'\xff\x01\xff\x01')
        with self.assertRaises(ValueError):
            BinData.from_raw_data(9, b'\xff\x02\xff\x01')
        with self.assertRaises(ValueError):
            BinData.from_raw_data(9, b'\xff\x01\xff\x02')
        with self.assertRaises(ValueError):
            BinData.from_raw_data(9, b'\xff\x01\xff\x80')
        BinData.from_raw_data(15, b'\xff\x7f\xff\x7f')
        with self.assertRaises(ValueError):
            BinData.from_raw_data(15, b'\xff\xff\xff\x7f')
        with self.assertRaises(ValueError):
            BinData.from_raw_data(15, b'\xff\x7f\xff\xff')

    def test_eq(self):
        a = BinData(8, [0x00])
        b = BinData(8, [0x00])
        c = BinData(8, [0x01])
        d = BinData(7, [0x00])
        e = BinData(8, [])
        self.assertEqual(a, b)
        self.assertEqual(hash(a), hash(b))
        self.assertNotEqual(a, c)
        self.assertNotEqual(a, d)
        self.assertNotEqual(a, e)
        self.assertNotEqual(a, 3.0)
        self.assertNotEqual(a, [0])
        self.assertNotEqual(a, b'\x00')
        self.assertNotEqual(a, 'zlew')

    def test_getitem(self):
        a = BinData(19, [0x12345, 0x6789a, 0x3cdef])
        self.assertEqual(a[-3], 0x12345)
        self.assertEqual(a[-2], 0x6789a)
        self.assertEqual(a[-1], 0x3cdef)
        self.assertEqual(a[0], 0x12345)
        self.assertEqual(a[1], 0x6789a)
        self.assertEqual(a[2], 0x3cdef)
        with self.assertRaises(TypeError):
            a['zlew']
        with self.assertRaises(TypeError):
            a[2.0]
        with self.assertRaises(IndexError):
            a[-4]
        with self.assertRaises(IndexError):
            a[3]

    def test_setitem(self):
        a = BinData(19, [0x12345, 0x6789a, 0x3cdef])
        b = a[:]
        a[0] = 0x54321
        a[-1] = 0x7edcb
        self.assertEqual(a[-3], 0x54321)
        self.assertEqual(a[-2], 0x6789a)
        self.assertEqual(a[-1], 0x7edcb)
        self.assertEqual(a[0], 0x54321)
        self.assertEqual(a[1], 0x6789a)
        self.assertEqual(a[2], 0x7edcb)
        self.assertEqual(b[-3], 0x12345)
        self.assertEqual(b[-2], 0x6789a)
        self.assertEqual(b[-1], 0x3cdef)
        self.assertEqual(b[0], 0x12345)
        self.assertEqual(b[1], 0x6789a)
        self.assertEqual(b[2], 0x3cdef)
        with self.assertRaises(TypeError):
            a['zlew'] = 0x12345
        with self.assertRaises(TypeError):
            a[2.0] = 0x12345
        with self.assertRaises(IndexError):
            a[-4] = 0x12345
        with self.assertRaises(IndexError):
            a[3] = 0x12345
        with self.assertRaises(TypeError):
            a[0] = 'zlew'
        with self.assertRaises(TypeError):
            a[0] = 3.0
        with self.assertRaises(ValueError):
            a[2] = 0xbcdef

    def test_getslice(self):
        a = BinData(19, [0x12345, 0x6789a, 0x3cdef])
        self.assertEqual(a[:], a)
        self.assertEqual(a[:],
                         BinData.from_spaced_hex(19, '12345 6789a 3cdef'))
        self.assertEqual(a[1:], BinData.from_spaced_hex(19, '6789a 3cdef'))
        self.assertEqual(a[4:], BinData.from_spaced_hex(19, ''))
        self.assertEqual(a[-1:], BinData.from_spaced_hex(19, '3cdef'))
        self.assertEqual(a[:-1], BinData.from_spaced_hex(19, '12345 6789a'))
        self.assertEqual(a[:1], BinData.from_spaced_hex(19, '12345'))
        self.assertEqual(a[1:2], BinData.from_spaced_hex(19, '6789a'))
        self.assertEqual(a[2:1], BinData.from_spaced_hex(19, ''))
        self.assertEqual(a[::-1],
                         BinData.from_spaced_hex(19, '3cdef 6789a 12345'))
        self.assertEqual(a[::2], BinData.from_spaced_hex(19, '12345 3cdef'))
        self.assertEqual(a[1::2], BinData.from_spaced_hex(19, '6789a'))
        b = [5 ** x for x in range(5)]
        c = BinData(10, b)
        for start in range(-10, 10):
            for end in range(-10, 10):
                for stride in range(-10, 10):
                    if stride != 0:
                        t = b[start:end:stride]
                        self.assertEqual(list(c[start:end:stride]), t)
        with self.assertRaises(TypeError):
            a[3.0:]
        with self.assertRaises(TypeError):
            a[:3.0]
        with self.assertRaises(TypeError):
            a[::3.0]
        with self.assertRaises(TypeError):
            a['zlew':]

    def test_setslice(self):
        a = BinData(19, [0x12345, 0x6789a, 0x3cdef])
        a[:] = BinData.from_spaced_hex(19, '54321 29876 7edcb')
        self.assertEqual(a, BinData.from_spaced_hex(19, '54321 29876 7edcb'))
        a[:] = BinData.from_spaced_hex(19, '11111 22222')
        self.assertEqual(a, BinData.from_spaced_hex(19, '11111 22222'))
        a = BinData(19, [0x12345, 0x6789a, 0x3cdef])
        a[1:2] = BinData.from_spaced_hex(19, '54321 29876 7edcb')
        c = BinData.from_spaced_hex(19, '12345 54321 29876 7edcb 3cdef')
        self.assertEqual(a, c)
        a[::-2] = BinData.from_spaced_hex(19, '11111 22222 33333')
        c = BinData.from_spaced_hex(19, '33333 54321 22222 7edcb 11111')
        self.assertEqual(a, c)
        with self.assertRaises(TypeError):
            a[3.0:] = BinData.from_spaced_hex(19, '11111 22222')
        with self.assertRaises(TypeError):
            a[:3.0] = BinData.from_spaced_hex(19, '11111 22222')
        with self.assertRaises(TypeError):
            a[::3.0] = BinData.from_spaced_hex(19, '11111 22222')
        with self.assertRaises(TypeError):
            a['zlew':] = BinData.from_spaced_hex(19, '11111 22222')
        with self.assertRaises(TypeError):
            a[:] = 'zlew'
        with self.assertRaises(TypeError):
            a[:] = [1, 2, 3]
        with self.assertRaises(TypeError):
            a[:] = b'\x12\x34'
        with self.assertRaises(ValueError):
            a[:] = BinData.from_spaced_hex(20, '11111 22222 33333')
        with self.assertRaises(ValueError):
            a[::2] = BinData.from_spaced_hex(19, '11111 22222')

    def test_add(self):
        a = BinData.from_spaced_hex(19, '11111 22222 33333')
        b = BinData.from_spaced_hex(19, '44444 55555')
        c = BinData.from_spaced_hex(20, '66666 77777')
        d = BinData.from_spaced_hex(19, '11111 22222 33333 44444 55555')
        self.assertEqual(a + b, d)
        with self.assertRaises(ValueError):
            a + c
        with self.assertRaises(TypeError):
            a + 'zlew'
        with self.assertRaises(TypeError):
            'zlew' + a
