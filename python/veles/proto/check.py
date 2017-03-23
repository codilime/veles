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

from veles.schema import model, fields
from veles.proto.node import PosFilter


class Check(model.PolymorphicModel):
    pass


class CheckGone(Check):
    object_type = 'gone'

    node = fields.NodeID()


class CheckParent(Check):
    object_type = 'parent'

    node = fields.NodeID()
    parent = fields.NodeID()


class CheckPos(Check):
    object_type = 'pos'

    node = fields.NodeID()
    pos_start = fields.Integer(optional=True)
    pos_end = fields.Integer(optional=True)


class CheckTags(Check):
    object_type = 'tags'

    node = fields.NodeID()
    tags = fields.Set(fields.String())


class CheckTag(Check):
    object_type = 'tag'

    node = fields.NodeID()
    tag = fields.String()
    present = fields.Boolean()


class CheckAttr(Check):
    object_type = 'attr'

    node = fields.NodeID()
    key = fields.String()
    data = fields.Any(optional=True)


class CheckData(Check):
    object_type = 'data'

    node = fields.NodeID()
    key = fields.String()
    data = fields.Any(optional=True)


class CheckBinDataSize(Check):
    object_type = 'bindata_size'

    node = fields.NodeID()
    key = fields.String()
    size = fields.SmallUnsignedInteger()


class CheckBinData(Check):
    object_type = 'bindata'

    node = fields.NodeID()
    key = fields.String()
    start = fields.SmallUnsignedInteger()
    end = fields.SmallUnsignedInteger(optional=True)
    data = fields.Binary()


class CheckList(Check):
    object_type = 'list'

    parent = fields.NodeID()
    tags = fields.Set(fields.String())
    pos_filter = fields.Object(PosFilter)
    nodes = fields.Set(fields.NodeID())
