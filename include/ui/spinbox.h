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
#ifndef UI_SPINBOX_H
#define UI_SPINBOX_H

#include <QAbstractSpinBox>

#include "include/ui/spinboxvalidator.h"

namespace veles {
namespace ui {

class SpinBox : public QAbstractSpinBox {
  Q_OBJECT
 public:
  SpinBox(QWidget* parent = nullptr);

  uint64_t value() const;
  void stepBy(int steps) override;

  void setDisplayIntegerBase(int base);
  void setMaximum(uint64_t maximum);
  void setMinimum(uint64_t minimum);
  void setRange(uint64_t minimum, uint64_t maximum);

  void setSingleStep(uint64_t val);

 public slots:
  void setValue(uint64_t value);

 signals:
  void valueChanged(uint64_t i);

 protected:
  QAbstractSpinBox::StepEnabled stepEnabled() const override;

 private:
  int base_;
  uint64_t value_;
  uint64_t single_step_;
  SpinBoxValidator *validator_;
};

}  // namespace ui
}  // namespace veles

#endif // UI_SPINBOX_H
