/*
 * Copyright 2017 CodiLime
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
#ifndef VELES_UTIL_SETTINGS_NETWORK_H
#define VELES_UTIL_SETTINGS_NETWORK_H
#include <QString>

namespace veles {
namespace util {
namespace settings {
namespace network {

uint32_t port ();
void setPort (uint32_t port);
QString ipAddress ();
void setIpAddress (QString addr);

}  // namespace network
}  // namespace settings
}  // namespace util
}  // namespace veles

#endif // VELES_UTIL_SETTINGS_NETWORK_H
