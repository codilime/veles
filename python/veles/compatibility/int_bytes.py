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


def int_to_bytes(num, size, endian):
    if not isinstance(num, six.integer_types):
        raise TypeError('converted value is not an integer')
    try:
        return num.to_bytes(size, endian)
    except AttributeError:
        if endian not in ['little', 'big']:
            raise ValueError('byteorder must be either \'little\' or \'big\'')
        if num >= (1 << (size * 8)):
            raise OverflowError('int too big to convert')
        if num < 0:
            raise OverflowError('can\'t convert negative int to unsigned')
        res = bytearray([
            num >> (x * 8) & 0xff
            for x in range(size)
        ])
        if endian == 'big':
            res = res[::-1]
        return bytes(res)


def int_from_bytes(data, endian):
    try:
        return int.from_bytes(data, endian)
    except AttributeError:
        if endian not in ['little', 'big']:
            raise ValueError('byteorder must be either \'little\' or \'big\'')
        if endian == 'big':
            data = data[::-1]
        return sum(
            x << (i * 8)
            for i, x in enumerate(bytearray(data))
        )
