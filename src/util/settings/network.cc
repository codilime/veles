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
#include <QSettings>

#include "util/settings/network.h"



namespace veles {
namespace util {
namespace settings {
namespace network {

bool enabled() {
  QSettings settings;
  return settings.value("network.enabled", false).toBool();
}

void setEnabled(bool enabled) {
  QSettings settings;
  settings.setValue("network.enabled", enabled);
}

uint32_t port() {
  QSettings settings;
  return settings.value("network.port", 3135).toUInt();
}

void setPort(uint32_t port) {
  QSettings settings;
  settings.setValue("network.port", port);
}

QString ipAddress() {
  QSettings settings;
  return settings.value("network.ip", "127.0.0.1").toString();
}

void setIpAddress(QString addr) {
  QSettings settings;
  settings.setValue("network.ip", addr);
}

}  // namespace network
}  // namespace settings
}  // namespace util
}  // namespace veles
