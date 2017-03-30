# Copyright 2016-2017 CodiLime
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

from .parser import ParseState


class IsaParseResult:
    """
    Represents a disassembly result and intermediate states.  Has the following
    fields:

    - iw: a dictionary mapping bundle fields (IsaField instances)
      to their values, as parsed from the bundle.  Only fields that have
      actually been read are present here.  IsaSubField instances are not
      included here - they compute their values on the fly instead.
    - iwmask: like iw, but maps fields to masks of bits that have been
      successfully read from the bundle.
    - iwrm: like iw, but maps fields to masks of bits that have actually been
      used while decoding the bundle.  If bits are lit in iwmask, but not
      here, the bundle has unused fields (and it's suspicious if they are
      in non-default state, ie. usually non-0).  If bits are lit here, but not
      in iwmask, something went wrong while parsing the bundle, or there's
      a bug in the disassembler.
    - errors: a list of errors encountered while parsing this bundle (these
      are all instances of Exception subclasses).
    - len: length of the parsed bundle, in code bytes.
    - desync: a bool flag that is set if bundle parser cannot successfully
      determine where the bundle ends - if this happens, len represents
      the point where the parser gave up, and is mostly meaningless.
    - base: an expression representing the beginning of the code section this
      bundle resides in - passed straight from the dissassembler caller.
      If None, the instruction position is considered to be absolute.
    - anchors: a dictionary with ``base``-relative addresses of "points of
      interest" in the bundle - 'start' is mapped to the beginning of the
      bundle before parsing starts, and parsers can define further anchors
      as necessary.  This is mostly used to decode PC-relative arguments.
    - insns: a list of decoded instructions in this bundle (ie. IsaSTInsn
      instances).
    """
    # XXX: base needs to be better-defined once we're connected to Świtezianka.
    # Atm it's a dumb string for testing purposes...
    def __init__(self, base, start):
        self.iw = {}
        self.iwmask = {}
        self.iwrm = {}
        self.errors = []
        self.desync = False
        self.insns = []
        self.base = base
        self.len = None
        self.anchors = {
            'start': start,
        }


class Isa:
    """
    Represents an Instruction Set Architecture.  Subclass this to create your
    own.
    """

    # The top-level parser.  Subclasses need to override this.
    parser = None

    def parse(self, data, base=None, pos=0):
        """
        Runs the disassembler on given data, returning an IsaParseResult
        instance.  ``pos`` is the position in ``data`` to start disassembly
        from.  ``base`` is a symbol representing the beginning of the
        code section passed as data, or None if pos is to be treated as
        an absolute number (it affects disassembly of PC-relative arguments).
        """
        # XXX: the whole thing should use BinData, and support extra offset
        # to pos.
        res = IsaParseResult(base, pos)
        s = ParseState(res, data, pos)
        for x in self.parser:
            x.parse(s)
        res.len = s.pos - pos
        return res
