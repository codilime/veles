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

#include <QColorDialog>
#include <QDialog>
#include <QtCore>

#include "ui/color_picker_button.h"

namespace Ui {
class OptionsDialog;
}

namespace veles {
namespace ui {

class OptionsDialog : public QDialog {
  Q_OBJECT

 public:
  explicit OptionsDialog(QWidget* parent = nullptr);
  ~OptionsDialog() override;
  Ui::OptionsDialog* ui;
  void show();
  void applyChanges();
  void updateColorButton();

 public slots:
  void accept() override;

 private:
  ColorPickerButton* color_3d_begin_button_;
  ColorPickerButton* color_3d_end_button_;
  QColorDialog* color_dialog;
  QColorDialog* color_dialog_;
};

}  // namespace ui
}  // namespace veles
