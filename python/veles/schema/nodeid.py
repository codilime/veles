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

from veles.compatibility import pep487


class NodeID(pep487.NewObject):
    NULL_VAL = b'\00'*24

    def __init__(self, value=None):
        if value is None:
            value = self.NULL_VAL
        if isinstance(value, str):
            value = binascii.a2b_hex(value)
        if not isinstance(value, bytes) or len(value) != 24:
            raise ValueError('value is not valid id')
        self._bytes = value

    @property
    def bytes(self):
        return self._bytes

    def to_bytes(self):
        return self.bytes

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

    def __bool__(self):
        return self.bytes != self.NULL_VAL
