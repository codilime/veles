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
#ifndef SEARCHDIALOG_H
#define SEARCHDIALOG_H

#include <QDialog>
#include <QtCore>
#include "include/ui/hexedit.h"
#include "data/bindata.h"

namespace Ui {
class SearchDialog;
}

namespace veles {
namespace ui {

class SearchDialog : public QDialog {
  Q_OBJECT
 public:
  explicit SearchDialog(HexEdit *hexEdit, QWidget *parent = 0);
  ~SearchDialog();
  qint64 findNext();
  Ui::SearchDialog *ui;

 private slots:
  void on_pbFind_clicked();
  void on_pbReplace_clicked();
  void on_pbReplaceAll_clicked();

 private:
  data::BinData getContent(int comboIndex, const QString &input);
  bool isHexStr(QString hexStr);
  qint64 replaceOccurrence(qint64 idx, const data::BinData &replaceBa);
  qint64 findIndex(qint64 startSearchPos);
  qint64 lastIndexOf(const data::BinData& pattern, qint64 startPos);
  qint64 indexOf(const data::BinData& pattern, qint64 startPos);
  void replace(qint64 pos, qint64 len, const data::BinData &data);

  HexEdit *_hexEdit;
  data::BinData _findBa;
  qint64 _lastFoundPos;
  qint64 _lastFoundSize;
};

}  // namespace ui
}  // namespace veles

#endif  // SEARCHDIALOG_H
