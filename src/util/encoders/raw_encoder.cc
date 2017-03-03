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
#include "util/encoders/raw_encoder.h"

#include <cstring>

namespace veles {
namespace util {
namespace encoders {

QString RawEncoder::displayName() {
  return "Raw";
}

QString RawEncoder::encode(const QByteArray &data) {
  return QString::fromLatin1(data);
}

QByteArray RawEncoder::decode(const QString &str) {
  return str.toLatin1();
}

bool RawEncoder::validateEncoded(const QString &str) {
  return Encoder::validateEncoded(str.toLower());
}

}  // namespace encoders
}  // namespace util
}  // namespace veles
