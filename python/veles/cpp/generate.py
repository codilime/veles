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
from veles.proto import node, messages, check, operation
import six


def generate_cpp_code():
    if six.PY2:
        raise RuntimeError('C++ code can only be generated on Python 3.x')
    code = '#define MSGPACK_CLASSES_HEADER \\\n'
    classes_to_generate = [node.Node, node.PosFilter]
    poly_classes = [messages.MsgpackMsg, check.Check, operation.Operation]
    for cls_type in classes_to_generate:
        code += 'class {};\\\n'.format(cls_type.cpp_type())
    for cls_type in poly_classes:
        code += 'class {};\\\n'.format(cls_type.cpp_type())
        for sub_class in cls_type.object_types.values():
            code += 'class {};\\\n'.format(sub_class.cpp_type())
    for cls_type in classes_to_generate:
        code += cls_type.generate_header_code()
    for cls_type in poly_classes:
        code += cls_type.generate_base_header_code()
        for sub_class in cls_type.object_types.values():
            code += sub_class.generate_header_code()

    code += '\n'

    code += '#define MSGPACK_CLASSES_SOURCE \\\n'
    for cls_type in classes_to_generate:
        code += cls_type.generate_source_code()
    for cls_type in poly_classes:
        code += cls_type.generate_base_source_code()
        for sub_class in cls_type.object_types.values():
            code += sub_class.generate_source_code()

    return code


if __name__ == '__main__':
    code = generate_cpp_code()
    print(code)
