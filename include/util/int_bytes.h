/*
 * Copyright 2017 CodiLime
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

#include <cassert>
#include <cstddef>
#include <cstdint>

namespace veles {
namespace util {

template <typename T>
T bytesToIntLe(const uint8_t* bytes, size_t num_bytes) {
  assert(num_bytes <= sizeof(T));
  T res = T();
  for (size_t i = 0; i < num_bytes; ++i) {
    res |= static_cast<T>(bytes[i]) << (i * 8);
  }
  return res;
}

template <typename T>
void intToBytesLe(T val, size_t num_bytes, uint8_t* out) {
  assert(num_bytes <= sizeof(T));
  for (size_t i = 0; i < num_bytes; ++i) {
    out[i] = (val >> (i * 8)) & 0xff;
  }
}

}  // namespace util
}  // namespace veles
