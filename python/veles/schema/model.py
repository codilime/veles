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

    @classmethod
    def generate_cpp_code(cls, bases=None, extra_pack=None, extra_code=''):
        # TODO nil handling
        code = '\\\n'
        inherit = ''
        if bases:
            inherit = ' : {}'.format(
                ', '.join(['public {}'.format(base[0]) for base in bases]))
        code += 'class {}{} {{\\\n'.format(cls.__name__, inherit)
        code += ' public:\\\n'

        # TODO some encapsulation ?
        for field in cls.fields:
            code += '  {} {};\\\n'.format(field.cpp_type(), field.name)

        # TODO set basic types to some predictable starting value
        if bases:
            bases_cons = ', '.join(['{}({})'.format(base[0], base[1])
                                    for base in bases])
            code += '  {}() : {} {{}}\\\n'.format(
                cls.__name__, bases_cons)
            if cls.fields:
                code += '  {}({}) : {}, {} {{}}\\\n'.format(
                    cls.__name__,
                    ', '.join(
                        'const {}& {}'.format(field.cpp_type(), field.name)
                        for field in cls.fields),
                    bases_cons,
                    ', '.join('{}({})'.format(field.name, field.name)
                              for field in cls.fields))
        else:
            code += '  {}() {{}}\\\n'.format(cls.__name__)
            if cls.fields:
                code += '  {}({}) : {} {{}}\\\n'.format(
                    cls.__name__,
                    ', '.join(
                        'const {}& {}'.format(field.cpp_type(), field.name)
                        for field in cls.fields),
                    ', '.join('{}({})'.format(field.name, field.name)
                              for field in cls.fields))

        code += extra_code

        if extra_pack is None:
            extra_pack = []
        code += ' public:\\\n'
        code += '  MSGPACK_DEFINE_MAP(' + ', '.join(
            [field.name for field in cls.fields] + extra_pack) + ');\\\n'

        code += '};\\\n'

        return code

    @classmethod
    def cpp_type(cls):
        return cls.__name__

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

    @classmethod
    def generate_base_code(cls):
        code = '''\\
class {0} {{\\
 public:\\
  template<typename T> static {0} * createInstance() {{ return new T; }}\\
  static std::map<std::string, {0}*(*)()>& object_types() {{\\
    static std::map<std::string, {0}*(*)()> object_types;\\
    return object_types;\\
  }}\\
  static void initObjectTypes() {{\\
{1}  }}\\
  std::string object_type;\\
  {0}(std::string object_type) : object_type(object_type) {{}}\\
  virtual ~{0}() {{}}\\
  virtual void msgpack_unpack(msgpack::object const& o) = 0;\\
  virtual void serialize(msgpack::packer<msgpack::sbuffer>& pac) = 0;\\
}};\\
'''.format(cls.cpp_type(),
           ''.join(
               ['    object_types()["{}"] = &createInstance<{}>;\\\n'.format(
                   obj_type, obj_class.cpp_type())
                for obj_type, obj_class in cls.object_types.items()]))
        return code

    @classmethod
    def generate_cpp_code(cls, bases=None, extra_pack=None, extra_code=''):
        # TODO think how serialize should behave exactly - we can't do it like
        # msgpack_unpack since it is template function
        serialize = '''\\
  void serialize(msgpack::packer<msgpack::sbuffer>& pac) {\\
    pac.pack(*this);\\
  }'''

        return super(PolymorphicModel, cls).generate_cpp_code(
            [(cls.__bases__[0].__name__,
              '"{}"'.format(cls.object_type))], ['object_type'], serialize)
