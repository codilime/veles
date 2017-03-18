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

import six

from veles.compatibility import pep487
from veles.proto.exceptions import SchemaError
from veles.data import bindata
from . import nodeid


class Field(pep487.NewObject):
    def __init__(self, optional=False, default=None):
        self.name = None
        if not isinstance(optional, bool):
            raise TypeError('optional must be a bool')
        self.optional = optional
        self.default = default
        if default is not None:
            self.validate(default)

    def __get__(self, instance, owner=None):
        if instance is None:
            return self
        return instance.__dict__[self.name]

    def __set__(self, instance, value):
        self.validate(value)
        instance.__dict__[self.name] = value

    def __set_name__(self, owner, name):
        self.name = name

    def validate(self, value):
        if value is None:
            if not self.optional:
                raise SchemaError(
                    'Attribute {} is not optional and can\'t be None.'.format(
                        self.name))
        else:
            self._validate(value)

    def _validate(self, value):
        if not isinstance(value, self.value_type):
            raise SchemaError('Attribute {} has to be {}.'.format(
                self.name, self.value_type.__name__))

    def load(self, value):
        if value is None:
            self.validate(None)
            return None
        else:
            return self._load(value)

    def _load(self, value):
        self.validate(value)
        return value

    def dump(self, value):
        if value is None:
            return None
        return self._dump(value)

    def _dump(self, value):
        return value


class Any(Field):
    value_type = object


class Integer(Field):
    def __init__(self, optional=False, default=None,
                 minimum=None, maximum=None):
        if not isinstance(minimum, six.integer_types) and minimum is not None:
            raise TypeError('minimum must be an int')
        if not isinstance(maximum, six.integer_types) and maximum is not None:
            raise TypeError('maximum must be an int or None')
        if minimum is not None and maximum is not None and minimum > maximum:
            raise ValueError('minimum must be less than maximum')
        self.minimum = minimum
        self.maximum = maximum
        super(Integer, self).__init__(optional, default)

    def _validate(self, value):
        if not isinstance(value, six.integer_types) or isinstance(value, bool):
            raise SchemaError('Attribute {} has to be int type.'.format(
                self.name))
        if self.minimum is not None and value < self.minimum:
            raise SchemaError('Attribute {} minimum value is {}.'.format(
                self.name, self.minimum))
        if self.maximum is not None and value > self.maximum:
            raise SchemaError('Attribute {} maximum value is {}.'.format(
                self.name, self.maximum))


class UnsignedInteger(Integer):
    def __init__(self, optional=False, default=None,
                 minimum=0, maximum=None):
        if not isinstance(minimum, six.integer_types):
            raise TypeError('minimum must be an int')
        if minimum < 0:
            raise ValueError('UnsignedInteger minimum must not be negative')
        super(UnsignedInteger, self).__init__(
            optional, default, minimum, maximum)


INT64_MIN = -2**63
INT64_MAX = 2**63-1
UINT64_MAX = 2**64-1


class SmallInteger(Integer):
    def __init__(self, optional=False, default=None,
                 minimum=INT64_MIN, maximum=INT64_MAX):
        if not isinstance(minimum, six.integer_types):
            raise TypeError('minimum must be an int')
        if not isinstance(maximum, six.integer_types):
            raise TypeError('maximum must be an int')
        if minimum < INT64_MIN:
            raise ValueError('SmallInteger minimum too small')
        if maximum > INT64_MAX:
            raise ValueError('SmallInteger maximum too large')
        super(SmallInteger, self).__init__(
            optional, default, minimum, maximum)


class SmallUnsignedInteger(Integer):
    def __init__(self, optional=False, default=None,
                 minimum=0, maximum=UINT64_MAX):
        if not isinstance(minimum, six.integer_types):
            raise TypeError('minimum must be an int')
        if not isinstance(maximum, six.integer_types):
            raise TypeError('maximum must be an int')
        if minimum < 0:
            raise ValueError('SmallUnsignedInteger minimum too small')
        if maximum > UINT64_MAX:
            raise ValueError('SmallUnsignedInteger maximum too large')
        super(SmallUnsignedInteger, self).__init__(
            optional, default, minimum, maximum)


class Boolean(Field):
    value_type = bool


class Float(Field):
    value_type = float


class String(Field):
    value_type = six.text_type


class Binary(Field):
    value_type = bytes


class NodeID(Field):
    value_type = nodeid.NodeID


class BinData(Field):
    value_type = bindata.BinData


class List(Field):
    def __init__(self, element, optional=False, default=[]):
        super(List, self).__init__(optional, default)
        if not isinstance(element, Field):
            raise TypeError("List element must be a Field.")
        self.element = element

    def __set_name__(self, owner, name):
        super(List, self).__set_name__(owner, name)
        self.element.__set_name__(owner, name + '.element')

    def _validate(self, value):
        if not isinstance(value, list):
            raise SchemaError(
                'Attribute {} has to be a list'.format(self.name))
        for val in value:
            self.element.validate(val)

    def _load(self, value):
        if not isinstance(value, list):
            raise SchemaError(
                'Attribute {} has to be a list'.format(self.name))
        return [
            self.element.load(val)
            for val in value
        ]

    def _dump(self, value):
        return [
            self.element.dump(val)
            for val in value
        ]


class Set(Field):
    def __init__(self, element, optional=False, default=set()):
        super(Set, self).__init__(optional, default)
        if not isinstance(element, Field):
            raise TypeError("Set element must be a Field.")
        self.element = element

    def __set_name__(self, owner, name):
        super(Set, self).__set_name__(owner, name)
        self.element.__set_name__(owner, name + '.element')

    def _validate(self, value):
        if not isinstance(value, set):
            raise SchemaError(
                'Attribute {} has to be a set'.format(self.name))
        for val in value:
            self.element.validate(val)

    def _load(self, value):
        if not isinstance(value, list):
            raise SchemaError(
                'Attribute {} has to be a msgpack list'.format(self.name))
        return {
            self.element.load(val)
            for val in value
        }

    def _dump(self, value):
        return [
            self.element.dump(val)
            for val in value
        ]


class Map(Field):
    def __init__(self, key, value, optional=False, default={}):
        super(Map, self).__init__(optional, default)
        self.key = key
        self.value = value

    def __set_name__(self, owner, name):
        super(Map, self).__set_name__(owner, name)
        self.key.__set_name__(owner, name + '.key')
        self.value.__set_name__(owner, name + '.value')

    def _validate(self, value):
        if not isinstance(value, dict):
            raise SchemaError(
                'Attribute {} has to be a dict'.format(self.name))
        for k, v in value.items():
            self.key.validate(k)
            self.value.validate(v)

    def _load(self, value):
        if not isinstance(value, dict):
            raise SchemaError(
                'Attribute {} has to be a dict'.format(self.name))
        return {
            self.key.load(k): self.value.load(v)
            for k, v in value.items()
        }

    def _dump(self, value):
        return {
            self.key.dump(k): self.value.dump(v)
            for k, v in value.items()
        }


class Object(Field):
    def __init__(self, value_type, optional=False, default=None):
        super(Object, self).__init__(optional, default)
        self.value_type = value_type

    def _load(self, value):
        if value is None:
            self.validate(None)
            return None
        return self.value_type.load(value)

    def _dump(self, value):
        if value is None:
            return None
        return value.dump()
