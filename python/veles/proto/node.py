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

from veles.messages import fields
from veles.schema import nodeid, model


class Node(model.Model):
    id = fields.Extension(obj_type=nodeid.NodeID)
    parent = fields.Extension(obj_type=nodeid.NodeID, optional=True)
    pos_start = fields.Integer(optional=True)
    pos_end = fields.Integer(optional=True)
    tags = fields.Array(elements_types=[fields.String()],
                        local_type=set, optional=True)
    attr = fields.Map(keys_types=[fields.String()], optional=True)
    data = fields.Array(elements_types=[fields.String()], optional=True)
    bindata = fields.Map(keys_types=[fields.String()],
                         values_types=[fields.Integer()], optional=True)
