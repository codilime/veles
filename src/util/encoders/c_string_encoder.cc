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
#include "util/encoders/c_string_encoder.h"

#include "util/string_utils.h"

namespace veles {
namespace util {
namespace encoders {

QString CStringEncoder::encodingDisplayName() { return "C String"; }

QString CStringEncoder::encode(const QByteArray& data) {
  QString res = "\"";
  for (auto byte : data) {
    res += QString::asprintf("\\x%02x", static_cast<unsigned char>(byte));
  }
  res += "\"";
  return res;
}

}  // namespace encoders
}  // namespace util
}  // namespace veles
