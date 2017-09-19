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
#include "util/settings/hexedit.h"

#include <QColor>
#include <QSettings>

namespace veles {
namespace util {
namespace settings {
namespace hexedit {

int columnsNumber() {
  QSettings settings;
  return settings.value("hexedit.columnsNumber", 16).toInt();
}

void setColumnsNumber(int number) {
  QSettings settings;
  settings.setValue("hexedit.columnsNumber", number);
}

bool resizeColumnsToWindowWidth() {
  QSettings settings;
  return settings.value("hexedit.resizeColumnsToWindowWidth", false).toBool();
}

void setResizeColumnsToWindowWidth(bool on) {
  QSettings settings;
  settings.setValue("hexedit.resizeColumnsToWindowWidth", on);
}

QColor colorOfBytes() {
  QSettings settings;
  uint v = settings.value("hexedit.colorOfBytes", QRgb(4280504044)).toUInt();
  return QColor(QRgb(v));
}

void setColorOfBytes(const QColor& color) {
  QSettings settings;
  settings.setValue("hexedit.colorOfBytes", color.rgb());
}

}  // namespace hexedit
}  // namespace settings
}  // namespace util
}  // namespace veles
