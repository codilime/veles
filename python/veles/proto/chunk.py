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

from __future__ import unicode_literals

from enum import Enum

from veles.schema import model, fields
from veles.data.repack import Repacker


class FieldSignMode(Enum):
    unsigned = 'unsigned'
    signed = 'signed'


class FieldFloatMode(Enum):
    ieee754_single = 'ieee754_single'
    ieee754_double = 'ieee754_double'


class FieldStringMode(Enum):
    raw = 'raw'
    zero_padded = 'zero_padded'
    zero_terminated = 'zero_terminated'


class FieldStringEncoding(Enum):
    raw = 'raw'
    utf8 = 'utf8'
    utf16 = 'utf16'


class FieldType(model.PolymorphicModel):
    pass


class FieldTypeFixed(FieldType):
    shift = fields.SmallInteger()
    sign_mode = fields.Enum(FieldSignMode)


class FieldTypeFloat(FieldType):
    mode = fields.Enum(FieldFloatMode)
    complex = fields.Boolean()


class FieldTypeString(FieldType):
    mode = fields.Enum(FieldStringMode)
    encoding = fields.Enum(FieldStringEncoding)


class ChunkDataItem(model.PolymorphicModel):
    pass


class ChunkDataItemSubchunk(ChunkDataItem):
    name = fields.String()
    pos_start = fields.Integer()
    pos_end = fields.Integer()
    ref = fields.NodeID()


class ChunkDataItemSubblob(ChunkDataItem):
    name = fields.String()
    ref = fields.NodeID()


class ChunkDataItemField(ChunkDataItem):
    pos_start = fields.Integer()
    pos_end = fields.Integer()
    name = fields.String()
    repack = fields.Object(Repacker)
    num_elements = fields.SmallUnsignedInteger()
    type = fields.Object(FieldType)
    raw_value = fields.BinData()


class ChunkDataItemComputed(ChunkDataItem):
    name = fields.String()
    type = fields.Object(FieldType)
    raw_value = fields.BinData()


class ChunkDataItemPad(ChunkDataItem):
    pos_start = fields.Integer()
    pos_end = fields.Integer()
