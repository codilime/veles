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
#include "util/version.h"

namespace veles {
namespace util {
namespace version {

const unsigned year = 2017;
const unsigned month = 6;
const unsigned release = 0;
const unsigned patch = 1;
const QString codename = "SWOND";
const QString string =
    patch > 0
        ? QString("%1.%2.%3.%4.%5")
              .arg(QString("%1").arg(year),
                   QString("%1").arg(month, 2, 10, QChar('0')),
                   QString("%1").arg(release), QString("%1").arg(patch),
                   codename)
        : QString("%1.%2.%3.%4")
              .arg(QString("%1").arg(year),
                   QString("%1").arg(month, 2, 10, QChar('0')),
                   QString("%1").arg(release), codename);

}  // namespace version
}  // namespace util
}  // namespace veles
