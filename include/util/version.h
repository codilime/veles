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
#ifndef VELES_UTIL_VERSION_H
#define VELES_UTIL_VERSION_H

#include <QString>

namespace veles {
namespace util {
namespace version {

extern const unsigned year;
extern const unsigned month;
extern const unsigned release;
extern const unsigned patch;
extern const QString codename;
extern const QString string;

}
}
}

#endif
