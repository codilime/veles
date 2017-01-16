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
#ifndef VELES_UI_GOTOADDRESSDIALOG_H
#define VELES_UI_GOTOADDRESSDIALOG_H

#include <QDialog>

namespace Ui {
class GoToAddressDialog;
}

namespace veles {
namespace ui {

class GoToAddressDialog : public QDialog {
  Q_OBJECT
 public:
  explicit GoToAddressDialog(QWidget *parent = 0);
  ~GoToAddressDialog();
  void setRange(qint64 start, qint64 end);
  qint64 address();

 private:
  Ui::GoToAddressDialog *ui_;
 private slots:
  void formatChanged(QString);

};

}  // namespace ui
}  // namespace veles

#endif  // VELES_UI_GOTOADDRESSDIALOG_H
