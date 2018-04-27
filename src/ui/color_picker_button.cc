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
#include "ui/color_picker_button.h"

namespace veles {
namespace ui {

ColorPickerButton::ColorPickerButton(const QColor& initial_color,
                                     QWidget* parent)
    : QPushButton(parent) {
  setFlat(true);
  setAutoFillBackground(true);
  setColor(initial_color);
  color_dialog_ = new QColorDialog(this);
  connect(this, &ColorPickerButton::clicked, [this]() {
    color_dialog_->setCurrentColor(color_);
    color_dialog_->show();
  });
  connect(color_dialog_, &QColorDialog::colorSelected, this,
          &ColorPickerButton::setColor);
}

QColor ColorPickerButton::getColor() const { return color_; }

void ColorPickerButton::setColor(const QColor& color) {
  color_ = color;
  QPalette pal = palette();
  pal.setColor(QPalette::Button, color_);
  setPalette(pal);
  update();
}

}  // namespace ui
}  // namespace veles
