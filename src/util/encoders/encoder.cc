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
#include "util/encoders/encoder.h"

namespace veles {
namespace util {
namespace encoders {

QString stripSpaces(QString str) {
  QString newStr;

  for (auto character : str) {
    if (!character.isSpace()) {
      newStr.append(character);
    }
  }

  return newStr;
}

bool Encoder::validateEncoded(const QString &str) {
  QString toCompare = encode(decode(str));

  return stripSpaces(str) == stripSpaces(toCompare);
}

}  // namespace encoders
}  // namespace util
}  // namespace veles
