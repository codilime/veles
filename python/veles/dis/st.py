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

from veles.messages import fields
from veles.schema import model
from . import mem
from . import reg


class IsaSTArg(model.PolymorphicModel):
    """
    A base class of syntax tree argument nodes.
    """


class IsaSTInsn(model.Model):
    """
    A syntax tree node representing a single instruction.  Fields:

    - name: str, or None if instruction unknown (ie. disassembly failure)
    - args: list of IsaSTArg
    """

    name = fields.String(optional=True)
    args = fields.Array(
        elements_types=[fields.Object(local_type=IsaSTArg)],
        local_type=tuple, optional=True)
    mods = fields.Array(local_type=tuple, optional=True)

    def __str__(self):
        name = self.name or "???"
        res = name + ''.join(".{}".format(mod) for mod in self.mods)
        if self.args:
            res += " " + ", ".join(str(x) for x in self.args)
        return res

    def __repr__(self):
        return "IsaSTInsn<{}>".format(self)


class IsaSTUnkArg(IsaSTArg):
    """
    A syntax tree node representing an unknown argument (ie. disassembly
    failure).
    """

    object_type = 'IsaSTUnkArg'

    def __str__(self):
        return "???"

    def __repr__(self):
        return "IsaSTUnkArg()"


class IsaSTImm(IsaSTArg):
    """
    A syntax tree node representing an immediate value.  Fields:

    - width: a width in bits (not directly displayed)
    - base: None if value is absolute, otherwise the value base
    - val: the value (if absolute), or offset from base
    """
    # XXX: define what exactly is a base

    object_type = 'IsaSTImm'

    width = fields.Integer(minimum=1)
    base = fields.Integer(optional=True, minimum=2)
    val = fields.Integer()

    def __str__(self):
        if self.base is not None:
            return "{}+{:#x}".format(self.base, self.val)
        return hex(self.val)

    def __repr__(self):
        return "IsaSTImm({})".format(self)


class IsaSTReg(IsaSTArg):
    """
    A syntax tree node representing a register argument.  Fields:

    - reg: an instance of BaseRegister class.
    """
    # XXX: needs a change to make it serializable

    object_type = 'IsaSTReg'

    reg = fields.Object(local_type=reg.BaseRegister)

    def __str__(self):
        return "${}".format(self.reg.name)

    def __repr__(self):
        return "IsaSTReg<{}>".format(self)


class IsaSTMem(IsaSTArg):
    """
    A syntax tree node representing a memory reference.  Fields:

    - space: an instance of MemSpace class.
    - expr: an instance of IsaSTArg, IsaSTAdd, or IsaSTMul.
    - seg: an instance of ISaSTArg, or None
    """
    # XXX: needs a change to make it serializable

    object_type = 'IsaSTMem'

    space = fields.Object(local_type=mem.MemSpace)
    expr = fields.Object(local_type=IsaSTArg)
    seg = fields.Object(local_type=IsaSTArg, optional=True)

    def __str__(self):
        res = "{}[{}]".format(self.space.name, self.expr)
        if self.seg is not None:
            res = "{}:{}".format(self.seg, res)
        return res

    def __repr__(self):
        return "IsaSTMem<{}>".format(self)


# XXX: integrate the following two into IsaSTMem?

class IsaSTMul(IsaSTArg):
    """
    A syntax tree node representing a multiplication (for addresses in mem
    references).  e1 and e2 are IsaSTArg instances.
    """

    object_type = 'IsaSTMul'

    e1 = fields.Object(local_type=IsaSTArg)
    e2 = fields.Object(local_type=IsaSTArg)

    def __str__(self):
        return "{}*{}".format(self.e1, self.e2)

    def __repr__(self):
        return "IsaSTMul<{}>".format(self)


class IsaSTAdd(IsaSTArg):
    """
    A syntax tree node representing an addition (for addresses in mem
    references).  e1 and e2 are IsaSTArg or IsaSTMul instances.
    """

    object_type = 'IsaSTAdd'

    e1 = fields.Object(local_type=IsaSTArg)
    e2 = fields.Object(local_type=IsaSTArg)

    def __str__(self):
        return "{}+{}".format(self.e1, self.e2)

    def __repr__(self):
        return "IsaSTAdd<{}>".format(self)
