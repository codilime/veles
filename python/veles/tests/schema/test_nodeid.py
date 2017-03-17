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

from veles.schema.nodeid import NodeID


class TestNodeID(unittest.TestCase):
    def test_restricted(self):
        with self.assertRaises(ValueError):
            NodeID(b'\x00'*NodeID.WIDTH)
        with self.assertRaises(ValueError):
            NodeID.from_hex('00'*NodeID.WIDTH)

    def test_invalid(self):
        with self.assertRaises(TypeError):
            NodeID(123456)
        with self.assertRaises(TypeError):
            NodeID(u'zlew')
        with self.assertRaises(ValueError):
            NodeID(b'\x11'*(NodeID.WIDTH-1))
        with self.assertRaises(ValueError):
            NodeID(b'\x11'*(NodeID.WIDTH+1))
        with self.assertRaises(TypeError):
            NodeID.from_hex(123456)
        with self.assertRaises(ValueError):
            NodeID.from_hex('11'*(NodeID.WIDTH-1))
        with self.assertRaises(ValueError):
            NodeID.from_hex('11'*(NodeID.WIDTH+1))

    def test_valid(self):
        obj = NodeID()
        obj2 = NodeID()
        self.assertNotEqual(obj, obj2)
        byte = b'\x11\x12'*(NodeID.WIDTH//2)
        obj = NodeID(byte)
        self.assertEqual(byte, obj.bytes)
        obj = NodeID.root_id
        self.assertEqual(b'\xff'*NodeID.WIDTH, obj.bytes)

    def test_from_hex(self):
        obj = NodeID.from_hex('1112'*(NodeID.WIDTH//2))
        self.assertEqual(b'\x11\x12'*(NodeID.WIDTH//2), obj.bytes)

    def test_hashing(self):
        byte = b'\x11\x12'*(NodeID.WIDTH//2)
        byte2 = b'\x12\x11'*(NodeID.WIDTH//2)
        obj = NodeID(byte)
        obj2 = NodeID(byte)
        obj3 = NodeID(byte2)
        s = {obj}
        self.assertIn(obj2, s)
        self.assertNotIn(obj3, s)
        self.assertNotEqual(obj, byte)
        self.assertNotEqual(obj, byte2)
