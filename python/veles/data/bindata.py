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

import operator

from six.moves import range

from veles.compatibility.int_bytes import int_to_bytes, int_from_bytes


class BinData(object):
    """
    Represents all kinds of uniform-sized raw binary data.

    Conceptually, a BinData instance is a mutable array of <size> <width>-bit
    sized elements, where width can be any reasonably sized integer (up to
    several thousand should be OK).  The data is stored as a big array of
    octets - each element is stored as ceil(width/8) octets in little-endian
    format.

    BinData instances behave as mutable sequences that can contain only ints
    that fit in the range of ``width``-bit unsigned numbers.

    To create a copy of a BinData instance, use slicing: ``x[:]``.
    """

    def __init__(self, width, data=()):
        """
        Creates a BinData instance from given data.  ``data`` can be one of:

        - an interable of ints, representing the elements.  Each int must be
          in range for a ``width``-bit unsigned number.
        - an int: this creates a zero-filled BinData instance of the given
          size.
        """
        width = operator.index(width)
        if width <= 0:
            raise ValueError('BinData width must be positive')
        self._width = width
        ope = self.octets_per_element()
        if isinstance(data, int):
            self._raw_data = bytearray(data * ope)
        else:
            self._raw_data = bytearray()
            for x in data:
                x = operator.index(x)
                if x >= (1 << width) or x < 0:
                    raise ValueError('BinData element out of range for width')
                self._raw_data += int_to_bytes(x, ope, 'little')

    @classmethod
    def from_spaced_hex(cls, width, data):
        """
        Creates a BinData instance from a string containing space-separated
        hexadecimal numbers - this is converted to an array of ints and
        treated passed to the usual constructor.
        """
        data = [int(x, 16) for x in data.split()]
        return BinData(width, data)

    @classmethod
    def from_raw_data(cls, width, data):
        """
        Creates a BinData instance from given raw data.  Raw data must
        be a bytes-like object containing packed elements, with each
        element represented as ceil(width/8) octets in little-endian
        format.
        """
        self = cls(width)
        self._raw_data = bytearray(data)
        if len(self._raw_data) % self.octets_per_element() != 0:
            raise ValueError('raw data length not a multiple of element size')
        ope = self.octets_per_element()
        if self._width % 8 != 0:
            to_check = self._raw_data[ope-1::ope]
            correct = range(1 << (self._width % 8))
            if not all(x in correct for x in to_check):
                raise ValueError('raw data with non-zero unused bits')
        return self

    @property
    def width(self):
        """
        Returns this instance's width in bits.
        """
        return self._width

    @property
    def raw_data(self):
        """
        Returns this instance's raw data as bytes.  May be changed to
        return a memoryview in the future.
        """
        return bytes(self._raw_data)

    def __hash__(self):
        return hash((self._width, bytes(self._raw_data)))

    def __eq__(self, other):
        """
        Compares two BinData instances for equality.  Instances are considered
        equal iff they have the same width and data.  A BinData instance is
        not equal to instances of any other class.
        """
        return (isinstance(other, BinData) and
                self._width == other._width and
                self._raw_data == other._raw_data)

    def __len__(self):
        """
        Returns the length of this BinData instance in elements.
        """
        return len(self._raw_data) // self.octets_per_element()

    def octets_per_element(self):
        """
        Returns how many octets each element takes in the raw data.
        """
        return (self._width + 7) // 8

    def octets(self):
        """
        Returns size of the raw data of this instance, in octets.
        Equal to ``len(self) * self.octets_per_element()``.
        """
        return len(self._raw_data)

    def __getitem__(self, idx):
        """
        When the index is an int, returns a single element as an int.
        Negative indices are supported and access elements from the
        end of the array (as usual for Python sequences).

        When the index is a slice object, returns a subrange of elements
        as a BinData instance of the same width.  Slicing syntax
        works as usual for Python sequences, including negative indices,
        out of range indices, and strides (though strides other than 1
        are not particularly efficient).
        """
        ope = self.octets_per_element()
        if isinstance(idx, slice):
            start, stop, stride = idx.indices(len(self))
            if stride == 1:
                raw = self._raw_data[start * ope:stop * ope]
                return BinData.from_raw_data(self._width, raw)
            else:
                data = [self[x] for x in range(start, stop, stride)]
                return BinData(self._width, data)
        else:
            idx = operator.index(idx)
            if idx < 0:
                if idx < -len(self):
                    raise IndexError('BinData index out of range')
                idx += len(self)
            else:
                if idx >= len(self):
                    raise IndexError('BinData index out of range')
            raw = self._raw_data[ope * idx:ope * (idx + 1)]
            return int_from_bytes(raw, 'little')

    def __setitem__(self, idx, val):
        """
        When the index is an int, sets a single element to a given value,
        The value must be an int in the correct range.  Negative indices
        are supported and access elements from the end of the array
        (as usual for Python sequences).

        When the index is a slice object, the assigned value must be a
        BinData instance with the same width.  The elements selected
        by the slice are replaced with ones from the other BinData.
        If stride is 1, this can be used to expand or shrink this instance
        (though this is not very efficient).  If stride is not 1,
        the operation is not efficient, and the length of the slice must be
        equal to the length of the assigned value.
        """
        ope = self.octets_per_element()
        if isinstance(idx, slice):
            start, stop, stride = idx.indices(len(self))
            if not isinstance(val, BinData):
                raise TypeError(
                    'value assigned to BinData slice must be BinData')
            if val._width != self._width:
                raise ValueError(
                    'value assigned to BinData slice has mismatched width')
            if stride == 1:
                self._raw_data[start * ope:stop * ope] = val._raw_data
            else:
                indices = range(start, stop, stride)
                if len(indices) != len(val):
                    raise ValueError(
                        'value assigned to extended slice has mismatched '
                        'length')
                for i, v in zip(indices, val):
                    self[i] = v
        else:
            idx = operator.index(idx)
            if idx < 0:
                if idx < -len(self):
                    raise IndexError('BinData index out of range')
                idx += len(self)
            else:
                if idx >= len(self):
                    raise IndexError('BinData index out of range')
            val = operator.index(val)
            if val >= (1 << self._width) or val < 0:
                raise ValueError('BinData element out of range for width')
            raw = int_to_bytes(val, ope, 'little')
            self._raw_data[ope * idx:ope * (idx + 1)] = raw

    def __str__(self):
        """
        Returns the elements of this instance as space-separated hex numbers.
        Every element uses ceil(width/4) hex digits.
        """
        w = (self._width + 3) // 4
        return ' '.join('{:0{}x}'.format(x, w) for x in self)

    def __repr__(self):
        """
        Returns ``"BinData(self.width, str(self))"``.
        """
        return 'BinData.from_spaced_hex({}, \'{}\')'.format(self._width, self)

    def __add__(self, other):
        """
        Returns a BinData instance that is the concatenation of two BinData
        instances.  The concatenated instances must have the same width.
        """
        if not isinstance(other, BinData):
            return NotImplemented
        if self._width != other._width:
            raise ValueError('concatenating BinData of different widths')
        return BinData.from_raw_data(self._width,
                                     self._raw_data + other._raw_data)
