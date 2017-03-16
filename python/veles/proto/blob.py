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

from veles.schema import fields
from veles.schema.model import Model
from veles.schema.plugin import MethodSignature, QuerySignature


class BlobRange(Model):
    start = fields.SmallUnsignedInteger()
    end = fields.SmallUnsignedInteger()


class BlobGetMapResult(Model):
    map = fields.List(fields.Object(BlobRange))
    width = fields.SmallUnsignedInteger()


query_blob_get_map = QuerySignature(
    'get_map',
    fields.Empty(),
    fields.Object(BlobGetMapResult),
)


class BlobGetDataResult(Model):
    data = fields.BinData()
    mask = fields.BinData(optional=True)


query_blob_get_data = QuerySignature(
    'get_data',
    fields.Object(BlobRange),
    fields.Object(BlobGetDataResult),
)


class BlobSetDataParams(Model):
    start = fields.SmallUnsignedInteger()
    data = fields.BinData()


method_blob_set_data = MethodSignature(
    'set_data',
    fields.Object(BlobSetDataParams),
    fields.Empty(),
)


class BlobTruncateDataParams(Model):
    size = fields.SmallUnsignedInteger()


method_blob_truncate_data = MethodSignature(
    'truncate_data',
    fields.Object(BlobTruncateDataParams),
    fields.Empty(),
)
