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
#include "data/repack.h"
#include <stdlib.h>

namespace veles {
namespace data {

static size_t gcd(size_t a, size_t b) {
  while (b) {
    size_t tmp = a % b;
    a = b;
    b = tmp;
  }
  return a;
}

unsigned repackUnit(unsigned src_width,
                    const RepackFormat &format) {
  unsigned res = format.paddedWidth() / gcd(format.paddedWidth(), src_width) * src_width;
  // Ensure no overflow.
  assert(res % format.paddedWidth() == 0 && res % src_width == 0);
  return res;
}

size_t repackSize(unsigned src_width,
                  const RepackFormat &format,
                  size_t num_elements) {
  size_t bits = num_elements * format.paddedWidth();
  assert(bits / format.paddedWidth() == num_elements);
  return (bits + src_width - 1) / src_width;
}

size_t repackableSize(unsigned src_width,
                      const RepackFormat &format,
                      size_t src_size) {
  size_t bits = src_width * src_size;
  assert(bits / src_width == src_size);
  return bits / format.paddedWidth();
}

BinData repack(const BinData &src,
               const RepackFormat &format,
               size_t start, size_t num_elements) {
  unsigned repack_unit = repackUnit(src.width(), format);
  unsigned src_per_unit = repack_unit / src.width();
  unsigned dst_per_unit = repack_unit / format.paddedWidth();
  BinData workspace(repack_unit, 1);
  assert(start <= src.size());
  num_elements = std::min(num_elements,
    repackableSize(src.width(), format, src.size() - start));
  BinData res(format.width, num_elements);
  size_t src_end = start + repackSize(src.width(), format, num_elements);
  assert(src_end <= src.size());
  for (size_t dst_pos = 0, src_pos = start; dst_pos < num_elements;) {
    for (unsigned i = 0; i < src_per_unit && src_pos < src_end; i++, src_pos++) {
      unsigned work_pos;
      switch (format.endian) {
      case RepackEndian::LITTLE:
        work_pos = i * src.width();
        break;
      case RepackEndian::BIG:
        work_pos = (src_per_unit - i - 1) * src.width();
        break;
      default:
        abort();
      }
      BinData::copyBits(workspace.rawData(), work_pos, src.rawData(src_pos), 0, src.width());
    }
    for (unsigned i = 0; i < dst_per_unit && dst_pos < num_elements; i++, dst_pos++) {
      unsigned work_pos;
      switch (format.endian) {
      case RepackEndian::LITTLE:
        work_pos = i * format.paddedWidth() + format.lowPad;
        break;
      case RepackEndian::BIG:
        work_pos = (dst_per_unit - i - 1) * format.paddedWidth() + format.lowPad;
        break;
      default:
        abort();
      }
      BinData::copyBits(res.rawData(dst_pos), 0, workspace.rawData(), work_pos, format.width);
    }
  }
  return res;
}

}
}
