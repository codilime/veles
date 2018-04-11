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
#include "util/settings/visualization.h"

#include <QSettings>

#include "visualization/trigram.h"

namespace veles {
namespace util {
namespace settings {
namespace visualization {

bool showCaptions() {
  return QSettings().value("visualization.show_captions", false).toBool();
}

void setShowCaptions(bool showCaptions) {
  QSettings().setValue("visualization.show_captions", showCaptions);
}

bool autoBrightness() {
  return QSettings().value("visualization.auto_brightness", true).toBool();
}

void setAutoBrightness(bool autoBrightness) {
  QSettings().setValue("visualization.auto_brightness", autoBrightness);
}

int brightness() {
  auto defaultVal = (veles::visualization::k_maximum_brightness +
                     veles::visualization::k_minimum_brightness) /
                    2;
  return QSettings().value("visualization.brightness", defaultVal).toInt();
}

void setBrightness(int brightness) {
  QSettings().setValue("visualization.brightness", brightness);
}

const QColor& defaultColorBegin() {
  static const QColor color_begin{255, 127, 0};
  return color_begin;
}

QColor colorBegin() {
  auto v = QSettings().value("visualization.color_begin", defaultColorBegin());
  return v.value<QColor>();
}

void setColorBegin(const QColor& color) {
  QSettings().setValue("visualization.color_begin", color);
}

const QColor& defaultColorEnd() {
  static const QColor color_end{0, 127, 255};
  return color_end;
}

QColor colorEnd() {
  auto v = QSettings().value("visualization.color_end", defaultColorEnd());
  return v.value<QColor>();
}

void setColorEnd(const QColor& color) {
  QSettings().setValue("visualization.color_end", color);
}

}  // namespace visualization
}  // namespace settings
}  // namespace util
}  // namespace veles
