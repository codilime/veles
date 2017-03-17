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

import binascii
import random

from veles.compatibility import pep487
from veles.compatibility.int_bytes import int_to_bytes


class NodeID(pep487.NewObject):
    _NULL_VAL = b'\x00'*24
    _ROOT_VAL = b'\xff'*24
    _rand = random.SystemRandom()
    _root = None

    def __init__(self, value=None):
        if value is None:
            value = int_to_bytes(self._rand.getrandbits(192), 24, 'little')
        if isinstance(value, bytearray):
            value = bytes(value)
        if not isinstance(value, bytes):
            raise TypeError('wrong type provided')
        if len(value) != 24 or value in [self._NULL_VAL, self._ROOT_VAL]:
            raise ValueError('value is not valid id')
        self._bytes = value

    @staticmethod
    def from_hex(value):
        return NodeID(binascii.a2b_hex(value))

    @classmethod
    def root_id(cls):
        if cls._root:
            return cls._root
        # not the most elegant way since we prohibit normal creation of
        # object with this value of id
        cls._root = NodeID()
        cls._root._bytes = cls._ROOT_VAL
        return cls._root

    @property
    def bytes(self):
        return self._bytes

    def __str__(self):
        return binascii.b2a_hex(self.bytes).decode('ascii')

    def __repr__(self):
        return 'NodeID("{}")'.format(str(self))

    def __eq__(self, other):
        if isinstance(other, NodeID):
            return self.bytes == other.bytes
        return False

    def __hash__(self):
        return hash(self.bytes)
