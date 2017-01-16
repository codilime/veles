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
#include "util/encoders/base64_encoder.h"

#include <cstring>

namespace veles {
namespace util {
namespace encoders {

QString Base64Encoder::displayName(bool decode) {
  if (decode) {
    return "decode from base64";
  }

  return "encode to base64";
}

QString Base64Encoder::encode(const QByteArray &data) {
  return QString::fromLatin1(data.toBase64());
}

QByteArray Base64Encoder::decode(const QString &str) {
  return QByteArray::fromBase64(str.toLatin1());
}

}  // namespace encoders
}  // namespace util
}  // namespace veles
