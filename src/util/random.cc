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
#include "util/random.h"

#include <algorithm>
#include <array>
#include <functional>

namespace veles {
namespace util {

std::mt19937 g_mersenne_twister = []() {
  std::array<int, 5> seed_data;
  std::random_device r;
  std::generate_n(seed_data.data(), seed_data.size(), std::ref(r));
  std::seed_seq seq(seed_data.begin(), seed_data.end());
  return std::mt19937(seq);
}();

QString generateRandomUppercaseText(int size) {
  QString text = "";
  for (int i = 0; i < size; ++i) {
    unsigned random_value = g_mersenne_twister();
    text += QString("%1").arg(QChar('A' + random_value % 26));
  }
  return text;
}

QString generateSecureRandomConnectionKey() {
#ifdef __MINGW32__
// On MinGW random_device outputs the same sequence every time (sic!).
#error "std::random_device is broken on MinGW"
#endif
  // TODO(mkow): This is cryptographically-secure on all modern OS-es, but this
  // isn't explicitly guaranteed by the standard. We should fix it someday.
  std::random_device rd;
  std::uniform_int_distribution<uint32_t> uniform;
  auto gen_key_part = [&rd, &uniform]() {
    return QString("%1").arg(uniform(rd), /*fieldWidth=*/8, /*base=*/16,
                             QChar('0'));
  };
  return gen_key_part() + gen_key_part() + gen_key_part() + gen_key_part();
}

}  // namespace util
}  // namespace veles
