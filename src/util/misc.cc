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
#include "util/misc.h"
#include <cstdint>

namespace veles {
namespace util {
namespace misc {

/**
 * This magic performs the only safe method
 * of casting uchar to char according to the C++ standard.
*/
char ucharToChar(unsigned char value) {
  if (value < 0x80) {
    return static_cast<char>(value);
  } else {
    int64_t int_value = static_cast<int64_t>(value);
    int_value -= 0x100;
    return static_cast<char>(int_value);
  }
}

}  // namespace misc
}  // namespace util
}  // namespace veles
