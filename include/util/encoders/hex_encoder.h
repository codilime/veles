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
#pragma once

#include <QByteArray>
#include <QString>

#include "util/encoders/idecoder.h"
#include "util/encoders/iencoder.h"

namespace veles {
namespace util {
namespace encoders {

class HexEncoder : public IEncoder, public IDecoder {
 public:
  QString encode(const QByteArray& data) override;
  using IEncoder::encode;
  QByteArray decode(const QString& str) override;
  QString encodingDisplayName() override;
  QString decodingDisplayName() override;
};

}  // namespace encoders
}  // namespace util
}  // namespace veles
