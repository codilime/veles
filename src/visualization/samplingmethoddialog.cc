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
#include "visualization/samplingmethoddialog.h"

#include <limits>

#include <QComboBox>

#include "ui_samplingmethoddialog.h"

namespace veles {
namespace visualization {

SamplingMethodDialog::SamplingMethodDialog(QWidget* parent)
    : QDialog(parent), ui(new Ui::SamplingMethodDialog) {
  ui->setupUi(this);

  connect(ui->sampling_method_uniform, &QRadioButton::toggled, ui->sample_size,
          &QWidget::setEnabled);
  connect(ui->sampling_method_uniform, &QRadioButton::toggled, this,
          &SamplingMethodDialog::samplingMethodToggled);
  connect(
      ui->sample_size,
      static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this,
      [this](int new_kb_size) { this->sampleSizeChanged(new_kb_size * 1024); });
}

SamplingMethodDialog::~SamplingMethodDialog() { delete ui; }

// Qt controls operate on ints, so unfortunately we can't represent all the
// values of size_t in the UI until we fix that controls.
static int SaturatedCastToInt(size_t val) {
  return static_cast<int>(
      std::min<size_t>(val, std::numeric_limits<int>::max()));
}

void SamplingMethodDialog::setMaximumSampleSize(size_t size) {
  ui->sample_size->setMaximum(SaturatedCastToInt(size / 1024));
}

void SamplingMethodDialog::samplingMethodToggled(bool uniform) {
  emit samplingMethodChanged(uniform ? "Uniform random sampling"
                                     : "No sampling");
}

void SamplingMethodDialog::setSampleSize(size_t size) {
  ui->sample_size->setValue(SaturatedCastToInt(size));
}

}  // namespace visualization
}  // namespace veles
