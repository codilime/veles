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

from veles.schema import model
from veles.messages import fields


class BaseRegister(model.PolymorphicModel):
    """
    Represents an ISA register.  Fields:

    - name: a name displayed for this register, should be unique.
    - width: width in bits.
    """
    name = fields.String()
    width = fields.Integer(minimum=1)

    def __str__(self):
        return "${}".format(self.name)

    __repr__ = __str__


class Register(BaseRegister):
    """
    Represents a general-purpose, top-level ISA register.  Writes to this
    register will not be considered interesting on their own, and will be
    converted wholly to SSA.
    """

    object_type = 'Register'


class RegisterPC(BaseRegister):
    """
    Represents a program counter register, ie. one that always points to
    some place in the current bundle, and thus will return a known value
    when read.
    """

    object_type = 'RegisterPC'

    anchor = fields.String()
    offset = fields.Integer()


class RegisterSP(BaseRegister):
    """
    Represents a hardware stack pointer register.
    """

    # XXX: does this warrant a special class?  Might be better to handle it
    # elsewhere.

    object_type = 'RegisterSP'


# "You know what "special" means, right?" -- mupuf
class RegisterSpecial(BaseRegister):
    """
    Represents a special register.  Reads and writes of this register will be
    considered to be interesting events, and will not be converted to SSA.
    """

    object_type = 'RegisterSpecial'


class RegisterSplit(BaseRegister):
    """
    Represents a split register, ie. one that is really multiple other
    registers accessed together as a single entity.  ``parts`` is a list
    of (start bit, register) tuples.  When accessed, the access is converted
    to multiple smaller accesses to the parts.
    """

    object_type = 'RegisterSplit'

    # TODO how should this exactly work
    parts = fields.Array()


class SubRegister(BaseRegister):
    """
    Represents a subregister of another register.  Defined by starting bit
    position in the parent and width.
    """

    object_type = 'SubRegister'

    parent = fields.Object(local_type=BaseRegister)
    start = fields.Integer(minimum=0)
