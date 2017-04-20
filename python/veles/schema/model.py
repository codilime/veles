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
import sys

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
        if hasattr(cls, '_order'):
            attrs = [getattr(cls, x) for x in cls._order]
        else:
            attrs = cls.__dict__.values()
        for attr in attrs:
            if isinstance(attr, fields.Field):
                cls.fields.append(attr)

        sys.modules[cls.__module__].__dict__.setdefault(
            '_models', []).append(cls)

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
    def generate_header_code(cls, bases=None):
        if not cls.fields:
            return ''
        code = '\n'
        inherit = ''
        if bases:
            inherit = ' : {}'.format(
                ', '.join(['public {}'.format(base[0]) for base in bases]))
        code += 'class {}{} {{\n'.format(cls.cpp_type()[0], inherit)

        fields_code = ''
        constructor_args = []
        setters = []
        builder_cons = []
        for field in cls.fields:
            arg_type = (
                '{}' if field.cpp_type()[1] else 'std::shared_ptr<{}>').format(
                field.cpp_type()[0])
            if field.optional:
                fields_code += '  std::pair<bool, {0}> {1};\n'.format(
                    arg_type, field.name)
                arg = 'const std::pair<bool, {}>& {}'.format(
                    arg_type, field.name)
                builder_cons.append('{}(false, {})'.format(
                    field.name, field.cpp_type()[2]))
            else:
                fields_code += '  {} {};\n'.format(
                    arg_type, field.name)
                arg = '{} {}'.format(arg_type, field.name)
                builder_cons.append('{}({})'.format(
                    field.name, field.cpp_type()[2]))
            constructor_args.append(arg)
            setters.append('''
    Builder& set_{0}({1});
'''.format(field.name, arg))
        code += ' public:\n'
        code += '''  class Builder {{
{0}
 public:
  Builder() : {3} {{}}
  {1}
    std::shared_ptr<{2}> build();
  }};
'''.format(fields_code, ''.join(setters),
           cls.cpp_type()[1], ', '.join(builder_cons))
        # TODO some encapsulation ?
        code += fields_code

        if bases:
            bases_cons = ', '.join(['{}({})'.format(base[0], base[1])
                                    for base in bases if base[1] is not None])
            code += '  {}({}) : {}, {} {{}}\n'.format(
                cls.cpp_type()[0],
                ', '.join(constructor_args),
                bases_cons,
                ', '.join('{0}({0})'.format(field.name)
                          for field in cls.fields))
        else:
            code += '  {}({}) : {} {{}}\n'.format(
                cls.cpp_type()[0],
                ', '.join(constructor_args),
                ', '.join('{0}({0})'.format(field.name)
                          for field in cls.fields))
        code += '''
  static std::shared_ptr<{0}> loadMessagePack(const msgpack::object& obj);
'''.format(cls.cpp_type()[1])
        code += '''
  std::shared_ptr<messages::MsgpackObject> serializeToMsgpackObject();
'''
        code += '};\n'

        return code

    @classmethod
    def generate_source_code(cls, extra_pack=None):
        if not cls.fields:
            return ''
        if extra_pack is None:
            extra_pack = []
        code = ''

        to_object = []

        for field in cls.fields:
            arg_type = (
                '{}' if field.cpp_type()[1] else 'std::shared_ptr<{}>').format(
                field.cpp_type()[0])
            if field.optional:
                to_object.append('''if (this->{0}.first) {{
                  msg["{0}"] = messages::toMsgpackObject(this->{0}.second);
                }} else {{
                  msg["{0}"] = std::make_shared<messages::MsgpackObject>();
                }}
                '''.format(field.name))
                arg = 'const std::pair<bool, {}>& {}'.format(
                    arg_type, field.name)
            else:
                pack_code = ''
                if not field.cpp_type()[1]:
                    pack_code += '''if (this->{0} == nullptr) {{
  throw proto::SchemaError("Nonoptional field {0} not set when packing");
}}
'''.format(field.name)
                pack_code += (
                    '''msg["{0}"] = messages::toMsgpackObject(this->{0});
'''.format(field.name))
                to_object.append(pack_code)
                arg = '{} {}'.format(arg_type, field.name)
            code += '''
                {3}::Builder& {3}::Builder::set_{0}({1}) {{
                  this->{0} = {2};
                  return *this;
                }}
            '''.format(field.name, arg, ('std::shared_ptr<{}>({{}})'.format(
                field.cpp_type()[0]) if not field.cpp_type()[1]
                and not field.optional else '{}').format(field.name),
                       cls.cpp_type()[0])

        code += '''
  std::shared_ptr<messages::MsgpackObject> {2}::serializeToMsgpackObject() {{
    std::map<std::string, std::shared_ptr<messages::MsgpackObject>> msg;
    {0}
    {1}
    return std::make_shared<messages::MsgpackObject>(msg);
  }}
'''.format(''.join(to_object), '\n'.join(
            ['msg["{0}"] = messages::toMsgpackObject(this->{0});'.format(
                extra) for extra in extra_pack]), cls.cpp_type()[0])
        code += '''
std::shared_ptr<{0}> {1}::loadMessagePack(const msgpack::object& obj) {{
  auto loc_obj = std::make_shared<messages::MsgpackObject>(obj);
  std::shared_ptr<{0}> out;
  fromMsgpackObject(loc_obj, out);
  return out;
}}
'''.format(cls.cpp_type()[1], cls.cpp_type()[0])
        code += '''std::shared_ptr<{1}> {0}::Builder::build() {{
  return std::make_shared<{1}>({2});
}}'''.format(cls.cpp_type()[0], cls.cpp_type()[1],
             ', '.join(field.name for field in cls.fields))
        return code

    @classmethod
    def generate_header_conv_code(cls):
        if not cls.fields:
            return ''
        code = ('void fromMsgpackObject(const std::shared_ptr<MsgpackObject>'
                'obj, std::shared_ptr<{0}>& out);\n'.format(
                    cls.cpp_type()[1]))
        code += ('std::shared_ptr<MsgpackObject> toMsgpackObject'
                 '(std::shared_ptr<{}> val);\n'.format(cls.cpp_type()[1]))
        return code

    @classmethod
    def generate_source_conv_code(cls):
        if not cls.fields:
            return ''
        from_object = []
        for field in cls.fields:
            arg_type = (
                '{}' if field.cpp_type()[1] else 'std::shared_ptr<{}>').format(
                field.cpp_type()[0])
            builder = '''auto it_{0} = obj->getMap()->find("{0}");
if (it_{0} != obj->getMap()->end() && \
it_{0}->second->type() != ObjectType::NIL) {{
'''.format(field.name)
            conv_func = '''{1} obj_{0};
  fromMsgpackObject(it_{0}->second, obj_{0});'''.format(field.name, arg_type)
            if field.optional:
                builder += '''{2}
  b.set_{0}(std::pair<bool, {1}>(true, obj_{0}));
}}
'''.format(field.name, arg_type, conv_func)
            else:
                builder += '''  {2}
  b.set_{0}(obj_{0});
}} else {{
  throw proto::SchemaError("Nonoptional field \
{0} not found when unpacking");
}}'''.format(field.name, arg_type, conv_func)
            from_object.append(builder)
        code = '''void fromMsgpackObject(const std::shared_ptr<MsgpackObject> \
obj, std::shared_ptr<{0}>& out) {{
            {0}::Builder b;
            {1}
            out = b.build();
          }}
        '''.format(cls.cpp_type()[1], '\n'.join(from_object))
        code += '''std::shared_ptr<MsgpackObject> \
toMsgpackObject(std::shared_ptr<{0}> val) {{
            return val->serializeToMsgpackObject();
          }}
        '''.format(cls.cpp_type()[1])
        return code

    @classmethod
    def cpp_type(cls):
        """returns a tuple containing class name and fully qualified name
        (in case we want to subclass generated model to add some functionality
        this should be fully qualified name of this subclass - in such
        case 3rd tuple element containing name of header file needs to be
        provided)"""
        return cls.__name__, '::'.join(
            cls.__module__.split('.')[:-1] + [cls.__name__])

    def __str__(self):
        return '{}({})'.format(type(self).__name__, ', '.join(
            '{}={!r}'.format(field.name, field.__get__(self))
            for field in self.fields
        ))

    def __eq__(self, other):
        return type(self) == type(other) and self.__dict__ == other.__dict__

    def __ne__(self, other):
        return not self.__eq__(other)

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
            if not isinstance(cls.object_type, six.text_type):
                raise TypeError('object_type needs to be str')
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
    def generate_base_header_code(cls):
        code = '''
class {0} {{
 public:
  virtual std::shared_ptr<messages::MsgpackObject> \
serializeToMsgpackObject() = 0;
  static void initObjectTypes() {{
{1}  }}
  template<typename T> static std::shared_ptr<{0}> \
  createInstance(const std::shared_ptr<messages::MsgpackObject> obj) {{
    std::shared_ptr<T> out;
    fromMsgpackObject(obj, out);
    return out;
  }}
  static std::map<std::string, std::shared_ptr<{0}>(*)\
(const std::shared_ptr<messages::MsgpackObject>)>& objectTypes() {{
    static std::map<std::string, std::shared_ptr<{0}>(*)\
(const std::shared_ptr<messages::MsgpackObject>)> object_types;
    return object_types;
  }}
  std::string object_type;
  {0}(std::string object_type) : object_type(object_type) {{}}
  virtual ~{0}() {{}}

  static std::shared_ptr<{0}> polymorphicLoad(const msgpack::object& obj);
}};

'''.format(cls.cpp_type()[0],
           ''.join([
               '    objectTypes()["{0}"] = &createInstance<{1}>;\n'.format(
                   obj_type, obj_class.cpp_type()[0])
               for obj_type, obj_class in cls.object_types.items()
               if obj_class.fields]))
        return code

    @classmethod
    def generate_base_source_code(cls):
        code = '''std::shared_ptr<{0}> {0}::polymorphicLoad(const msgpack::object& obj) {{
    auto loc_obj = std::make_shared<messages::MsgpackObject>(obj);
    std::shared_ptr<{0}> out;
    fromMsgpackObject(loc_obj, out);
    return out;
  }}
'''.format(cls.cpp_type()[0])
        return code

    @classmethod
    def generate_base_header_conv_code(cls):
        code = '''void fromMsgpackObject(const std::shared_ptr<MsgpackObject>\
 obj, std::shared_ptr<{0}>& out);
std::shared_ptr<MsgpackObject> toMsgpackObject(std::shared_ptr<{0}> val);
'''.format(cls.cpp_type()[1])
        return code

    @classmethod
    def generate_base_source_conv_code(cls):
        code = '''void fromMsgpackObject(const std::shared_ptr<MsgpackObject>\
 obj, std::shared_ptr<{0}>& out) {{
  auto& types = {0}::objectTypes();
  if (types.size() == 0) {{
    {0}::initObjectTypes();
  }}
  auto obj_type = *(*obj->getMap())["object_type"]->getString();
  if (types.find(obj_type) ==  types.end()) {{
    throw proto::SchemaError("Unknown object_type: " + obj_type);
  }}
  out = types[obj_type](obj);
}}

std::shared_ptr<MsgpackObject> toMsgpackObject(std::shared_ptr<{0}> val) {{
  return val->serializeToMsgpackObject();
}}
'''.format(cls.cpp_type()[1])
        return code

    @classmethod
    def generate_header_code(cls, bases=None):
        return super(PolymorphicModel, cls).generate_header_code(
            [
                (cls.__bases__[0].__name__, '"{}"'.format(cls.object_type))
            ])

    @classmethod
    def generate_source_code(cls, extra_pack=None):
        return super(PolymorphicModel, cls).generate_source_code(
            ['object_type'])
