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
        for klass in cls.__bases__ + (cls,):
            for attr_name, attr in klass.__dict__.items():
                try:
                    attr.add_to_class(cls)
                except AttributeError:
                    pass

    def to_dict(self):
        val = {}
        for field in self.fields:
            val[field.name] = field.__get__(self)
        return val

    @classmethod
    def from_dict(cls, val):
        """Override if your class overrides __init__"""
        return cls(**val)

    @classmethod
    def load(cls, val):
        return cls.from_dict(val)

    def dump(self, packer):
        return packer.pack(self.to_dict())

    def __str__(self):
        return '{}: {}'.format(self.__class__.__name__, self.to_dict())

    __repr__ = __str__


class PolymorphicModel(Model):
    # string name representing the type, base abstract classes should leave
    # this as None, while its subclasses should set it to value unique amongst
    # other subclasses
    object_type = None

    def __init__(self, **kwargs):
        assert self.object_type is not None, (
            'Can\'t instantiate class {}'.format(self.__class__.__name__))
        super(PolymorphicModel, self).__init__(**kwargs)

    def __init_subclass__(cls, **kwargs):
        if cls.object_type is None:
            cls.object_types = {}
            return

        super(PolymorphicModel, cls).__init_subclass__(**kwargs)

        assert cls.object_type not in cls.object_types, (
            'object_type {} already used'.format(cls.object_type))
        cls.object_types[cls.object_type] = cls

    def to_dict(self):
        val = super(PolymorphicModel, self).to_dict()
        val['object_type'] = self.object_type
        return val

    @classmethod
    def load(cls, val):
        if not isinstance(val, dict) or 'object_type' not in val:
            raise ValueError('Malformed object')
        if val['object_type'] not in cls.object_types:
            raise ValueError('Unknown object type')
        return cls.object_types[val.pop('object_type')].from_dict(val)

    def __str__(self):
        return '{}: {}'.format(self.object_type, self.to_dict())

    __repr__ = __str__
