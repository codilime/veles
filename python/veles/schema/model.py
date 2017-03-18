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

from copy import copy

import six

from veles.compatibility import pep487
from veles.proto.exceptions import SchemaError
from . import fields


class Model(pep487.NewObject):
    fields = []

    def __init__(self, **kwargs):
        for field in self.fields:
            if field.name in kwargs:
                setattr(self, field.name, kwargs.pop(field.name))
            else:
                setattr(self, field.name, copy(field.default))
        if kwargs:
            raise TypeError('got an unexpected keyword argument {}'.format(
                kwargs.popitem()[0]))

    def __init_subclass__(cls, **kwargs):
        super(Model, cls).__init_subclass__(**kwargs)

        cls.fields = super(cls, cls).fields[:]
        for attr in cls.__dict__.values():
            if isinstance(attr, fields.Field):
                cls.fields.append(attr)

    def dump(self):
        val = {}
        for field in self.fields:
            val[six.text_type(field.name)] = field.dump(field.__get__(self))
        return val

    @classmethod
    def load(cls, val):
        if not isinstance(val, dict):
            raise SchemaError('a serialized model must be a dict')
        val = dict(val)
        args = {}
        for field in cls.fields:
            if field.name in val:
                args[field.name] = field.load(val.pop(field.name))
        if val:
            raise SchemaError('got unexpected key {} in model {}'.format(
                val.popitem()[0], cls.__name__))
        return cls(**args)

    def __str__(self):
        return '{}({})'.format(type(self).__name__, ', '.join(
            '{}={!r}'.format(field.name, field.__get__(self))
            for field in self.fields
        ))

    def __eq__(self, other):
        return type(self) == type(other) and self.__dict__ == other.__dict__

    __repr__ = __str__


class PolymorphicModel(Model):
    # string name representing the type, base abstract classes should leave
    # this as None, while its subclasses should set it to value unique amongst
    # other subclasses
    object_type = None

    def __init__(self, **kwargs):
        if self.object_type is None:
            raise TypeError(
                'Can\'t instantiate class {}'.format(type(self)))
        super(PolymorphicModel, self).__init__(**kwargs)

    def __init_subclass__(cls, **kwargs):
        super(PolymorphicModel, cls).__init_subclass__(**kwargs)

        if not hasattr(cls, 'object_types'):
            cls.object_types = {}
        elif cls.object_type is not None:
            if cls.object_type in cls.object_types:
                raise TypeError(
                    'object_type {} already used'.format(cls.object_type))
            cls.object_types[cls.object_type] = cls

    def dump(self):
        res = super(PolymorphicModel, self).dump()
        res[u'object_type'] = self.object_type
        return res

    @classmethod
    def load(cls, val):
        if not isinstance(val, dict):
            raise SchemaError('A serialized model must be a dict')
        val = dict(val)
        if 'object_type' not in val:
            raise SchemaError('A polymorphic model has no type')
        ot = val.pop('object_type')
        if ot not in cls.object_types:
            raise SchemaError('Unknown object type')
        rcls = cls.object_types[ot]
        if not issubclass(rcls, cls):
            raise SchemaError('Object type not a subclass of {}'.format(
                cls.__name__))
        return super(PolymorphicModel, rcls).load(val)
