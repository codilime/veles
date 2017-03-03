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

# Fields


class IsaBaseField:
    """
    Represents an instruction field - ie. a string of bits that is somehow
    read from the code segment by the parser, and used for instruction
    decoding.

    All subclasses need to have a "width" field, which is their width in bits.
    """

    def __init__(self):
        self.name = None

    def fullmask(self):
        """
        Returns the "full mask" of this field - ie. mask of all bits that
        could possibly be set in it.
        """
        return (1 << self.width) - 1

    def get(self, state):
        """
        Gets the value of this field, given the disassembler state.  Returns
        a (value, mask) tuple - the mask is a bitmask of bits that have
        actually been successfully read from the code segment by the parser.
        """
        raise NotImplementedError

    def set(self, state, val, mask):
        """
        Sets (part of) the value of this field in the disassembler state.
        Bits set in the mask are considered to have just been successfully
        read (with the values taken from corresponding bits in val).  If any
        bits in the mask collide with already-read bytes in the state, it's
        a bug in the parser, and an assertion will be triggered.  ``val``
        should only contain bits that are covered by ``mask``.
        """
        raise NotImplementedError

    def setrm(self, state, mask):
        """
        Marks a given part of the field as consumed by instruction decoding.
        It's not an error to mark some bits as consumed multiple times (they
        could affect decoding in multiple places, after all).
        """
        raise NotImplementedError

    def __set_name__(self, owner, name):
        self.owner = owner
        self.name = name

    def __repr__(self):
        return self.name


class IsaField(IsaBaseField):
    """
    A top-level instruction field (ie. not a subfield).
    """

    def __init__(self, width):
        super().__init__()
        self.width = width
        assert isinstance(width, int)
        assert width > 0

    def get(self, state):
        return state.iw.get(self, 0), state.iwmask.get(self, 0)

    def set(self, state, val, mask):
        assert (val & ~mask) == 0
        assert (mask & ~self.fullmask()) == 0
        if self not in state.iw:
            state.iw[self] = 0
            state.iwmask[self] = 0
            state.iwrm[self] = 0
        state.iw[self] |= val
        assert (state.iwmask[self] & mask) == 0
        state.iwmask[self] |= mask

    def setrm(self, state, mask):
        state.iwrm[self] |= mask


class IsaSubField(IsaBaseField):
    """
    A subfield of another instruction field, selected by starting bit
    position and width.
    """

    def __init__(self, parent_field, start, width):
        super().__init__()
        self.parent_field = parent_field
        self.start = start
        self.width = width
        assert isinstance(parent_field, IsaBaseField)
        assert isinstance(start, int)
        assert isinstance(width, int)
        assert start >= 0
        assert width > 0
        assert start+width <= parent_field.width

    def get(self, state):
        pval, pmask = self.parent_field.get(state)
        fmask = self.fullmask()
        return pval >> self.start & fmask, pmask >> self.start & fmask

    def set(self, state, val, mask):
        assert mask < (1 << self.width)
        self.parent_field.set(state, val << self.start, mask << self.start)

    def setrm(self, state, mask):
        self.parent_field.setrm(state, mask << self.start)


class IsaSplitField(IsaBaseField):
    """
    A field made up of several discontiguous smaller fields.  ``subfields``
    is the list of fields to glue together, starting from LSB.
    """

    def __init__(self, *subfields):
        super().__init__()
        pos = 0
        self.subfields = []
        for field in subfields:
            assert isinstance(field, IsaBaseField)
            self.subfields.append((pos, field))
            pos += field.width
        self.width = pos

    def get(self, state):
        val = mask = 0
        for p, f in self.subfields:
            sv, sm = f.get(state)
            val |= sv << p
            mask |= sm << p
        return val, mask

    def set(self, state, val, mask):
        for p, f in self.subfields:
            m = f.fullmask()
            f.set(state, val >> p & m, mask >> p & m)

    def setrm(self, state, mask):
        for p, f in self.subfields:
            m = f.fullmask()
            f.setrm(state, mask >> p & m)


# Matches

class MatchError(Exception):
    """
    Raised (and then stored to dstate.errors) if an IsaSwitch failed to match
    any case.
    """

    # XXX: could use some details about the broken field...


class IsaSwitch:
    """
    A base class for decoders acting as a "switch" over an instruction field.

    The parameters are an instruction field to switch on, and a list of
    possible matches.
    """

    def __init__(self, field, matches):
        self.field = field
        self.matches = matches
        fm = self.field.fullmask()
        for m in matches:
            assert isinstance(m, IsaMatch)
            if m.mask is None:
                m.mask = fm

    def find(self, state):
        """
        Finds the matching entry given a disassembly state and returns its
        action.  If no entry matches, or the field wasn't read correctly,
        a MatchError is raised.
        """
        v, m = self.field.get(state)
        matched = False
        maybe_matched = False
        for x in self.matches:
            if (v & x.mask & m) == (x.val & m):
                if (x.mask & m) != x.mask:
                    if matched:
                        raise MatchError("multiple matches")
                    maybe_matched = True
                else:
                    if matched or maybe_matched:
                        raise MatchError("multiple matches")
                    matched = True
                    res = x.action
                    self.field.setrm(state, x.mask)
        if maybe_matched:
            if m != (1 << self.field.width) - 1:
                raise MatchError("matching on unknown field")
        if not matched:
            raise MatchError("no match")
        return res


class IsaMatch:
    """
    Represents a single case of IsaSwitch - associates a field value with
    an action (whose semantics are determined by IsaSwitch subclass).

    The value can be either a simple value (ie. an integer), or a (value, mask)
    tuple (which will be considered to match if the field bits selected by the
    mask are equal to the value).
    """

    def __init__(self, val, action):
        if isinstance(val, tuple):
            self.val, self.mask = val
        else:
            self.val = val
            self.mask = None
        self.action = action
