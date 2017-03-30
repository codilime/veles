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

from enum import Enum
import operator

from six.moves import range

from .bindata import BinData

try:
    from math import gcd
except ImportError:
    from fractions import gcd

from veles.schema.model import Model
from veles.schema import fields


class Endian(Enum):
    """
    Represents endianness used for repacking data.
    """
    LITTLE = 'little'
    BIG = 'big'


class Repacker(Model):
    """
    Repacks binary data to a different element width.  Repacking conceptually
    works as follows:

    1. All source elements starting from the given index, are glued
       together to form a big string of bits.  If LITTLE endian is selected,
       lower-indexed elements are less significant in the resulting
       string, otherwise they are more significant.
    2. Starting from LSB (for LITTLE endian) or MSB (for BIG
       endian) of the resulting string, num_elements * (repacker.to_width +
       repacker.low_pad + repacker.high_pad) bits are extracted and cut into
       num_elements pieces (again ordered according to the selected
       endian).
    3. From each of the num_element pieces, high high_pad and low low_pad bits
       are cut off, and the rest is returned as the result.

    Of course, the actual repacking operation only reads as many source
    elements as are actually necessary to determine the output.  This number
    can be determined by the repack_size() function.  It is an error if fewer
    elements than that are available in the source.
    """

    endian = fields.Enum(Endian)
    from_width = fields.SmallUnsignedInteger(minimum=1)
    to_width = fields.SmallUnsignedInteger(minimum=1)
    high_pad = fields.SmallUnsignedInteger(default=0)
    low_pad = fields.SmallUnsignedInteger(default=0)

    def __init__(self, endian, from_width, to_width, **kwargs):
        super(Repacker, self).__init__(
            endian=endian,
            from_width=from_width,
            to_width=to_width,
            **kwargs
        )

    @property
    def padded_width(self):
        """
        Destination element width, including padding.
        """
        return self.to_width + self.high_pad + self.low_pad

    @property
    def repack_unit(self):
        """
        Returns repacking unit for given format, ie. lowest common multiple
        of source and padded destination widths in bits.
        """
        g = gcd(self.padded_width, self.from_width)
        return self.padded_width * self.from_width // g

    def repack_size(self, num_elements):
        """
        Returns the number of source elements that would be read to perform
        the given repacking.  This is equal to
        ceil(self.padded_width * num_elements / self.from_width).
        """
        num_elements = operator.index(num_elements)
        if num_elements < 0:
            raise ValueError('number of elements cannot be negative')
        bits = num_elements * self.padded_width
        return (bits + self.from_width - 1) // self.from_width

    def repackable_size(self, from_size):
        """
        Returns the number of destination elements that can be retrieved
        from the given source with the given repacking.  This is equal to
        floor(self.from_width * from_size / self.padded_width).
        """
        if from_size < 0:
            raise ValueError('source size cannot be negative')
        bits = self.from_width * from_size
        return bits // self.padded_width

    def repack(self, src, start=0, num_elements=None):
        """
        Repacks data from the given source BinData.  The width of the source
        must match the width specified at repacker construction.  ``start``
        specifies the source index (in source elements) where unpacking
        should start.  ``num_element`` specifies the number of destination
        elements that should be returned (or None to repack as much data
        as possible).
        """
        if not isinstance(src, BinData):
            raise TypeError('repack needs a BinData instance')
        if self.from_width != src.width:
            raise ValueError('repack source width mismatch')
        start = operator.index(start)
        if start < 0:
            raise ValueError('start cannot be negative')
        ru = self.repack_unit
        spu = ru // self.from_width
        dpu = ru // self.padded_width
        if num_elements is None:
            num_elements = self.repackable_size(len(src) - start)
        rs = self.repack_size(num_elements)
        if start + rs > len(src):
            raise ValueError('not enough data in source')
        units = (rs + spu - 1) // spu
        res = BinData(self.to_width, num_elements)
        mask = (1 << self.to_width) - 1
        for u in range(units):
            unit = 0
            sp = start + u * spu
            for i, x in enumerate(src[sp:sp + spu]):
                if self.endian is Endian.LITTLE:
                    unit |= x << (i * self.from_width)
                else:
                    unit |= x << ((spu - i - 1) * self.from_width)
            for i in range(min(dpu, num_elements - u * dpu)):
                if self.endian is Endian.LITTLE:
                    pos = i * self.padded_width
                else:
                    pos = (dpu - i - 1) * self.padded_width
                res[u * dpu + i] = unit >> (pos + self.low_pad) & mask
        return res
