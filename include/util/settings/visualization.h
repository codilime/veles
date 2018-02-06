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

#include <QColor>

namespace veles {
namespace util {
namespace settings {
namespace visualization {

bool showCaptions();
void setShowCaptions(bool showCaptions);

bool autoBrightness();
void setAutoBrightness(bool autoBrightness);

int brightness();
void setBrightness(int brightness);

const QColor& getDefaultColorBegin();
QColor colorBegin();
void setColorBegin(const QColor& color);

const QColor& getDefaultColorEnd();
QColor colorEnd();
void setColorEnd(const QColor& color);

}  // namespace visualization
}  // namespace settings
}  // namespace util
}  // namespace veles
