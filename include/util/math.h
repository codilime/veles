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

#include <type_traits>

namespace veles {
namespace util {
namespace math {

// Will be replaced by std::gcd after switching to C++17.
template <typename T1, typename T2>
typename std::common_type<T1, T2>::type gcd(T1 a, T2 b) {
  while (b) {
    auto tmp = a % b;
    a = b;
    b = tmp;
  }
  return a;
}

}  // namespace math
}  // namespace util
}  // namespace veles
