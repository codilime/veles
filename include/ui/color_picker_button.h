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
#pragma once

#include <QColor>
#include <QColorDialog>
#include <QPushButton>

namespace veles {
namespace ui {

class ColorPickerButton : public QPushButton {
  Q_OBJECT

 public:
  explicit ColorPickerButton(const QColor& initial_color,
                             QWidget* parent = nullptr);
  QColor getColor() const;

 public slots:
  void setColor(const QColor& color);

 private:
  QColor color_;
  QColorDialog* color_dialog_;
};

}  // namespace ui
}  // namespace veles
