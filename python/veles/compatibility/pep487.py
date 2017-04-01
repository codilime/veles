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

import sys
import types
from collections import OrderedDict

import six


if sys.version_info < (3, 6):
    class NewType(type):
        def __init__(self, name, bases, ns, **kwargs):
            super(NewType, self).__init__(name, bases, ns)

        def __new__(cls, *args, **kwargs):
            if len(args) != 3:
                return super(NewType, cls).__new__(cls, *args)

            name, bases, ns = args

            init = ns.get('__init_subclass__')
            if isinstance(init, types.FunctionType):
                ns['__init_subclass__'] = classmethod(init)

            if six.PY3:
                ns['_order'] = list(ns)

            self = super(NewType, cls).__new__(cls, name, bases, ns)

            for k, v in self.__dict__.items():
                func = getattr(v, '__set_name__', None)
                if func is not None:
                    func(self, k)

            init = getattr(super(self, self), '__init_subclass__', None)
            if init is not None:
                init(**kwargs)

            return self

        @classmethod
        def __prepare__(metacls, name, bases):
            # Never, ever depend on it apart from code generation which must
            # be run on Python3
            return OrderedDict()

    class NewObject(six.with_metaclass(NewType, object)):
        @classmethod
        def __init_subclass__(cls, **kwargs):
            pass

else:
    NewType = type
    NewObject = object
