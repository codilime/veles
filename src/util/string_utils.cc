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
#include "util/string_utils.h"

namespace veles {
namespace util {
namespace string {

QString filter(QString src, const std::function<bool(const QChar&)>& pred) {
  QString newStr;
  for (auto character : src) {
    if (pred(character)) {
      newStr.append(character);
    }
  }
  return newStr;
}

QString stripNulls(const QString& src) {
  return filter(src, [](const QChar& x) { return !x.isNull(); });
}

QString stripSpaces(const QString& src) {
  return filter(src, [](const QChar& x) { return !x.isSpace(); });
}

}  // namespace string
}  // namespace util
}  // namespace veles
