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
            NodeID(b'\x00'*24)
        with self.assertRaises(ValueError):
            NodeID(b'\xff'*24)
        with self.assertRaises(ValueError):
            NodeID.from_hex('00'*24)
        with self.assertRaises(ValueError):
            NodeID.from_hex('ff'*24)

    def test_invalid(self):
        with self.assertRaises(TypeError):
            NodeID(123456)
        with self.assertRaises(ValueError):
            NodeID(b'\x11'*23)
        with self.assertRaises(ValueError):
            NodeID(b'\x11'*25)
        with self.assertRaises(TypeError):
            NodeID.from_hex(123456)
        with self.assertRaises(ValueError):
            NodeID.from_hex('11'*23)
        with self.assertRaises(ValueError):
            NodeID.from_hex('11'*25)

    def test_valid(self):
        obj = NodeID()
        obj2 = NodeID()
        self.assertNotEqual(obj, obj2)
        byte = b'\x11'*24
        obj = NodeID(byte)
        self.assertEqual(byte, obj.bytes)
        obj = NodeID.root_id()
        self.assertEqual(b'\xff'*24, obj.bytes)

    def test_from_hex(self):
        obj = NodeID.from_hex('11'*24)
        self.assertEqual(b'\x11'*24, obj.bytes)
