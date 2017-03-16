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

import msgpack
import six

from veles.data.bindata import BinData
from veles.compatibility import pep487
from veles.schema import nodeid
from veles.compatibility.int_bytes import int_to_bytes, int_from_bytes


EXT_NODE_ID = 0
EXT_BINDATA = 1
EXT_BIGINT = 2
EXT_NEGBIGINT = 3


class MsgpackWrapper(pep487.NewObject):
    def __init__(self):
        self.packer = msgpack.Packer(
            use_bin_type=True, default=MsgpackWrapper.pack_obj)
        self.unpacker = msgpack.Unpacker(
            encoding='utf-8', ext_hook=MsgpackWrapper.load_obj)

    @classmethod
    def pack_obj(cls, obj):
        if isinstance(obj, (set, frozenset)):
            return list(obj)
        if isinstance(obj, nodeid.NodeID):
            return msgpack.ExtType(EXT_NODE_ID, obj.bytes)
        if isinstance(obj, BinData):
            width = int_to_bytes(obj.width, 4, 'little')
            return msgpack.ExtType(EXT_BINDATA, width + obj.raw_data)
        if isinstance(obj, six.integer_types):
            sz = (obj.bit_length() + 7) // 8
            if obj < 0:
                raw = int_to_bytes(-obj, sz, 'little')
                return msgpack.ExtType(EXT_NEGBIGINT, raw)
            else:
                raw = int_to_bytes(obj, sz, 'little')
                return msgpack.ExtType(EXT_BIGINT, raw)
        if callable(getattr(obj, "to_dict", None)):
            return obj.to_dict()
        raise TypeError('Object of unknown type {}'.format(obj))

    @classmethod
    def load_obj(cls, code, data):
        if code == EXT_NODE_ID:
            return nodeid.NodeID(data)
        elif code == EXT_BINDATA:
            width = int_from_bytes(data[:4], 'little')
            return BinData.from_raw_data(width, data[4:])
        elif code == EXT_BIGINT:
            return int_from_bytes(data, 'little')
        elif code == EXT_NEGBIGINT:
            return -int_from_bytes(data, 'little')
        return msgpack.ExtType(code, data)
