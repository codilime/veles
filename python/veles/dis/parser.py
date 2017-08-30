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

from .field import IsaSwitch, MatchError


class ParseOOBError(Exception):
    """
    Inserted to dstate.errors to represent an OOB access to the code segment
    while decoding the instruction.
    """


class ParseState:
    def __init__(self, res, data, pos):
        self.res = res
        self.data = data
        self.pos = pos


class ParseWord:
    """
    Fetches a single field from the code segment.  The width of the field must
    be a multiple of the code segment byte size.
    """

    def __init__(self, field, endian=None):
        self.field = field
        self.endian = endian

    def parse(self, pstate):
        if pstate.res.desync:
            return
        width = pstate.data.width
        nbytes = self.field.width // width
        assert self.field.width % width == 0
        if nbytes != 1:
            assert self.endian in ('little', 'big')
        mask = 0
        val = 0
        for x in range(nbytes):
            if self.endian == 'little':
                shift = x * width
            else:
                shift = (nbytes - 1 - x) * width
            if pstate.pos + x < len(pstate.data):
                val |= pstate.data[pstate.pos + x] << shift
                mask |= ((1 << width) - 1) << shift
        self.field.set(pstate.res, val, mask)
        pstate.pos += nbytes
        if pstate.pos > len(pstate.data):
            err = ParseOOBError("reading {} failed".format(self.field))
            pstate.res.errors.append(err)


class ParseInsn:
    """
    Parses a single instruction based on the fields already read.
    """

    def __init__(self, action):
        self.action = action

    def parse(self, pstate):
        insn = self.action.parse(pstate.res)
        pstate.res.insns.append(insn)


class ParseSwitch(IsaSwitch):
    """
    Selects a parser based on the value of a field - see IsaSwitch
    and IsaMatch for how the matching is done.
    """

    def parse(self, pstate):
        try:
            m = self.find(pstate.res)
        except MatchError as e:
            pstate.res.errors.append(e)
            pstate.res.desync = True
            return
        if not isinstance(m, (list, tuple)):
            m = [m]
        for x in m:
            x.parse(pstate)
