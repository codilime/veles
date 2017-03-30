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

import binascii
import random

from veles.compatibility import pep487
from veles.compatibility.int_bytes import int_to_bytes


class NodeID(pep487.NewObject):
    WIDTH = 24
    _NULL_VAL = b'\x00'*WIDTH
    _ROOT_VAL = b'\xff'*WIDTH
    _rand = random.SystemRandom()

    def __init__(self, value=None):
        if value is None:
            value = int_to_bytes(
                self._rand.getrandbits(192), self.WIDTH, 'little')
        if isinstance(value, bytearray):
            value = bytes(value)
        if not isinstance(value, bytes):
            raise TypeError('wrong type provided')
        if len(value) != self.WIDTH or value == self._NULL_VAL:
            raise ValueError('value is not valid id')
        self._bytes = value

    @staticmethod
    def from_hex(value):
        return NodeID(binascii.a2b_hex(value))

    @property
    def bytes(self):
        return self._bytes

    def __str__(self):
        if self == self.root_id:
            return 'root'
        return binascii.b2a_hex(self.bytes).decode('ascii')

    def __repr__(self):
        if self == self.root_id:
            return 'NodeID.root_id'
        return 'NodeID.from_hex("{}")'.format(str(self))

    def __eq__(self, other):
        if isinstance(other, NodeID):
            return self.bytes == other.bytes
        return False

    def __hash__(self):
        return hash(self.bytes)


NodeID.root_id = NodeID(NodeID._ROOT_VAL)
