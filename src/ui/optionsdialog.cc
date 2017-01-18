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
#include "include/ui/optionsdialog.h"
#include <QMessageBox>
#include "ui_optionsdialog.h"
#include "util/settings/hexedit.h"
#include "util/settings/network.h"
#include "util/settings/theme.h"

namespace veles {
namespace ui {

OptionsDialog::OptionsDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::OptionsDialog) {
  ui->setupUi(this);
  ui->colorsBox->addItems(util::settings::theme::availableIds());
  QString ipRange = "(?:[0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])";
  QRegExp ipRegex("^" + ipRange + "\\." + ipRange +
                  "\\." + ipRange + "\\." + ipRange + "$");
  QRegExpValidator *ipValidator = new QRegExpValidator(ipRegex, ui->ipAddress);
  ui->ipAddress->setValidator(ipValidator);

  connect(ui->hexColumnsAutoCheckBox, &QCheckBox::stateChanged,
          [this](int state) {
            ui->hexColumnsSpinBox->setEnabled(state != Qt::Checked);
          });
  connect(ui->networkEnabled, &QCheckBox::stateChanged,
          [this](int state) {
            ui->portBox->setEnabled(state == Qt::Checked);
            ui->ipAddress->setEnabled(state == Qt::Checked);
          });
}

OptionsDialog::~OptionsDialog() { delete ui; }

void OptionsDialog::show() {
  ui->colorsBox->setCurrentText(util::settings::theme::currentId());
  Qt::CheckState checkState = Qt::Unchecked;
  if (util::settings::hexedit::resizeColumnsToWindowWidth()) {
    checkState = Qt::Checked;
  }
  ui->hexColumnsAutoCheckBox->setCheckState(checkState);
  ui->hexColumnsSpinBox->setValue(util::settings::hexedit::columnsNumber());
  ui->hexColumnsSpinBox->setEnabled(checkState != Qt::Checked);

  ui->networkEnabled->setChecked(util::settings::network::enabled());
  ui->ipAddress->setText(util::settings::network::ipAddress());
  ui->ipAddress->setEnabled(ui->networkEnabled->isChecked());
  ui->portBox->setValue(util::settings::network::port());
  ui->portBox->setEnabled(ui->networkEnabled->isChecked());

  QWidget::show();
}

void OptionsDialog::accept() {
  bool restart_needed = false;
  QString newTheme = ui->colorsBox->currentText();
  if (newTheme != util::settings::theme::currentId()) {
    veles::util::settings::theme::setCurrentId(newTheme);
    restart_needed = true;
  }

  util::settings::hexedit::setResizeColumnsToWindowWidth(
      ui->hexColumnsAutoCheckBox->checkState() == Qt::Checked);
  util::settings::hexedit::setColumnsNumber(ui->hexColumnsSpinBox->value());

  bool new_network = ui->networkEnabled->isChecked();
  if (new_network != util::settings::network::enabled()) {
    util::settings::network::setEnabled(new_network);
    restart_needed = true;
  }

  QString new_ip = ui->ipAddress->text();
  if (new_ip != util::settings::network::ipAddress()) {
    util::settings::network::setIpAddress(new_ip);
    restart_needed = true;
  }

  uint32_t new_port = ui->portBox->value();
  if (new_port != util::settings::network::port()) {
    util::settings::network::setPort(new_port);
    restart_needed = true;
  }

  if (restart_needed) {
    QMessageBox::about(
        this, tr("Options change"),
        tr("Some changes will only take effect after application restart"));
  }

  emit accepted();
  QDialog::hide();
}

}  // namespace ui
}  // namespace veles
