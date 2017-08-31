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
#include "util/encoders/c_data_encoder.h"

namespace veles {
namespace util {
namespace encoders {

QString CDataEncoder::encodingDisplayName() { return "C Data"; }

QString CDataEncoder::encode(const QByteArray& data) {
  QString res = "unsigned char data[] = {";
  int lineLen = 1000;  // fold after first line
  for (auto byte : data) {
    auto byteStr =
        QString::asprintf("0x%02x,", static_cast<unsigned char>(byte));
    lineLen += byteStr.length() + 1;
    if (lineLen > 80) {
      byteStr = "\n" + QString(indentation, ' ') + byteStr;
      lineLen = byteStr.length() - 1;  // - 1 for newline
      res += byteStr;
    } else {
      res += " " + byteStr;
    }
  }
  res += "\n};\n";
  return res;
}

}  // namespace encoders
}  // namespace util
}  // namespace veles
