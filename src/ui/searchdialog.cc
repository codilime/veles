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

SearchDialog::SearchDialog(HexEdit* hexEdit, QWidget* parent)
    : QDialog(parent),
      ui(new Ui::SearchDialog),
      _lastFoundPos(-1),
      _lastFoundSize(0) {
  ui->setupUi(this);
  _hexEdit = hexEdit;
  message_box_not_found_ = new QMessageBox(this);
  message_box_not_found_->setWindowTitle(tr("HexEdit"));
  message_box_not_found_->setText(tr("Not found, wrapping to the beginning."));
  message_box_not_found_->setStandardButtons(QMessageBox::Close);
  message_box_not_found_->setDefaultButton(QMessageBox::Close);
  message_box_not_found_->setIcon(QMessageBox::Information);

  message_box_not_valid_hex_string_ = new QMessageBox(this);
  message_box_not_valid_hex_string_->setWindowTitle(tr("HexEdit"));
  message_box_not_valid_hex_string_->setStandardButtons(QMessageBox::Close);
  message_box_not_valid_hex_string_->setDefaultButton(QMessageBox::Close);

  connect(_hexEdit, &HexEdit::selectionChanged,
          [this](qint64 start_addr, qint64 selection_size) {
            _lastFoundPos = start_addr;
            _lastFoundSize = 0;
          });

  ui->warning_label->setVisible(false);

  connect(ui->cbFind, &QComboBox::editTextChanged, this,
          &SearchDialog::findTextChanged);
  connect(ui->cbReplace, &QComboBox::editTextChanged, this,
          &SearchDialog::replaceTextChanged);
  connect(
      ui->cbFindFormat,
      static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
      [this](int) {
        enableReplace(ui->cbFind->currentText(), ui->cbReplace->currentText());
      });
  connect(
      ui->cbReplaceFormat,
      static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
      [this](int) {
        enableReplace(ui->cbFind->currentText(), ui->cbReplace->currentText());
      });
}

SearchDialog::~SearchDialog() { delete ui; }

qint64 SearchDialog::indexOf(const data::BinData& pattern, qint64 startPos) {
  // TODO(mwk): implement this as BinData method or as separate util
  const data::BinData& data = _hexEdit->dataModel()->binData();
  if (startPos == -1) {
    startPos = 0;
  }
  qint64 index = startPos;
  while (index + pattern.size() <= data.size()) {
    size_t numberOfMatches = 0;
    while (numberOfMatches < pattern.size() &&
           numberOfMatches + index < data.size() &&
           pattern.element64(numberOfMatches) ==
               _hexEdit->byteValue(index + numberOfMatches)) {
      ++numberOfMatches;
    }

    if (numberOfMatches == pattern.size()) {
      return index;
    }

    index++;
  }

  return -1;
}

qint64 SearchDialog::lastIndexOf(const data::BinData& pattern,
                                 qint64 startPos) {
  // TODO(mwk): implement this as BinData method or as separate util
  const data::BinData& data = _hexEdit->dataModel()->binData();
  if (startPos == -1) {
    startPos = data.size();
  }
  qint64 index = startPos - 1;
  while (index >= 0) {
    size_t numberOfMatches = 0;
    while (numberOfMatches < pattern.size() &&
           numberOfMatches + index < data.size() &&
           pattern.element64(numberOfMatches) ==
               _hexEdit->byteValue(index + numberOfMatches)) {
      ++numberOfMatches;
    }

    if (numberOfMatches == pattern.size()) {
      return index;
    }

    index--;
  }

  return -1;
}

void SearchDialog::replace(qint64 pos, const data::BinData& data) {
  _hexEdit->setBytesValues(pos, data);
}

void SearchDialog::enableReplace(const QString& find, const QString& replace) {
  auto replace_len = replace.toUtf8().length();
  if (ui->cbReplaceFormat->currentIndex() == 1) {
    replace_len *= (_hexEdit->dataModel()->binData().width() + 3) / 4;
  }
  auto find_len = find.toUtf8().length();
  if (ui->cbFindFormat->currentIndex() == 1) {
    find_len *= (_hexEdit->dataModel()->binData().width() + 3) / 4;
  }
  ui->warning_label->setVisible(!replace.isEmpty() && replace_len != find_len);
  bool enable_replace = !replace.isEmpty() && replace_len == find_len;
  ui->pbReplace->setEnabled(enable_replace);
  ui->pbReplaceAll->setEnabled(enable_replace);
}

qint64 SearchDialog::findNext(bool include_overlapping, bool interactive) {
  emit enableFindNext(false);

  _findBa =
      getContent(ui->cbFindFormat->currentIndex(), ui->cbFind->currentText());

  if (_findBa.size() == 0) {
    return -1;
  }

  bool backwards = ui->cbBackwards->isChecked();
  bool overlapping = ui->cbOverlapping->isChecked() && include_overlapping;

  qint64 start_search_pos_modifier = 0;
  if (_lastFoundSize > 0) {
    if (overlapping) {
      start_search_pos_modifier = 1;
    } else {
      start_search_pos_modifier = _lastFoundSize;
    }
  }

  if (backwards) {
    start_search_pos_modifier = -(start_search_pos_modifier - 1);
  }

  qint64 start_search_pos = _lastFoundPos + start_search_pos_modifier;

  qint64 idx = -1;
  if (backwards) {
    idx = lastIndexOf(_findBa, start_search_pos);
  } else {
    idx = indexOf(_findBa, start_search_pos);
  }

  if (idx >= 0) {
    _hexEdit->setSelection(idx, _findBa.size(), true);
    _lastFoundSize = _findBa.size();
    emit enableFindNext(true);
  } else {
    _lastFoundSize = 0;
    if (backwards) {
      _hexEdit->setSelection(_hexEdit->dataModel()->binData().size() - 1, 0,
                             false);
    } else {
      _hexEdit->setSelection(0, 0, false);
    }
    if (interactive) {
      message_box_not_found_->show();
    }
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

  findNext(/*include_overlapping=*/false);
}

void SearchDialog::on_pbReplaceAll_clicked() {
  _lastFoundPos = -1;
  int replaceCounter = 0;
  int idx = 0;
  while (idx >= 0) {
    idx = findNext(/*include_overlapping=*/false, /*interactive=*/false);
    if (idx >= 0) {
      data::BinData replaceBa = getContent(ui->cbReplaceFormat->currentIndex(),
                                           ui->cbReplace->currentText());
      replaceOccurrence(idx, replaceBa);
      replaceCounter += 1;
    }
  }

  if (replaceCounter > 0) {
    QMessageBox::information(
        this, tr("HexEdit"),
        QString(tr("%1 occurrences replaced.")).arg(replaceCounter));
  }
}

void SearchDialog::findTextChanged(const QString& text) {
  enableReplace(text, ui->cbReplace->currentText());
}

void SearchDialog::replaceTextChanged(const QString& text) {
  enableReplace(ui->cbFind->currentText(), text);
}

bool SearchDialog::isHexStr(const QString& hexStr) {
  auto hexCharsPerByte = _hexEdit->dataModel()->binData().width() / 4;
  QRegExp hexMatcher(QString("^(([0-9A-F]{%1})|\\s)*$").arg(hexCharsPerByte),
                     Qt::CaseInsensitive);
  return hexMatcher.exactMatch(hexStr);
}

data::BinData SearchDialog::getContent(int comboIndex, const QString& input) {
  std::vector<uint64_t> findBa;
  int hexCharsPerByte = _hexEdit->dataModel()->binData().width() / 4;
  switch (comboIndex) {
    case 0:  // hex
      if (!isHexStr(input)) {
        message_box_not_valid_hex_string_->setText(
            QString(tr("\"%1\" is not valid hex string.")).arg(input));
        message_box_not_valid_hex_string_->show();
      } else {
        QString currentByte;
        for (QChar hexChar : input.toLatin1()) {
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
      for (auto byte : input.toUtf8()) {
        findBa.push_back(byte);
      }
      break;
  }

  data::BinData result(hexCharsPerByte * 4, findBa.size());
  size_t pos = 0;
  for (auto x : findBa) {
    result.setElement64(pos++, x);
  }
  return result;
}

void SearchDialog::replaceOccurrence(qint64 idx,
                                     const data::BinData& replaceBa) {
  replace(idx, replaceBa);
}

}  // namespace ui
}  // namespace veles
