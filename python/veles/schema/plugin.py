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

from veles.schema import fields


class MethodSignature:
    def __init__(self, name, params, result):
        if not isinstance(name, six.text_type):
            raise TypeError("name must be str")
        if not isinstance(params, fields.Field):
            raise TypeError("params must be a field")
        if not isinstance(result, fields.Field):
            raise TypeError("result must be a field")
        self.name = name
        self.params = params
        self.result = result


class QuerySignature:
    def __init__(self, name, params, result):
        if not isinstance(name, six.text_type):
            raise TypeError("name must be str")
        if not isinstance(params, fields.Field):
            raise TypeError("params must be a field")
        if not isinstance(result, fields.Field):
            raise TypeError("result must be a field")
        self.name = name
        self.params = params
        self.result = result


class BroadcastSignature:
    def __init__(self, name, params, result):
        if not isinstance(name, six.text_type):
            raise TypeError("name must be str")
        if not isinstance(params, fields.Field):
            raise TypeError("params must be a field")
        if not isinstance(result, fields.Field):
            raise TypeError("result must be a field")
        self.name = name
        self.params = params
        self.result = result
