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
#ifndef SLICE_H
#define SLICE_H

#include <stdint.h>

#include <QColor>
#include <QString>

namespace veles {
namespace ui {

struct Slice {
  Slice(uint64_t begin, uint64_t end, QString name, QString comment,
        QColor color)
      : begin(begin), end(end), comment(comment), color(color) {}

  uint64_t begin, end;
  QString name;
  QString comment;
  QColor color;
};

class SliceStorage {};

}  // namespace ui
}  // namespace veles

#endif  // SLICE_H
