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

#include "util/encoders/idecoder.h"
#include "util/encoders/iencoder.h"

#include <QString>
#include <QStringList>

namespace veles {
namespace util {
namespace encoders {

class EncodersFactory {
 public:
  static QStringList keys();
  static IEncoder* createEncoder(const QString& id);
  static IDecoder* createDecoder(const QString& id);
};

}  // namespace encoders
}  // namespace util
}  // namespace veles
