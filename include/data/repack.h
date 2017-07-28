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
#pragma once

#include "data/bindata.h"
#include "models.h"

namespace veles {
namespace data {

/** Represents data repacking format.  */
class Repacker : public RepackerModel {
 public:
  explicit Repacker(Endian endian = Endian::LITTLE, uint64_t from_width = 8,
                    uint64_t to_width = 8, uint64_t high_pad = 0,
                    uint64_t low_pad = 0)
      : RepackerModel(endian, from_width, to_width, high_pad, low_pad) {}
  using RepackerModel::RepackerModel;

  /** Element width, including padding.  */
  unsigned paddedWidth() const { return to_width + high_pad + low_pad; }

  /** Returns repacking unit for given format, ie. lowest common multiple
      of source and padded destination widths in bits.  */
  unsigned repackUnit() const;

  /** Returns the number of source elements that would be read to perform
      the given repacking.  This is equal to
      ceil(paddedWidth() * num_elements / from_width).  */
  size_t repackSize(size_t num_elements) const;

  /** Returns the number of destination elements that can be retrieved
      from the given source with the given repacking.  This is equal to
      floor(from_width * src_size / paddedWidth()).  */
  size_t repackableSize(size_t src_size) const;

  /** Repacks binary data to a different element width.  Repacking conceptually
      works as follows:

      1. All source elements starting from the given index, are glued
         together to form a big string of bits.  If LITTLE endian is selected,
         lower-indexed elements are less significant in the resulting
         string, otherwise they are more significant.
      2. Starting from LSB (for LITTLE endian) or MSB (for BIG
         endian) of the resulting string, num_elements * (format.width +
         format.lowPad + format.highPad) bits are extracted and cut into
         num_elements pieces (again ordered according to the selected
         endian).
      3. From each of the num_element pieces, high highPad and low lowPad bits
         are cut off, and the rest is returned as the result.

      Of course, the actual repacking operation only reads as many source
      elements as are actually necessary to determine the output.  This number
      can be determined by the repackSize() function.  It is an error if fewer
      elements than that are available in the source.  */
  BinData repack(const BinData& src, size_t start, size_t num_elements) const;
};

}  // namespace data
}  // namespace veles
