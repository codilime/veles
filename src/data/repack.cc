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

#include <cstdlib>

#include "util/math.h"

using veles::util::math::gcd;

namespace veles {
namespace data {

unsigned Repacker::repackUnit() const {
  unsigned res = paddedWidth() / gcd(paddedWidth(), from_width) * from_width;
  // Ensure no overflow.
  assert(res % paddedWidth() == 0 && res % from_width == 0);
  return res;
}

size_t Repacker::repackSize(size_t num_elements) const {
  size_t bits = num_elements * paddedWidth();
  assert(bits / paddedWidth() == num_elements);
  return (bits + from_width - 1) / from_width;
}

size_t Repacker::repackableSize(size_t src_size) const {
  size_t bits = from_width * src_size;
  assert(bits / from_width == src_size);
  return bits / paddedWidth();
}

BinData Repacker::repack(const BinData& src, size_t start,
                         size_t num_elements) const {
  unsigned repack_unit = repackUnit();
  unsigned src_per_unit = repack_unit / from_width;
  unsigned dst_per_unit = repack_unit / paddedWidth();
  assert(src.width() == from_width);
  BinData workspace(repack_unit, 1);
  assert(start <= src.size());
  num_elements = std::min(num_elements, repackableSize(src.size() - start));
  BinData res(to_width, num_elements);
  size_t src_end = start + repackSize(num_elements);
  assert(src_end <= src.size());
  for (size_t dst_pos = 0, src_pos = start; dst_pos < num_elements;) {
    for (unsigned i = 0; i < src_per_unit && src_pos < src_end;
         i++, src_pos++) {
      unsigned work_pos;
      switch (endian) {
        case Endian::LITTLE:
          work_pos = i * from_width;
          break;
        case Endian::BIG:
          work_pos = (src_per_unit - i - 1) * from_width;
          break;
        default:
          abort();
      }
      BinData::copyBits(workspace.rawData(), work_pos, src.rawData(src_pos), 0,
                        from_width);
    }
    for (unsigned i = 0; i < dst_per_unit && dst_pos < num_elements;
         i++, dst_pos++) {
      unsigned work_pos;
      switch (endian) {
        case Endian::LITTLE:
          work_pos = i * paddedWidth() + low_pad;
          break;
        case Endian::BIG:
          work_pos = (dst_per_unit - i - 1) * paddedWidth() + low_pad;
          break;
        default:
          abort();
      }
      BinData::copyBits(res.rawData(dst_pos), 0, workspace.rawData(), work_pos,
                        to_width);
    }
  }
  return res;
}

}  // namespace data
}  // namespace veles
