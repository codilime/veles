/*
 * Copyright 2016 CodiLime
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include "data/bindata.h"

#include <algorithm>
#include <cassert>

#include <QtGlobal>

namespace veles {
namespace data {

void BinData::copyBits(uint8_t* dst, unsigned dst_bit, const uint8_t* src,
                       unsigned src_bit, unsigned num_bits) {
  dst += dst_bit >> 3;
  src += src_bit >> 3;
  dst_bit &= 7;
  src_bit &= 7;
  while (num_bits > 0) {
    if (src_bit == 0 && dst_bit == 0 && num_bits >= 8) {
      unsigned cur_bytes = num_bits >> 3;
      memcpy(dst, src, cur_bytes);
      dst += cur_bytes;
      src += cur_bytes;
      num_bits -= cur_bytes << 3;
    } else {
      unsigned cur_bits = std::min({8 - src_bit, 8 - dst_bit, num_bits});
      uint8_t mask = (1 << cur_bits) - 1;
      uint8_t bits = (*src >> src_bit) & mask;
      *dst &= ~(mask << dst_bit);
      *dst |= bits << dst_bit;
      src_bit += cur_bits;
      dst_bit += cur_bits;
      num_bits -= cur_bits;
      if (src_bit == 8) {
        src++;
        src_bit = 0;
      }
      if (dst_bit == 8) {
        dst++;
        dst_bit = 0;
      }
    }
  }
}

QString BinData::toString(size_t maxElements) {
  QString res, suffix;

  if (maxElements > 0 && maxElements < size()) {
    suffix = ", ...";
  } else {
    maxElements = size();
  }

  for (size_t elementIndex = 0; elementIndex < maxElements; ++elementIndex) {
    if (elementIndex > 0) {
      res += ", ";
    }

    QString element = "0x";
    auto bitsLeft = width();
    auto bitsToRead = bitsLeft % 64;
    if (bitsToRead != 0) {
      element +=
          QString::number(
              bits64(elementIndex, bitsLeft - bitsToRead, bitsToRead), 16)
              .rightJustified((bitsToRead + 3) / 4, '0');
      bitsLeft -= bitsToRead;
    }

    while (bitsLeft >= 64) {
      element += QString::number(bits64(elementIndex, bitsLeft - 64, 64), 16)
                     .rightJustified(16, '0');
      bitsLeft -= 64;
    }
    assert(bitsLeft == 0);

    res += element;
  }

  return res + suffix;
}

}  // namespace data
}  // namespace veles
