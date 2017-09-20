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
#include "ui/spinbox.h"

#include <QLineEdit>

#include "ui/spinboxvalidator.h"

namespace veles {
namespace ui {

SpinBox::SpinBox(QWidget* parent)
    : QAbstractSpinBox(parent), base_(10), single_step_(1) {
  validator_ = new SpinBoxValidator(base_, lineEdit());
  lineEdit()->setValidator(validator_);

  connect(lineEdit(), &QLineEdit::textChanged, [this](const QString& text) {
    bool ok;
    uint64_t new_value = text.toULongLong(&ok, base_);
    if (value_ != new_value) {
      value_ = new_value;
      emit valueChanged(value_);
    }
  });
}

uint64_t SpinBox::value() const { return value_; }

void SpinBox::setValue(uint64_t value) {
  int not_used;
  QString text = QString::number(value, base_);

  if (validator_->validate(text, not_used) == SpinBoxValidator::Acceptable) {
    lineEdit()->setText(text);
  } else {
    validator_->fixup(text);
    lineEdit()->setText(text);
  }
}

QAbstractSpinBox::StepEnabled SpinBox::stepEnabled() const {
  StepEnabled ret = StepUpEnabled | StepDownEnabled;
  return ret;
}
void SpinBox::stepBy(int steps) {
  int64_t new_steps = steps * single_step_;
  if (new_steps > 0 && value_ + new_steps < value_) {
    setValue(UINT64_MAX);
  } else if (new_steps < 0 && value_ + new_steps > value_) {
    setValue(0);
  } else {
    setValue(value_ + new_steps);
  }
}

void SpinBox::setDisplayIntegerBase(int base) {
  base_ = base;
  validator_->setBase(base_);
  setValue(value_);
}

void SpinBox::setMaximum(uint64_t maximum) { validator_->setTop(maximum); }

void SpinBox::setMinimum(uint64_t minimum) { validator_->setBottom(minimum); }

void SpinBox::setRange(uint64_t minimum, uint64_t maximum) {
  validator_->setRange(minimum, maximum);
}

void SpinBox::setSingleStep(uint64_t val) { single_step_ = val; }

}  // namespace ui
}  // namespace veles
