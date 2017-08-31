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
#pragma once

#include <QDialog>
#include <QMessageBox>
#include <QtCore>

#include "data/bindata.h"
#include "include/ui/hexedit.h"

namespace Ui {
class SearchDialog;
}

namespace veles {
namespace ui {

class SearchDialog : public QDialog {
  Q_OBJECT
 public:
  explicit SearchDialog(HexEdit* hexEdit, QWidget* parent = nullptr);
  ~SearchDialog() override;
  qint64 findNext(bool include_overlapping = true, bool interactive = true);
  Ui::SearchDialog* ui;

 signals:
  void enableFindNext(bool enable);

 protected:
  void showEvent(QShowEvent* event) override;

 private slots:
  void on_pbFind_clicked();
  void on_pbReplace_clicked();
  void on_pbReplaceAll_clicked();
  void findTextChanged(const QString& text);
  void replaceTextChanged(const QString& text);

 private:
  data::BinData getContent(int comboIndex, const QString& input);
  bool isHexStr(const QString& hexStr);
  void replaceOccurrence(qint64 idx, const data::BinData& replaceBa);
  qint64 findIndex(qint64 startSearchPos);
  qint64 lastIndexOf(const data::BinData& pattern, qint64 startPos);
  qint64 indexOf(const data::BinData& pattern, qint64 startPos);
  void replace(qint64 pos, const data::BinData& data);
  void enableReplace(const QString& find, const QString& replace);

  HexEdit* _hexEdit;
  data::BinData _findBa;
  qint64 _lastFoundPos;
  qint64 _lastFoundSize;
  QMessageBox* message_box_not_found_;
  QMessageBox* message_box_not_valid_hex_string_;
};

}  // namespace ui
}  // namespace veles
