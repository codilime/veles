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
#include "util/encoders/factory.h"

#include "util/encoders/base64_encoder.h"
#include "util/encoders/hex_encoder.h"

namespace veles {
namespace util {
namespace encoders {

QStringList EncodersFactory::keys() { return {"base64", "hex"}; }

Encoder *EncodersFactory::create(const QString &id) {
  if (id == "base64") {
    return new Base64Encoder();
  } else if (id == "hex") {
    return new HexEncoder();
  }

  return nullptr;
}

}  // namespace encoders
}  // namespace util
}  // namespace veles
