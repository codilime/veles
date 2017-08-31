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

#include <QDialog>
#include <QString>

#include <ui/spinbox.h>

namespace Ui {
class SelectRangeDialog;
}

namespace veles {
namespace visualization {

class SelectRangeDialog : public QDialog {
  Q_OBJECT

 public:
  explicit SelectRangeDialog(QWidget* parent = nullptr);
  ~SelectRangeDialog() override;

  void setRange(size_t min_address, size_t max_address);
  void resetNumberFormat();
  size_t getStartAddress();
  size_t getEndAddress();

 private slots:
  void addressChanged(int new_address);
  void numberBaseChanged(veles::ui::SpinBox* box, const QString& base);

 private:
  void setAddressRanges();

  Ui::SelectRangeDialog* ui;
};

}  // namespace visualization
}  // namespace veles
