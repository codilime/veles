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

from veles.messages import fields
from veles.compatibility import pep487


class Model(pep487.NewObject):
    def __init__(self, **kwargs):
        if any(i not in [field.name for field in self.fields] for i in kwargs):
            raise TypeError('got an unexpected keyword argument')
        for field_name, field in kwargs.items():
            setattr(self, field_name, field)

    def __init_subclass__(cls, **kwargs):
        super(Model, cls).__init_subclass__(**kwargs)

        cls.fields = []
        for attr_name, attr in cls.__dict__.items():
            try:
                attr.add_to_class(cls)
            except AttributeError:
                pass

    def to_dict(self):
        val = {}
        for field in self.fields:
            val[field.name] = field.__get__(self)
        return val

    def __str__(self):
        return str(self.to_dict())

    __repr__ = __str__


class ObjectID(pep487.NewObject):
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
        return 'ObjectID("{}")'.format(str(self))

    def __eq__(self, other):
        if isinstance(other, ObjectID):
            return self.bytes == other.bytes
        return False

    def __hash__(self):
        return hash(self.bytes)

    def __bool__(self):
        return self.bytes != self.NULL_VAL


class Node(Model):
    id = fields.Extension(obj_type=ObjectID)
    parent = fields.Extension(obj_type=ObjectID, optional=True)
    pos_start = fields.Integer(optional=True)
    pos_end = fields.Integer(optional=True)
    tags = fields.Array(elements_types=[fields.String()],
                        local_type=set, optional=True)
    attr = fields.Map(keys_types=[fields.String()], optional=True)
    data = fields.Array(elements_types=[fields.String()], optional=True)
    bindata = fields.Map(keys_types=[fields.String()],
                         values_types=[fields.Integer()], optional=True)
