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
#pragma once

#include <QDialog>
#include <QString>

namespace Ui {
class SamplingMethodDialog;
}

namespace veles {
namespace visualization {

class SamplingMethodDialog : public QDialog {
  Q_OBJECT

 public:
  explicit SamplingMethodDialog(QWidget* parent = nullptr);
  ~SamplingMethodDialog() override;

 signals:
  void samplingMethodChanged(QString sampling_method);
  void sampleSizeChanged(int size);

 public slots:
  void setMaximumSampleSize(int size);
  void samplingMethodToggled(bool uniform);
  void setSampleSize(int size);

 private:
  Ui::SamplingMethodDialog* ui;
};

}  // namespace visualization
}  // namespace veles
