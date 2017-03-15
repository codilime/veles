#!/usr/bin/env python3

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
from veles.proto import node, messages


def generate_cpp_code():
    code = '#define MSGPACK_CLASSES_DEFS \\\n'
    classes_to_generate = [node.Node]
    poly_classes = [messages.MsgpackMsg]
    for cls_type in classes_to_generate:
        code += 'class {};\\\n'.format(cls_type.cpp_type())
    for cls_type in poly_classes:
        code += 'class {};\\\n'.format(cls_type.cpp_type())
        for sub_class in cls_type.object_types.values():
            code += 'class {};\\\n'.format(sub_class.cpp_type())
    for cls_type in classes_to_generate:
        code += cls_type.generate_cpp_code()
    for cls_type in poly_classes:
        code += cls_type.generate_base_code()
        for sub_class in cls_type.object_types.values():
            code += sub_class.generate_cpp_code()

    code += '\n'

    code += '''#define MSGPACK_CLASSES_INIT \\
  static void initMessages() {{\\
{}\\
  }}'''.format(
        ''.join(['    {}::initObjectTypes();'.format(cls_type.cpp_type())
                 for cls_type in poly_classes]))

    return code


if __name__ == '__main__':
    code = generate_cpp_code()
    print(code)
