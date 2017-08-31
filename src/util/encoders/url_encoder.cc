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
#include "util/encoders/url_encoder.h"

#include <cctype>

namespace veles {
namespace util {
namespace encoders {

QString UrlEncoder::encodingDisplayName() { return "Url-encoded"; }

QString UrlEncoder::decodingDisplayName() { return "Url-encoded"; }

QString UrlEncoder::encode(const QByteArray& data) {
  QString hex = QString::fromLatin1(data.toHex());
  QString res;
  for (int i = 0; i < hex.length(); i += 2) {
    res += "%" + hex.mid(i, 2);
  }
  return res;
}

QByteArray UrlEncoder::decode(const QString& str) {
  QByteArray res;
  for (int i = 0; i < str.length(); i++) {
    if (str.at(i) == '%' && i + 2 < str.length() &&
        isxdigit(str.at(i + 1).toLatin1()) != 0 &&
        isxdigit(str.at(i + 2).toLatin1()) != 0) {
      res += static_cast<unsigned char>(str.mid(i + 1, 2).toInt(nullptr, 16));
      i += 2;
    } else {
      res += str.at(i);
    }
  }
  return res;
}

}  // namespace encoders
}  // namespace util
}  // namespace veles
