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
#include "util/encoders/text_encoder.h"
#include "util/string_utils.h"

using veles::util::string::stripNulls;

namespace veles {
namespace util {
namespace encoders {

QString TextEncoder::encodingDisplayName() {
  return "Text";
}

QString TextEncoder::decodingDisplayName() {
  return "Raw data";  // User can paste anything, not only text.
}

QString TextEncoder::encode(const QByteArray& data) {
  // On Windows, there's no way to copy a string containing nulls to clipboard,
  // we have to strip them.
  return stripNulls(QString::fromLatin1(data.data(), data.size()));
}

QByteArray TextEncoder::decode(const QString& str) {
  return str.toLatin1();
}

}  // namespace encoders
}  // namespace util
}  // namespace veles
