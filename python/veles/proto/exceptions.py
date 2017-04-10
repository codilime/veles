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

from veles.compatibility.pep487 import NewObject


class VelesException(Exception, NewObject):
    types = {}

    def __init__(self, *args):
        if type(self) is VelesException:
            code, msg = args
            if code in VelesException.types:
                self.__class__ = VelesException.types[code]
                args = msg,
            else:
                self.code = code
        else:
            code = self.code
            if args:
                msg, = args
            else:
                msg = self.msg
        self.msg = msg
        super(VelesException, self).__init__(*args)

    def __init_subclass__(cls, **kwargs):
        super(VelesException, cls).__init_subclass__(**kwargs)
        if hasattr(cls, 'code'):
            VelesException.types[cls.code] = cls

    @classmethod
    def load(cls, value):
        if not isinstance(value, dict):
            raise SchemaError('serialized exception must be a dict')
        if set(value) != {'type', 'message'}:
            raise SchemaError('wrong set of keys in serialized exception')
        if not isinstance(value['type'], six.text_type):
            raise SchemaError('exception type must be a string')
        if not isinstance(value['message'], six.text_type):
            raise SchemaError('exception message must be a string')
        return VelesException(value['type'], value['message'])

    def dump(self):
        return {
            u'type': six.text_type(self.code),
            u'message': six.text_type(self.msg),
        }

    @classmethod
    def cpp_type(cls):
        return 'veles::proto::VelesException'

    def __eq__(self, other):
        return (isinstance(other, VelesException)
                and self.code == other.code
                and self.msg == other.msg)

    def __hash__(self):
        return hash((self.code, self.msg))


class ObjectGoneError(VelesException):
    code = 'object_gone'
    msg = "Object has been deleted, or never existed"


class ObjectExistsError(VelesException):
    code = 'object_exists'
    msg = "Object with the given id already exists"


class WritePastEndError(VelesException):
    code = 'write_past_end'
    msg = "Data written past the end of object"


class SchemaError(VelesException):
    code = 'schema_error'
    msg = "Schema violation"


class ParentCycleError(VelesException):
    code = 'parent_cycle'
    msg = "Parent cycle would be created"


class UnknownSubscriptionError(VelesException):
    code = 'unknown_subscription'
    msg = "Unknown subscription id used"


class SubscriptionInUseError(VelesException):
    code = 'subscription_in_use'
    msg = "Subscription id already in use"


class PreconditionFailedError(VelesException):
    code = 'precondition_failed'
    msg = "Transaction precondition failed"


class RegistryNoMatchError(VelesException):
    code = 'registry_no_match'
    msg = "Plugin function not found in registry"


class RegistryMultiMatchError(VelesException):
    code = 'registry_multi_match'
    msg = "Multiple matching plugin functions found in registry"


class ConnectionLostError(VelesException):
    code = 'connection_lost'
    msg = "Connection lost"


class AuthenticationError(VelesException):
    code = 'auth_error'
    msg = 'Authentication key check failed'


class ProtocolMismatchError(VelesException):
    code = 'protocol_mismatch_error'
    msg = 'Incompatible protocol version'
