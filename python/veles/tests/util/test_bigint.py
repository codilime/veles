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

from veles.util.bigint import bigint_encode, bigint_decode


CASES = [
    0,
    1,
    0xfe,
    0xff,
    0x100,
    0x101,
    0x1234,
    0xffff,
    0x10000,
    -1,
    -0xfe,
    -0xff,
    -0x100,
    -0x101,
    -0xffff,
    -0x10000,
    0x123456789abcdef1234566,
    0x123456789abcdef1234567,
    0x123456789abcdef1234568,
    0x123456789abcdef1234467,
    0x123456789abcdef1234667,
    (1 << 0x800) - 1,
    1 << 0x800,
    -((1 << 0x800) - 1),
    -(1 << 0x800),
]


class TestBigint(unittest.TestCase):
    def test_encode(self):
        self.assertEqual(bigint_encode(0), b'\x80\x00\x00\x00')
        self.assertEqual(bigint_encode(1), b'\x80\x00\x00\x01\x01')
        self.assertEqual(bigint_encode(0xff), b'\x80\x00\x00\x01\xff')
        self.assertEqual(bigint_encode(0x100), b'\x80\x00\x00\x02\x01\x00')
        self.assertEqual(bigint_encode(-1), b'\x7f\xff\xff\xff\xfe')
        self.assertEqual(bigint_encode(-0xff), b'\x7f\xff\xff\xff\x00')
        self.assertEqual(bigint_encode(-0x100), b'\x7f\xff\xff\xfe\xfe\xff')

    def test_roundtrip(self):
        for x in CASES:
            self.assertEqual(x, bigint_decode(bigint_encode(x)))

    def test_order(self):
        for x in CASES:
            for y in CASES:
                ex = bigint_encode(x)
                ey = bigint_encode(y)
                self.assertEqual(x < y, ex < ey)
