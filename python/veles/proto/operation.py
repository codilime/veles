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
from veles.schema.nodeid import NodeID


class Operation(model.PolymorphicModel):
    node = fields.NodeID()


class OperationCreate(Operation):
    object_type = 'create'

    parent = fields.NodeID(default=NodeID.root_id)
    pos_start = fields.Integer(optional=True)
    pos_end = fields.Integer(optional=True)
    tags = fields.Set(fields.String())
    attr = fields.Map(fields.String(), fields.Any())
    data = fields.Map(fields.String(), fields.Any())
    bindata = fields.Map(fields.String(), fields.Binary())


class OperationDelete(Operation):
    object_type = 'delete'


class OperationSetParent(Operation):
    object_type = 'set_parent'

    parent = fields.NodeID(default=NodeID.root_id)


class OperationSetPos(Operation):
    object_type = 'set_pos'

    pos_start = fields.Integer(optional=True)
    pos_end = fields.Integer(optional=True)


class OperationAddTag(Operation):
    object_type = 'add_tag'

    tag = fields.String()


class OperationDelTag(Operation):
    object_type = 'del_tag'

    tag = fields.String()


class OperationSetAttr(Operation):
    object_type = 'set_attr'

    key = fields.String()
    data = fields.Any(optional=True)


class OperationSetData(Operation):
    object_type = 'set_data'

    key = fields.String()
    data = fields.Any(optional=True)


class OperationSetBinData(Operation):
    object_type = 'set_bindata'

    key = fields.String()
    start = fields.SmallUnsignedInteger(default=0)
    data = fields.Binary()
    truncate = fields.Boolean(default=False)
