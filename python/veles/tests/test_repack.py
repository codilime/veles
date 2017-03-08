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
from veles.data.repack import Endian, Repacker


class TestRepacker(unittest.TestCase):
    def test_endian(self):
        self.assertNotEqual(Endian.LITTLE, Endian.BIG)

    def test_simple_copy(self):
        r = Repacker(endian=Endian.LITTLE, from_width=8, to_width=8)
        self.assertEqual(r.repack_unit, 8)
        self.assertEqual(r.repack_size(num_elements=2), 2)
        self.assertEqual(r.repackable_size(from_size=2), 2)
        a = BinData(8, [1, 2, 3, 4])
        b = r.repack(a, start=1, num_elements=2)
        self.assertEqual(b, BinData(8, [2, 3]))
        self.assertEqual(r.repack(a), a)

    def test_gather_8to16_little(self):
        r = Repacker(endian=Endian.LITTLE, from_width=8, to_width=16)
        self.assertEqual(r.repack_unit, 16)
        self.assertEqual(r.repack_size(2), 4)
        self.assertEqual(r.repackable_size(2), 1)
        self.assertEqual(r.repackable_size(3), 1)
        self.assertEqual(r.repackable_size(4), 2)
        a = BinData(8, [1, 2, 3, 4, 5, 6])
        b = r.repack(a, start=1, num_elements=2)
        self.assertEqual(b, BinData.from_spaced_hex(16, '0302 0504'))
        c = r.repack(a, start=1)
        self.assertEqual(b, c)
        d = r.repack(a)
        self.assertEqual(d, BinData.from_spaced_hex(16, '0201 0403 0605'))

    def test_gather_8to16_big(self):
        r = Repacker(endian=Endian.BIG, from_width=8, to_width=16)
        self.assertEqual(r.repack_unit, 16)
        self.assertEqual(r.repack_size(2), 4)
        self.assertEqual(r.repackable_size(2), 1)
        self.assertEqual(r.repackable_size(3), 1)
        self.assertEqual(r.repackable_size(4), 2)
        a = BinData(8, [1, 2, 3, 4, 5, 6])
        b = r.repack(a, start=1, num_elements=2)
        self.assertEqual(b, BinData.from_spaced_hex(16, '0203 0405'))
        c = r.repack(a, start=1)
        self.assertEqual(b, c)
        d = r.repack(a)
        self.assertEqual(d, BinData.from_spaced_hex(16, '0102 0304 0506'))

    def test_mash_8to12_little(self):
        r = Repacker(Endian.LITTLE, 8, 12)
        self.assertEqual(r.repack_unit, 24)
        self.assertEqual(r.repack_size(1), 2)
        self.assertEqual(r.repack_size(2), 3)
        self.assertEqual(r.repackable_size(1), 0)
        self.assertEqual(r.repackable_size(2), 1)
        self.assertEqual(r.repackable_size(3), 2)
        self.assertEqual(r.repackable_size(4), 2)
        a = BinData.from_spaced_hex(8, '12 34 56 78 9a')
        b = r.repack(a, 1, 2)
        self.assertEqual(b, BinData.from_spaced_hex(12, '634 785'))
        c = r.repack(a, 1)
        self.assertEqual(b, c)
        d = r.repack(a)
        self.assertEqual(d, BinData.from_spaced_hex(12, '412 563 a78'))

    def test_mash_8to12_big(self):
        r = Repacker(Endian.BIG, 8, 12)
        self.assertEqual(r.repack_unit, 24)
        self.assertEqual(r.repack_size(1), 2)
        self.assertEqual(r.repack_size(2), 3)
        self.assertEqual(r.repackable_size(1), 0)
        self.assertEqual(r.repackable_size(2), 1)
        self.assertEqual(r.repackable_size(3), 2)
        self.assertEqual(r.repackable_size(4), 2)
        a = BinData.from_spaced_hex(8, '12 34 56 78 9a')
        b = r.repack(a, 1, 2)
        self.assertEqual(b, BinData.from_spaced_hex(12, '345 678'))
        c = r.repack(a, 1)
        self.assertEqual(b, c)
        d = r.repack(a)
        self.assertEqual(d, BinData.from_spaced_hex(12, '123 456 789'))

    def test_split_8to1_little(self):
        r = Repacker(Endian.LITTLE, 8, 1)
        self.assertEqual(r.repack_unit, 8)
        self.assertEqual(r.repack_size(12), 2)
        self.assertEqual(r.repack_size(8), 1)
        self.assertEqual(r.repack_size(9), 2)
        self.assertEqual(r.repack_size(17), 3)
        self.assertEqual(r.repackable_size(1), 8)
        a = BinData.from_spaced_hex(8, '12 34 56')
        b = r.repack(a, 1, 12)
        c = BinData.from_spaced_hex(1, ' '.join(format(0x634, '012b')[::-1]))
        self.assertEqual(b, c)

    def test_split_8to1_big(self):
        r = Repacker(Endian.BIG, 8, 1)
        self.assertEqual(r.repack_unit, 8)
        self.assertEqual(r.repack_size(12), 2)
        self.assertEqual(r.repack_size(8), 1)
        self.assertEqual(r.repack_size(9), 2)
        self.assertEqual(r.repack_size(17), 3)
        self.assertEqual(r.repackable_size(1), 8)
        a = BinData.from_spaced_hex(8, '12 34 56')
        b = r.repack(a, 1, 12)
        c = BinData.from_spaced_hex(1, ' '.join(format(0x345, '012b')))
        self.assertEqual(b, c)

    def test_split_60to20_little(self):
        r = Repacker(Endian.LITTLE, 60, 20)
        self.assertEqual(r.repack_unit, 60)
        self.assertEqual(r.repack_size(1), 1)
        self.assertEqual(r.repack_size(2), 1)
        self.assertEqual(r.repack_size(3), 1)
        self.assertEqual(r.repack_size(4), 2)
        self.assertEqual(r.repackable_size(1), 3)
        a = BinData(60, [0xfedcba987654321])
        b = r.repack(a)
        self.assertEqual(b, BinData.from_spaced_hex(20, '54321 a9876 fedcb'))

    def test_split_60to20_big(self):
        r = Repacker(Endian.BIG, 60, 20)
        self.assertEqual(r.repack_unit, 60)
        self.assertEqual(r.repack_size(1), 1)
        self.assertEqual(r.repack_size(2), 1)
        self.assertEqual(r.repack_size(3), 1)
        self.assertEqual(r.repack_size(4), 2)
        self.assertEqual(r.repackable_size(1), 3)
        a = BinData(60, [0xfedcba987654321])
        b = r.repack(a)
        self.assertEqual(b, BinData.from_spaced_hex(20, 'fedcb a9876 54321'))

    def test_split_16to8_little(self):
        r = Repacker(Endian.LITTLE, 16, 8)
        self.assertEqual(r.repack_unit, 16)
        self.assertEqual(r.repack_size(3), 2)
        self.assertEqual(r.repackable_size(3), 6)
        a = BinData(16, [0x1234, 0x5678, 0x9abc])
        b = r.repack(a, 1, 3)
        self.assertEqual(b, BinData.from_spaced_hex(8, '78 56 bc'))

    def test_split_16to8_big(self):
        r = Repacker(Endian.BIG, 16, 8)
        self.assertEqual(r.repack_unit, 16)
        self.assertEqual(r.repack_size(3), 2)
        self.assertEqual(r.repackable_size(3), 6)
        a = BinData(16, [0x1234, 0x5678, 0x9abc])
        b = r.repack(a, 1, 3)
        self.assertEqual(b, BinData.from_spaced_hex(8, '56 78 9a'))

    def test_padded_8to23_left_little(self):
        r = Repacker(Endian.LITTLE, 8, 23, high_pad=9)
        self.assertEqual(r.repack_unit, 32)
        self.assertEqual(r.repack_size(2), 8)
        self.assertEqual(r.repackable_size(7), 1)
        self.assertEqual(r.repackable_size(8), 2)
        a = BinData.from_spaced_hex(8, '11 22 33 44 55 66 77 88 99 aa')
        b = r.repack(a, 1, 2)
        self.assertEqual(b, BinData.from_spaced_hex(23, '443322 087766'))

    def test_padded_8to23_right_little(self):
        r = Repacker(Endian.LITTLE, 8, 23, low_pad=9)
        self.assertEqual(r.repack_unit, 32)
        self.assertEqual(r.repack_size(2), 8)
        self.assertEqual(r.repackable_size(7), 1)
        self.assertEqual(r.repackable_size(8), 2)
        a = BinData.from_spaced_hex(8, '11 22 33 44 55 66 77 88 99 aa')
        b = r.repack(a, 1, 2)
        self.assertEqual(b, BinData.from_spaced_hex(23, '2aa219 4cc43b'))

    def test_padded_8to23_mixed_little(self):
        r = Repacker(Endian.LITTLE, 8, 23, low_pad=8, high_pad=1)
        self.assertEqual(r.repack_unit, 32)
        self.assertEqual(r.repack_size(2), 8)
        self.assertEqual(r.repackable_size(7), 1)
        self.assertEqual(r.repackable_size(8), 2)
        a = BinData.from_spaced_hex(8, '11 22 33 44 55 66 77 88 99 aa')
        b = r.repack(a, 1, 2)
        self.assertEqual(b, BinData.from_spaced_hex(23, '554433 198877'))

    def test_padded_8to23_left_big(self):
        r = Repacker(Endian.BIG, 8, 23, high_pad=9)
        self.assertEqual(r.repack_unit, 32)
        self.assertEqual(r.repack_size(2), 8)
        self.assertEqual(r.repackable_size(7), 1)
        self.assertEqual(r.repackable_size(8), 2)
        a = BinData.from_spaced_hex(8, '11 22 33 44 55 66 77 88 99 aa')
        b = r.repack(a, 1, 2)
        self.assertEqual(b, BinData.from_spaced_hex(23, '334455 778899'))

    def test_padded_8to23_right_big(self):
        r = Repacker(Endian.BIG, 8, 23, low_pad=9)
        self.assertEqual(r.repack_unit, 32)
        self.assertEqual(r.repack_size(2), 8)
        self.assertEqual(r.repackable_size(7), 1)
        self.assertEqual(r.repackable_size(8), 2)
        a = BinData.from_spaced_hex(8, '11 22 33 44 55 66 77 88 99 aa')
        b = r.repack(a, 1, 2)
        self.assertEqual(b, BinData.from_spaced_hex(23, '1119a2 333bc4'))

    def test_padded_8to23_mixed_big(self):
        r = Repacker(Endian.BIG, 8, 23, low_pad=8, high_pad=1)
        self.assertEqual(r.repack_unit, 32)
        self.assertEqual(r.repack_size(2), 8)
        self.assertEqual(r.repackable_size(7), 1)
        self.assertEqual(r.repackable_size(8), 2)
        a = BinData.from_spaced_hex(8, '11 22 33 44 55 66 77 88 99 aa')
        b = r.repack(a, 1, 2)
        self.assertEqual(b, BinData.from_spaced_hex(23, '223344 667788'))
