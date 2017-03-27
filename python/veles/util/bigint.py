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

from veles.compatibility.int_bytes import int_to_bytes, int_from_bytes


HDR_BIAS = 0x80000000
HDR_LEN = 4


def bigint_encode(val):
    """
    Encodes an arbitrarily-sized integer to bytes in a way that preserves
    ordering - ``bigint_encode(a) < bigint_encode(b)`` iff ``a < b``.

    The exact encoding is as follows:

    - first 4 bytes of the encoded value is a big endian number called the
      header.  It represents the number's size (in bytes) and sign.  Positive
      numbers have ``0x80000000 + size`` as the header, while negative numbers
      are have ``0x80000000 - size``.
    - the rest of the encoding represents the number itself, encoded as big
      endian of ``size`` bytes.  If the number is positive, it is stored
      directly.  If it is negative, its absolute value is stored with all bits
      inverted.
    - the number is always stored with the smallest size that will fit it -
      in particular, 0 is stored with a header of 0x80000000 and no payload.
    """
    val = operator.index(val)
    sz = (val.bit_length() + 7) // 8
    if val < 0:
        hdr = HDR_BIAS - sz
        rval = -val ^ ((1 << (sz * 8)) - 1)
    else:
        hdr = HDR_BIAS + sz
        rval = val
    return int_to_bytes(hdr, HDR_LEN, 'big') + int_to_bytes(rval, sz, 'big')


def bigint_decode(val):
    """
    Reverses the encoding performed by ``bigint_encode``.  Note that not all
    invalid encodings are detected.
    """
    if not isinstance(val, bytes):
        raise TypeError('encoded bigint must be bytes')
    if len(val) < HDR_LEN:
        raise ValueError('encoded bigint too short')
    hdr = int_from_bytes(val[:HDR_LEN], 'big')
    sz = abs(hdr - HDR_BIAS)
    rest = val[HDR_LEN:]
    if len(rest) != sz:
        raise ValueError('encoded bigint has wrong header')
    rval = int_from_bytes(rest, 'big')
    if hdr < HDR_BIAS:
        return -(rval ^ ((1 << (sz * 8)) - 1))
    else:
        return rval
