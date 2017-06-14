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
from datetime import datetime, tzinfo, timedelta
if six.PY3:
    from datetime import timezone
else:
    from time import mktime

from veles.data.bindata import BinData
from veles.compatibility import pep487
from veles.schema import nodeid
from veles.compatibility.int_bytes import int_to_bytes, int_from_bytes
from veles.util.bigint import bigint_encode, bigint_decode


EXT_NODE_ID = 0
EXT_BINDATA = 1
EXT_BIGINT = 2
EXT_DATETIME = 3


class UTC(tzinfo):
    def utcoffset(self, dt):
        return timedelta(0)
    def tzname(self, dt):
        return "UTC"
    def dst(self, dt):
        return timedelta(0)


class MsgpackWrapper(pep487.NewObject):
    def __init__(self):
        self.packer = msgpack.Packer(
            use_bin_type=True, default=MsgpackWrapper.pack_obj)
        self.unpacker = msgpack.Unpacker(
            encoding='utf-8', ext_hook=MsgpackWrapper.load_obj)

    @classmethod
    def pack_obj(cls, obj):
        if isinstance(obj, nodeid.NodeID):
            return msgpack.ExtType(EXT_NODE_ID, obj.bytes)
        elif isinstance(obj, BinData):
            width = int_to_bytes(obj.width, 4, 'little')
            return msgpack.ExtType(EXT_BINDATA, width + obj.raw_data)
        elif isinstance(obj, six.integer_types):
            return msgpack.ExtType(EXT_BIGINT, bigint_encode(obj))
        elif isinstance(obj, datetime):
            if six.PY2:
                timestamp = int_to_bytes(int(mktime(obj.timetuple()) * 1000.), 8, 'little')
            else:
                timestamp = int_to_bytes(int(obj.timestamp() * 1000.), 8, 'little')
            return msgpack.ExtType(EXT_DATETIME, timestamp)
        raise TypeError('Object of unknown type {}'.format(obj))

    @classmethod
    def load_obj(cls, code, data):
        if code == EXT_NODE_ID:
            return nodeid.NodeID(data)
        elif code == EXT_BINDATA:
            width = int_from_bytes(data[:4], 'little')
            return BinData.from_raw_data(width, data[4:])
        elif code == EXT_BIGINT:
            return bigint_decode(data)
        elif code == EXT_DATETIME:
            timestamp = float(int_from_bytes(data[:8], 'little')) / 1000.
            if six.PY2:
                return datetime.fromtimestamp(timestamp, tz=UTC)
            else:
                return datetime.fromtimestamp(timestamp, tz=timezone.utc)
        return msgpack.ExtType(code, data)
