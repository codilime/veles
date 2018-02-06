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
#include "ui/dialogs/optionsdialog.h"

#include <QMessageBox>
#include <QPushButton>

#include "ui/veles_mainwindow.h"
#include "ui/velesapplication.h"
#include "ui_optionsdialog.h"
#include "util/settings/hexedit.h"
#include "util/settings/theme.h"
#include "util/settings/visualization.h"
#include "visualization/trigram.h"

namespace veles {
namespace ui {

OptionsDialog::OptionsDialog(QWidget* parent)
    : QDialog(parent), ui(new Ui::OptionsDialog) {
  ui->setupUi(this);
  ui->colorsBox->addItems(util::settings::theme::availableThemes());

  connect(ui->hexColumnsAutoCheckBox, &QCheckBox::stateChanged,
          [this](int state) {
            ui->hexColumnsSpinBox->setEnabled(state != Qt::Checked);
          });
  connect(ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked,
          this, &OptionsDialog::applyChanges);
  connect(ui->resetToDefaultsButton, &QPushButton::clicked, this,
          &OptionsDialog::resetToDefaults);

  // Initialize color pickers.
  auto color_begin = util::settings::visualization::colorBegin();
  auto color_end = util::settings::visualization::colorEnd();
  color_3d_begin_button_ = new ColorPickerButton(color_begin, this);
  color_3d_end_button_ = new ColorPickerButton(color_end, this);
  ui->visualizationGradientLayout->addWidget(color_3d_begin_button_);
  ui->visualizationGradientLayout->addWidget(color_3d_end_button_);
}

OptionsDialog::~OptionsDialog() { delete ui; }

void OptionsDialog::show() {
  ui->colorsBox->setCurrentText(util::settings::theme::currentTheme());
  Qt::CheckState checkState = Qt::Unchecked;
  if (util::settings::hexedit::resizeColumnsToWindowWidth()) {
    checkState = Qt::Checked;
  }
  ui->hexColumnsAutoCheckBox->setCheckState(checkState);
  ui->hexColumnsSpinBox->setValue(util::settings::hexedit::columnsNumber());
  ui->hexColumnsSpinBox->setEnabled(checkState != Qt::Checked);

  color_3d_begin_button_->setColor(util::settings::visualization::colorBegin());
  color_3d_end_button_->setColor(util::settings::visualization::colorEnd());

  QWidget::show();
}

void OptionsDialog::resetToDefaults() {
  Qt::CheckState resposive_columns_checked =
      util::settings::hexedit::getDefaultResizeColumnsToWindowWidth()
          ? Qt::Checked
          : Qt::Unchecked;

  ui->hexColumnsAutoCheckBox->setCheckState(resposive_columns_checked);
  ui->hexColumnsSpinBox->setValue(
      util::settings::hexedit::getDefaultColumnsNumber());
  ui->hexColumnsSpinBox->setEnabled(resposive_columns_checked != Qt::Checked);

  ui->colorsBox->setCurrentText(util::settings::theme::getDefaultTheme());

  color_3d_begin_button_->setColor(
      util::settings::visualization::getDefaultColorBegin());
  color_3d_end_button_->setColor(
      util::settings::visualization::getDefaultColorEnd());
}

void OptionsDialog::applyChanges() {
  bool restart_needed = false;
  QString newTheme = ui->colorsBox->currentText();
  if (newTheme != util::settings::theme::currentTheme()) {
    veles::util::settings::theme::setCurrentTheme(newTheme);
    restart_needed = true;
  }

  util::settings::hexedit::setResizeColumnsToWindowWidth(
      ui->hexColumnsAutoCheckBox->checkState() == Qt::Checked);
  util::settings::hexedit::setColumnsNumber(ui->hexColumnsSpinBox->value());

  util::settings::visualization::setColorBegin(
      color_3d_begin_button_->getColor());
  util::settings::visualization::setColorEnd(color_3d_end_button_->getColor());

  emit VelesApplication::instance()->settingsChanged();

  if (restart_needed) {
    QMessageBox::about(
        this, tr("Options change"),
        tr("Some changes will only take effect after application restart"));
  }
}

void OptionsDialog::accept() {
  applyChanges();
  emit accepted();
  QDialog::hide();
}

}  // namespace ui
}  // namespace veles
