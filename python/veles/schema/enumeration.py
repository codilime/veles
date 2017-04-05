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

from enum import Enum


class EnumModel(Enum):
    @classmethod
    def cpp_type(cls):
        return cls.__name__

    @classmethod
    def generate_header_code(cls):
        code = '''enum class {0} {{\\
{1}\\
}};\\
void fromMsgpackObject(const std::shared_ptr<MsgpackObject>\
 obj, {0}& out);\\
std::shared_ptr<MsgpackObject> toMsgpackObject({0} val);\\
'''.format(cls.cpp_type(), ',\\\n'.join(
            name.upper() for name in cls.__members__))
        return code

    @classmethod
    def generate_source_code(cls):
        code = '''void fromMsgpackObject(const std::shared_ptr<MsgpackObject>\
 obj, {0}& out) {{\\
{1}\\
  throw proto::SchemaError("Unrecognized enum value");\\
}}\\
std::shared_ptr<MsgpackObject> toMsgpackObject({0} val) {{\\
  switch (val) {{\\
{2}\\
    default:\\
      throw proto::SchemaError("Unrecognized enum value");\\
  }}\\
}}\\
'''.format(cls.cpp_type(),
           '\\\n'.join(['''  if (*obj->getString() == "{1}") {{\\
    out = {0}::{2};\\
    return;\\
  }}'''.format(cls.cpp_type(), name, name.upper())
                        for name in cls.__members__]),
           '\\\n'.join(['''    case {0}::{2}:\\
      return std::make_shared<MsgpackObject>("{1}");'''.format(
               cls.cpp_type(), name, name.upper())
               for name in cls.__members__]))
        return code
