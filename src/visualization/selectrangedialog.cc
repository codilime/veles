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
#include <cassert>
#include <functional>

#include <QComboBox>

#include "include/visualization/selectrangedialog.h"
#include "ui_selectrangedialog.h"

namespace veles {
namespace visualization {

SelectRangeDialog::SelectRangeDialog(QWidget* parent)
    : QDialog(parent), ui(new Ui::SelectRangeDialog) {
  ui->setupUi(this);

  connect(ui->start_address_spinbox_, &veles::ui::SpinBox::valueChanged, this,
          &SelectRangeDialog::addressChanged);
  connect(ui->end_address_spinbox_, &veles::ui::SpinBox::valueChanged, this,
          &SelectRangeDialog::addressChanged);

  // More overloaded signal bs
  auto currentIndexChangedPtr =
      static_cast<void (QComboBox::*)(const QString&)>(
          &QComboBox::currentIndexChanged);

  connect(ui->start_format_dropdown_, currentIndexChangedPtr,
          std::bind(&SelectRangeDialog::numberBaseChanged, this,
                    ui->start_address_spinbox_, std::placeholders::_1));
  connect(ui->end_format_dropdown_, currentIndexChangedPtr,
          std::bind(&SelectRangeDialog::numberBaseChanged, this,
                    ui->end_address_spinbox_, std::placeholders::_1));
}

SelectRangeDialog::~SelectRangeDialog() { delete ui; }

void SelectRangeDialog::setRange(size_t min_address, size_t max_address) {
  ui->start_address_spinbox_->setValue(min_address);
  ui->start_address_spinbox_->setMinimum(min_address);
  ui->end_address_spinbox_->setMaximum(max_address);
  ui->end_address_spinbox_->setValue(max_address);
  setAddressRanges();
}

void SelectRangeDialog::resetNumberFormat() {
  ui->start_format_dropdown_->setCurrentIndex(0);
  ui->end_format_dropdown_->setCurrentIndex(0);
}

size_t SelectRangeDialog::getStartAddress() {
  int sa = ui->start_address_spinbox_->value();
  assert(sa >= 0);
  return static_cast<size_t>(sa);
}

size_t SelectRangeDialog::getEndAddress() {
  int ea = ui->end_address_spinbox_->value();
  assert(ea >= 0);
  return static_cast<size_t>(ea);
}

void SelectRangeDialog::setAddressRanges() {
  ui->end_address_spinbox_->setMinimum(ui->start_address_spinbox_->value() + 1);
  ui->start_address_spinbox_->setMaximum(ui->end_address_spinbox_->value() - 1);
}

void SelectRangeDialog::addressChanged(int /*new_address*/) {
  setAddressRanges();
}

void SelectRangeDialog::numberBaseChanged(veles::ui::SpinBox* box,
                                          const QString& base) {
  if (base == "Hex") {
    box->setDisplayIntegerBase(16);
  } else {
    box->setDisplayIntegerBase(10);
  }
}

}  // namespace visualization
}  // namespace veles
