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
#include "include/ui/gotoaddressdialog.h"
#include "ui_gotoaddressdialog.h"

#include <QMessageBox>

namespace veles {
namespace ui {

GoToAddressDialog::GoToAddressDialog(QWidget *parent)
    : QDialog(parent),
      ui_(new Ui::GoToAddressDialog) {
  ui_->setupUi(this);

  connect(ui_->addrFormat, SIGNAL(currentIndexChanged(QString)), this, SLOT(formatChanged(QString)));

  formatChanged(ui_->addrFormat->currentText());
}

GoToAddressDialog::~GoToAddressDialog() { delete ui_; }

void GoToAddressDialog::setRange(qint64 start, qint64 end) {
    ui_->addrValue->setMinimum(start);
    ui_->addrValue->setMaximum(end);
    ui_->addrValue->setValue(start);
}

void GoToAddressDialog::formatChanged(QString format) {
  if (format == "Hex") {
    ui_->addrValue->setDisplayIntegerBase(16);
  } else if (format == "Dec") {
    ui_->addrValue->setDisplayIntegerBase(10);
  }
}

qint64 GoToAddressDialog::address() {
  return ui_->addrValue->value();
}

}  // namespace ui
}  // namespace veles
