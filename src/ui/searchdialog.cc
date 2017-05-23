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
#include "include/ui/searchdialog.h"
#include "ui_searchdialog.h"

namespace veles {
namespace ui {

SearchDialog::SearchDialog(HexEdit *hexEdit, QWidget *parent)
    : QDialog(parent),
      ui(new Ui::SearchDialog),
      _lastFoundPos(-1),
      _lastFoundSize(0) {
  ui->setupUi(this);
  _hexEdit = hexEdit;
  message_box_not_found_ = new QMessageBox(this);
  message_box_not_found_->setWindowTitle(tr("HexEdit"));
  message_box_not_found_->setText(
      tr("Not found, wrapping to the beginning."));
  message_box_not_found_->setStandardButtons(QMessageBox::Close);
  message_box_not_found_->setDefaultButton(QMessageBox::Close);
  message_box_not_found_->setIcon(QMessageBox::Information);

  message_box_not_valid_hex_string_= new QMessageBox(this);
  message_box_not_valid_hex_string_->setWindowTitle(tr("HexEdit"));
  message_box_not_valid_hex_string_->setStandardButtons(QMessageBox::Close);
  message_box_not_valid_hex_string_->setDefaultButton(QMessageBox::Close);
}

SearchDialog::~SearchDialog() { delete ui; }

qint64 SearchDialog::indexOf(const data::BinData &pattern, qint64 startPos) {
  // TODO: implement this as BinData method or as separate util
  auto data = _hexEdit->dataModel()->binData(_hexEdit->nodeID());
  if (startPos == -1) {
    startPos = 0;
  }
  qint64 index = startPos;
  while (index + pattern.size() <= data->size()) {
    size_t numberOfMatches = 0;
    while (numberOfMatches < pattern.size() &&
           numberOfMatches + index < data->size() &&
           pattern.element64(numberOfMatches) ==
               data->element64(index + numberOfMatches)) {

      ++numberOfMatches;
    }

    if (numberOfMatches == pattern.size()) {
      return index;
    }

    index++;
  }

  return -1;
}

qint64 SearchDialog::lastIndexOf(const data::BinData &pattern,
                                 qint64 startPos) {
  // TODO: implement this as BinData method or as separate util
  auto data = _hexEdit->dataModel()->binData(_hexEdit->nodeID());
  if (startPos == -1) {
    startPos = data->size();
  }
  qint64 index = startPos - 1;
  while (index > 0) {
    size_t numberOfMatches = 0;
    while (numberOfMatches < pattern.size() &&
           numberOfMatches + index < data->size() &&
           pattern.element64(numberOfMatches) ==
               data->element64(index + numberOfMatches)) {
      ++numberOfMatches;
    }

    if (numberOfMatches == pattern.size()) {
      return index;
    }

    index--;
  }

  return -1;
}

void SearchDialog::replace(qint64 pos, qint64 len, const data::BinData &data) {
  // TODO: implement this
}

qint64 SearchDialog::findNext() {
  emit enableFindNext(false);

  _findBa =
      getContent(ui->cbFindFormat->currentIndex(), ui->cbFind->currentText());

  if (_findBa.size() == 0) {
    return -1;
  }

  bool backwards = ui->cbBackwards->isChecked();

  qint64 startSearchPos = _lastFoundPos;

  if (!backwards) {
    startSearchPos += _lastFoundSize;
  }

  qint64 idx = -1;
  if (backwards) {
    idx = lastIndexOf(_findBa, startSearchPos);
  } else {
    idx = indexOf(_findBa, startSearchPos);
  }

  if (idx >= 0) {
    _hexEdit->setSelection(idx, _findBa.size(), true);
    _lastFoundPos = idx;
    _lastFoundSize = _findBa.size();
    emit enableFindNext(true);
  } else {
    _lastFoundPos = -1;
    _lastFoundSize = 0;
    _hexEdit->setSelection(0, 0, false);
    message_box_not_found_->show();
  }

  return idx;
}

void SearchDialog::showEvent(QShowEvent* event) {
  ui->cbFind->setFocus(Qt::OtherFocusReason);
}

void SearchDialog::on_pbFind_clicked() { findNext(); }

void SearchDialog::on_pbReplace_clicked() {
  _findBa =
      getContent(ui->cbFindFormat->currentIndex(), ui->cbFind->currentText());


  if (indexOf(_findBa, _lastFoundPos) == _lastFoundPos) {
    auto replaceData = getContent(ui->cbReplaceFormat->currentIndex(),
                                      ui->cbReplace->currentText());
    replaceOccurrence(_lastFoundPos, replaceData);
  }

  findNext();
}

void SearchDialog::on_pbReplaceAll_clicked() {
  _lastFoundPos = -1;
  int replaceCounter = 0;
  int idx = 0;
  int goOn = QMessageBox::Yes;
  while ((idx >= 0) && (goOn == QMessageBox::Yes)) {
    idx = findNext();
    if (idx >= 0) {
      data::BinData replaceBa = getContent(ui->cbReplaceFormat->currentIndex(),
                                        ui->cbReplace->currentText());
      int result = replaceOccurrence(idx, replaceBa);

      if (result == QMessageBox::Yes) replaceCounter += 1;

      if (result == QMessageBox::Cancel) goOn = result;
    }
  }

  if (replaceCounter > 0)
    QMessageBox::information(
        this, tr("HexEdit"),
        QString(tr("%1 occurrences replaced.")).arg(replaceCounter));
}

bool SearchDialog::isHexStr(QString hexStr) {
  auto hexCharsPerByte = _hexEdit->dataModel()->binData(
      _hexEdit->nodeID())->width() / 4;
  QRegExp hexMatcher(QString("^(([0-9A-F]{%1})|\\s)*$").arg(hexCharsPerByte),
      Qt::CaseInsensitive);
  return hexMatcher.exactMatch(hexStr);
}

data::BinData SearchDialog::getContent(int comboIndex, const QString &input) {
  std::vector<uint64_t> findBa;
  int hexCharsPerByte = _hexEdit->dataModel()->binData(
      _hexEdit->nodeID())->width() / 4;
  switch (comboIndex) {
    case 0:  // hex
      if (!isHexStr(input)) {
        message_box_not_valid_hex_string_->setText(
            QString(tr("\"%1\" is not valid hex string.")).arg(input));
        message_box_not_valid_hex_string_->show();
      } else {

        QString currentByte;
        for (QChar hexChar: input.toLatin1()) {
          if (hexChar.isSpace()) {
              continue;
          }
          currentByte += hexChar;

          if (currentByte.length() == hexCharsPerByte) {
              findBa.push_back(currentByte.toInt(nullptr, 16));
              currentByte = "";
          }
        }
      }
      break;
    case 1:  // text
      for (auto byte: input.toUtf8()) {
        findBa.push_back(byte);
      }
      break;
  }

  data::BinData result(hexCharsPerByte * 4, findBa.size());
  size_t pos = 0;
  for (auto x : findBa)
    result.setElement64(pos++, x);
  return result;
}

qint64 SearchDialog::replaceOccurrence(qint64 idx,
                                       const data::BinData &replaceBa) {
  int result = QMessageBox::Yes;
  if (ui->cbPrompt->isChecked()) {
    result = QMessageBox::question(
        this, tr("HexEdit"), tr("Replace occurrence?"),
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

    if (result == QMessageBox::Yes) {
       replace(idx, replaceBa.size(), replaceBa);
      _hexEdit->update();
    }
  } else {
    replace(idx, _findBa.size(), replaceBa);
  }
  return result;
}

}  // namespace ui
}  // namespace veles
