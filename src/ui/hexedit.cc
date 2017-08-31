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
#include <algorithm>

#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <QScrollBar>

#include "ui/hexedit.h"
#include "util/encoders/factory.h"
#include "util/misc.h"
#include "util/settings/theme.h"

using veles::util::misc::array_size;

namespace veles {
namespace ui {

using util::settings::shortcuts::ShortcutsModel;

static const qint64 verticalAreaSpaceWidth_ = 15;
static const qint64 horizontalAreaSpaceWidth_ = 5;
static const qint64 startMargin_ = 10;
static const qint64 endMargin_ = 10;
static const qint64 cursor_blink_time_ = 500;

void HexEdit::recalculateValues() {
  charWidth_ = fontMetrics().width(QLatin1Char('2'));
  charHeight_ = fontMetrics().height();

  verticalByteBorderMargin_ = charHeight_ / 5;
  dataBytesCount_ = dataModel_->binData().size();
  // TODO(mkow): This should be updated in a separate function (only on actual
  // change of this value) and then cause recalculation of cached text.
  byteCharsCount_ = (dataModel_->binData().width() + 3) / 4;
  byte_max_value_ = ~0ULL;
  if (dataModel_->binData().width() < 64) {
    byte_max_value_ = (1ULL << dataModel_->binData().width()) - 1;
  }

  addressBytes_ = 4;
  if (dataBytesCount_ + startOffset_ >= 0x100000000LL) {
    addressBytes_ = 8;
  }
  addressWidth_ = addressBytes_ * 2 * charWidth_ + verticalAreaSpaceWidth_;

  // plus one for only addr
  rowsCount_ = (dataBytesCount_ + bytesPerRow_ - 1) / bytesPerRow_ + 1;
  rowsOnScreen_ =
      (viewport()->height() - horizontalAreaSpaceWidth_ * 2) / charHeight_;

  // Spacing needs to be odd for proper border alignment.
  spaceAfterByte_ = ((charWidth_ / 3) | 1);
  spaceAfterAsciiByte_ = ((charWidth_ / 8) | 1);
  hexAreaWidth_ = bytesPerRow_ * byteCharsCount_ * charWidth_ +
                  (bytesPerRow_ - 1) * spaceAfterByte_ +
                  verticalAreaSpaceWidth_;
  asciiWidth_ = bytesPerRow_ * (charWidth_ + spaceAfterAsciiByte_);
  lineWidth_ =
      startMargin_ + addressWidth_ + hexAreaWidth_ + asciiWidth_ + endMargin_;

  verticalScrollBar()->setRange(0, rowsCount_ - rowsOnScreen_);
  verticalScrollBar()->setPageStep(rowsOnScreen_);
  startRow_ = verticalScrollBar()->value();

  horizontalScrollBar()->setRange(0, lineWidth_ - viewport()->width());
  startPosX_ = horizontalScrollBar()->value();
}

void HexEdit::resizeEvent(QResizeEvent* /*event*/) {
  if (autoBytesPerRow_) {
    adjustBytesPerRowToWindowSize();
  }
  recalculateValues();
}

void HexEdit::initParseMenu() {
  parsers_menu_.setTitle("Parse selection");
  parsers_menu_.clear();
  parsers_menu_.addAction("auto");
  parsers_menu_.addSeparator();
  for (const auto& id : parsers_ids_) {
    parsers_menu_.addAction(id);
  }
}

HexEdit::HexEdit(FileBlobModel* dataModel, QItemSelectionModel* selectionModel,
                 QWidget* parent)
    : QAbstractScrollArea(parent),
      dataModel_(dataModel),
      bindata_width_(dataModel_->binData().width()),
      chunkSelectionModel_(selectionModel),
      dataBytesCount_(0),
      bytesPerRow_(16),
      autoBytesPerRow_(false),
      startOffset_(0),
      byteCharsCount_(0),
      byte_max_value_(0),
      current_position_(0),
      selection_size_(0),
      selectionColor_(viewport()->palette().color(QPalette::Highlight)),
      current_area_(WindowArea::HEX),
      cursor_pos_in_byte_(0),
      cursor_visible_(false),
      edit_engine_(dataModel_) {
  auto font = util::settings::theme::font();
  setFont(font);

  connect(dataModel_, &FileBlobModel::newBinData, this, &HexEdit::newBinData);
  connect(dataModel_, &FileBlobModel::dataChanged, this, &HexEdit::dataChanged);

  if (chunkSelectionModel_ != nullptr) {
    connect(chunkSelectionModel_, &QItemSelectionModel::currentChanged, this,
            &HexEdit::modelSelectionChanged);
  }

  recalculateValues();

  // Initialize hex & ASCII text cache.
  for (size_t i = 0; i < array_size(hex_text_cache_); i++) {
    hex_text_cache_[i].setPerformanceHint(QStaticText::ModerateCaching);
    hex_text_cache_[i].setText(hexRepresentationFromByte(i));
    hex_text_cache_[i].setTextFormat(Qt::PlainText);
  }
  for (size_t i = 0; i < array_size(ascii_text_cache_); i++) {
    ascii_text_cache_[i].setPerformanceHint(QStaticText::ModerateCaching);
    ascii_text_cache_[i].setText(asciiRepresentationFromByte(i));
    ascii_text_cache_[i].setTextFormat(Qt::PlainText);
  }

  connect(verticalScrollBar(), &QAbstractSlider::valueChanged, this,
          &HexEdit::recalculateValues);
  connect(horizontalScrollBar(), &QAbstractSlider::valueChanged, this,
          &HexEdit::recalculateValues);

  createChunkDialog_ =
      new CreateChunkDialog(dataModel_, chunkSelectionModel_, this);

  createChunkAction_ = ShortcutsModel::getShortcutsModel()->createQAction(
      util::settings::shortcuts::CREATE_CHUNK, this,
      Qt::WidgetWithChildrenShortcut);
  connect(createChunkAction_, &QAction::triggered, [this]() {
    createChunkDialog_->updateParent();
    createChunkDialog_->show();
  });

  createChildChunkAction_ = ShortcutsModel::getShortcutsModel()->createQAction(
      util::settings::shortcuts::CREATE_CHILD_CHUNK, this,
      Qt::WidgetWithChildrenShortcut);
  connect(createChildChunkAction_, &QAction::triggered, [this]() {
    createChunkDialog_->updateParent(true);
    createChunkDialog_->show();
  });

  goToAddressDialog_ = new GoToAddressDialog(this);
  connect(goToAddressDialog_, &QDialog::accepted, [this]() {
    setSelection(goToAddressDialog_->address() - startOffset_, 0,
                 /*set_visible=*/true);
  });
  goToAddressDialog_->setRange(startOffset_, startOffset_ + dataBytesCount_);

  goToAddressAction_ = ShortcutsModel::getShortcutsModel()->createQAction(
      util::settings::shortcuts::GO_TO_ADDRESS, this,
      Qt::WidgetWithChildrenShortcut);
  connect(goToAddressAction_, &QAction::triggered,
          [this]() { goToAddressDialog_->show(); });

  removeChunkAction_ = ShortcutsModel::getShortcutsModel()->createQAction(
      util::settings::shortcuts::REMOVE_CHUNK, this,
      Qt::WidgetWithChildrenShortcut);
  auto remove_chunk_lambda = [this]() {
    auto selectedChunk = chunkSelectionModel_->currentIndex();
    if (!selectedChunk.isValid()) {
      return;
    }
    auto result = QMessageBox::warning(
        this, tr("remove chunk"),
        tr("Remove chunk %1 ?").arg(selectedChunk.data().toString()),
        QMessageBox::Yes | QMessageBox::No);
    if (result != QMessageBox::Yes) {
      return;
    }
    dataModel_->removeRow(selectedChunk.row(), selectedChunk.parent());
  };
  connect(removeChunkAction_, &QAction::triggered, remove_chunk_lambda);
  removeChunkAction_->setEnabled(false);
  auto removeChunkPassiveAction =
      ShortcutsModel::getShortcutsModel()->createQAction(
          util::settings::shortcuts::REMOVE_CHUNK, this,
          Qt::WidgetWithChildrenShortcut);
  connect(removeChunkPassiveAction, &QAction::triggered, remove_chunk_lambda);

  selectChunkAction_ = ShortcutsModel::getShortcutsModel()->createQAction(
      util::settings::shortcuts::SELECT_CHUNK, this,
      Qt::WidgetWithChildrenShortcut);
  connect(selectChunkAction_, &QAction::triggered, [this]() {
    if (!selectedChunk().isValid()) {
      return;
    }
    qint64 byte_offset;
    qint64 size;
    getRangeFromIndex(selectedChunk(), &byte_offset, &size);
    if (size == 0) {
      return;
    }
    setSelection(byte_offset, size, true);
  });

  saveChunkAction_ = ShortcutsModel::getShortcutsModel()->createQAction(
      util::settings::shortcuts::SAVE_CHUNK_TO_FILE, this,
      Qt::WidgetWithChildrenShortcut);
  connect(saveChunkAction_, &QAction::triggered, [this]() {
    saveChunkToFile(QFileDialog::getSaveFileName(this, tr("Save File")));
  });

  saveSelectionAction_ = ShortcutsModel::getShortcutsModel()->createQAction(
      util::settings::shortcuts::SAVE_SELECTION_TO_FILE, this,
      Qt::WidgetWithChildrenShortcut);
  connect(saveSelectionAction_, &QAction::triggered, [this]() {
    saveSelectionToFile(QFileDialog::getSaveFileName(this, tr("Save File")));
  });

  auto copy_action = ShortcutsModel::getShortcutsModel()->createQAction(
      util::settings::shortcuts::COPY, this, Qt::WidgetWithChildrenShortcut);
  connect(copy_action, &QAction::triggered, [this]() { copyToClipboard(); });

  auto paste_acton = ShortcutsModel::getShortcutsModel()->createQAction(
      util::settings::shortcuts::PASTE, this, Qt::WidgetWithChildrenShortcut);
  connect(paste_acton, &QAction::triggered, [this]() { pasteFromClipboard(); });

  addAction(copy_action);
  addAction(paste_acton);
  addAction(createChunkAction_);
  addAction(createChildChunkAction_);
  addAction(goToAddressAction_);
  addAction(removeChunkPassiveAction);
  addAction(selectChunkAction_);
  addAction(saveChunkAction_);
  addAction(saveSelectionAction_);

  menu_.addAction(createChunkAction_);
  menu_.addAction(createChildChunkAction_);
  menu_.addAction(goToAddressAction_);
  menu_.addAction(removeChunkAction_);

  menu_.addSeparator();

  menu_.addAction(selectChunkAction_);
  menu_.addAction(saveChunkAction_);

  menu_.addSeparator();

  menu_.addAction(saveSelectionAction_);

  auto copy_menu = menu_.addMenu("Copy selection as");

  for (const auto& id : util::encoders::EncodersFactory::keys()) {
    QSharedPointer<util::encoders::IEncoder> encoder(
        util::encoders::EncodersFactory::createEncoder(id));

    auto copyAction = new QAction(encoder->encodingDisplayName(), this);
    connect(copyAction, &QAction::triggered,
            [this, encoder] { copyToClipboard(encoder.data()); });

    copy_menu->addAction(copyAction);
  }

  auto paste_menu = menu_.addMenu("Paste from");

  menu_.addSeparator();

  for (const auto& id : util::encoders::EncodersFactory::keys()) {
    QSharedPointer<util::encoders::IDecoder> decoder(
        util::encoders::EncodersFactory::createDecoder(id));

    if (!decoder) {
      continue;
    }

    auto paste_action = new QAction(decoder->decodingDisplayName(), this);
    connect(paste_action, &QAction::triggered,
            [this, decoder] { pasteFromClipboard(decoder.data()); });

    paste_menu->addAction(paste_action);
  }

  hexEncoder_.reset(new util::encoders::HexEncoder());
  textEncoder_.reset(new util::encoders::TextEncoder());
  initParseMenu();
  parsers_menu_.setEnabled(false);
  menu_.addMenu(&parsers_menu_);

  connect(&parsers_menu_, &QMenu::triggered, this, &HexEdit::parse);

  cursor_timer_.setInterval(cursor_blink_time_);
  cursor_timer_.start();
  connect(&cursor_timer_, &QTimer::timeout, this,
          &HexEdit::flipCursorVisibility);

  createAction(util::settings::shortcuts::HEX_MOVE_TO_NEXT_CHAR, [this]() {
    setSelection(current_position_ + 1, selection_size_, /*set_visible=*/true);
  });

  createAction(util::settings::shortcuts::HEX_MOVE_TO_PREV_CHAR, [this]() {
    setSelection(current_position_ - 1, selection_size_, /*set_visible=*/true);
  });

  createAction(util::settings::shortcuts::HEX_MOVE_TO_NEXT_LINE, [this]() {
    setSelection(current_position_ + bytesPerRow_, selection_size_,
                 /*set_visible=*/true);
  });

  createAction(util::settings::shortcuts::HEX_MOVE_TO_PREV_LINE, [this]() {
    setSelection(current_position_ - bytesPerRow_, selection_size_,
                 /*set_visible=*/true);
  });

  createAction(util::settings::shortcuts::HEX_MOVE_TO_NEXT_PAGE, [this]() {
    setSelection(current_position_ + bytesPerRow_ * rowsOnScreen_,
                 selection_size_);
    scrollRows(rowsOnScreen_);
    if (!isByteVisible(current_position_)) {
      scrollToByte(current_position_, /*minimal_change=*/true);
    }
  });

  createAction(util::settings::shortcuts::HEX_MOVE_TO_PREV_PAGE, [this]() {
    setSelection(current_position_ - bytesPerRow_ * rowsOnScreen_,
                 selection_size_);
    scrollRows(-rowsOnScreen_);
    if (!isByteVisible(current_position_)) {
      scrollToByte(current_position_, /*minimal_change=*/true);
    }
  });

  createAction(util::settings::shortcuts::HEX_MOVE_TO_START_OF_LINE, [this]() {
    setSelection(current_position_ - current_position_ % bytesPerRow_,
                 selection_size_, /*set_visible=*/true);
  });

  createAction(util::settings::shortcuts::HEX_MOVE_TO_END_OF_LINE, [this]() {
    setSelection(current_position_ + bytesPerRow_ -
                     (current_position_ % bytesPerRow_) - 1,
                 selection_size_, /*set_visible=*/true);
  });

  createAction(util::settings::shortcuts::HEX_MOVE_TO_START_OF_FILE, [this]() {
    setSelection(0, selection_size_, /*set_visible=*/true);
  });

  createAction(util::settings::shortcuts::HEX_MOVE_TO_END_OF_FILE, [this]() {
    setSelection(dataBytesCount_ - 1, selection_size_, /*set_visible=*/true);
  });

  createAction(util::settings::shortcuts::HEX_MOVE_TO_NEXT_DIGIT, [this]() {
    cursor_pos_in_byte_ += 1;
    if (cursor_pos_in_byte_ >= byteCharsCount_) {
      cursor_pos_in_byte_ = 0;
    }
    resetCursor();
  });

  createAction(util::settings::shortcuts::HEX_MOVE_TO_PREV_DIGIT, [this]() {
    cursor_pos_in_byte_ -= 1;
    if (cursor_pos_in_byte_ < 0) {
      cursor_pos_in_byte_ = byteCharsCount_ - 1;
    }
    resetCursor();
  });

  createAction(util::settings::shortcuts::HEX_REMOVE_SELECTION,
               [this]() { setSelection(current_position_, 0); });

  createAction(util::settings::shortcuts::HEX_SELECT_ALL,
               [this]() { setSelection(0, dataBytesCount_); });

  createAction(util::settings::shortcuts::HEX_SELECT_NEXT_CHAR, [this]() {
    setSelectionEnd(current_position_ + 1);
    scrollToByte(current_position_, /*minimal_change=*/true);
  });

  createAction(util::settings::shortcuts::HEX_SELECT_PREV_CHAR, [this]() {
    setSelectionEnd(current_position_ - 1);
    scrollToByte(current_position_, /*minimal_change=*/true);
  });

  createAction(util::settings::shortcuts::HEX_SELECT_NEXT_LINE, [this]() {
    setSelectionEnd(current_position_ + bytesPerRow_);
    scrollToByte(current_position_, /*minimal_change=*/true);
  });

  createAction(util::settings::shortcuts::HEX_SELECT_PREV_LINE, [this]() {
    setSelectionEnd(current_position_ - bytesPerRow_);
    scrollToByte(current_position_, /*minimal_change=*/true);
  });

  createAction(util::settings::shortcuts::HEX_SCROLL_UP,
               [this]() { scrollRows(-1); });

  createAction(util::settings::shortcuts::HEX_SCROLL_DOWN,
               [this]() { scrollRows(1); });

  createAction(util::settings::shortcuts::HEX_SELECT_NEXT_PAGE, [this]() {
    setSelectionEnd(current_position_ + bytesPerRow_ * rowsOnScreen_);
    scrollRows(rowsOnScreen_);
    if (!isByteVisible(current_position_)) {
      scrollToByte(current_position_, /*minimal_change=*/true);
    }
  });

  createAction(util::settings::shortcuts::HEX_SELECT_PREV_PAGE, [this]() {
    setSelectionEnd(current_position_ - bytesPerRow_ * rowsOnScreen_);
    scrollRows(-rowsOnScreen_);
    if (!isByteVisible(current_position_)) {
      scrollToByte(current_position_, /*minimal_change=*/true);
    }
  });

  createAction(util::settings::shortcuts::HEX_SELECT_START_OF_LINE, [this]() {
    setSelectionEnd(current_position_ - (current_position_ % bytesPerRow_));
    scrollToByte(current_position_, /*minimal_change=*/true);
  });

  createAction(util::settings::shortcuts::HEX_SELECT_END_OF_LINE, [this]() {
    setSelectionEnd(current_position_ + bytesPerRow_ -
                    (current_position_ % bytesPerRow_) - 1);
    scrollToByte(current_position_, /*minimal_change=*/true);
  });

  createAction(util::settings::shortcuts::HEX_SELECT_START_OF_DOCUMENT,
               [this]() {
                 setSelectionEnd(0);
                 scrollToByte(current_position_, /*minimal_change=*/true);
               });

  createAction(util::settings::shortcuts::HEX_SELECT_END_OF_DOCUMENT, [this]() {
    setSelectionEnd(dataBytesCount_ - 1);
    scrollToByte(current_position_, /*minimal_change=*/true);
  });
}

QModelIndex HexEdit::selectedChunk() {
  if (chunkSelectionModel_ == nullptr) {
    return {};
  }
  return chunkSelectionModel_->currentIndex();
}

void HexEdit::createAction(util::settings::shortcuts::ShortcutType type,
                           const std::function<void()>& f) {
  auto action = ShortcutsModel::getShortcutsModel()->createQAction(
      type, this, Qt::WidgetWithChildrenShortcut);
  connect(action, &QAction::triggered, f);
  addAction(action);
}

// Returns where a border around a byte should be drawn.  The result assumes the
// same semantics as used in QRect methods (e.g. QRect::bottom).  Rectangles
// returned by this function are overlapping by exactly one pixel.
QRect HexEdit::bytePosToRect(qint64 pos, bool ascii, qint64 char_pos) {
  qint64 columnNum = pos % bytesPerRow_;
  qint64 rowNum = pos / bytesPerRow_ - startRow_;

  if (rowNum < 0 || rowNum >= rowsOnScreen_) {
    return {};
  }

  qint64 xPos;
  qint64 width;
  if (ascii) {
    xPos = (charWidth_ + spaceAfterAsciiByte_) * columnNum + addressWidth_ +
           hexAreaWidth_ + startMargin_ - startPosX_ + 1;
    width = charWidth_ + 1;
  } else {
    xPos = (byteCharsCount_ * charWidth_ + spaceAfterByte_) * columnNum +
           addressWidth_ + startMargin_ - startPosX_ + char_pos * charWidth_;
    width = charWidth_ * (byteCharsCount_ - char_pos) + spaceAfterByte_;
  }
  qint64 yPos = (rowNum + 1) * charHeight_;
  return QRect(xPos - spaceAfterByte_ / 2 - 1,
               yPos - charHeight_ + verticalByteBorderMargin_, width + 1,
               charHeight_ + 1);
}

qint64 HexEdit::pointToRowNum(QPoint pos) {
  return (pos.y() - verticalByteBorderMargin_) / charHeight_ + startRow_;
}

qint64 HexEdit::pointToColumnNum(QPoint pos) {
  qint64 columnNum = 0;
  qint64 realPosX = pos.x() + startPosX_;

  if (realPosX > addressWidth_ + hexAreaWidth_ + startMargin_ -
                     verticalAreaSpaceWidth_ / 2) {
    auto asciiAreaPos =
        realPosX - (addressWidth_ + hexAreaWidth_ + startMargin_);
    columnNum = (asciiAreaPos + spaceAfterByte_ / 2 - 1) /
                (charWidth_ + spaceAfterAsciiByte_);
  } else {
    auto hexAreaPos = realPosX - (addressWidth_ + startMargin_);
    columnNum = (hexAreaPos + spaceAfterByte_ / 2) /
                (byteCharsCount_ * charWidth_ + spaceAfterByte_);
  }

  return columnNum;
}

qint64 HexEdit::pointToBytePos(QPoint pos) {
  qint64 rowNum = pointToRowNum(pos);
  qint64 columnNum = pointToColumnNum(pos);

  if (columnNum < 0) {
    columnNum = 0;
  }
  if (columnNum >= bytesPerRow_) {
    columnNum = bytesPerRow_ - 1;
  }

  auto bytePos = rowNum * bytesPerRow_ + columnNum;
  if (bytePos >= dataBytesCount_) {
    bytePos = dataBytesCount_ - 1;
  }

  if (bytePos < 0) {
    bytePos = 0;
  }

  return bytePos;
}

HexEdit::WindowArea HexEdit::pointToWindowArea(QPoint pos) {
  qint64 row_num = pointToRowNum(pos);

  if (row_num < startRow_ || row_num - startRow_ >= rowsOnScreen_) {
    return WindowArea::OUTSIDE;
  }

  qint64 real_pos_x = pos.x() + startPosX_;

  if (real_pos_x < startMargin_ + addressWidth_ - verticalAreaSpaceWidth_ / 2) {
    return WindowArea::ADDRESS;
  }

  if (real_pos_x < startMargin_ + addressWidth_ + hexAreaWidth_ -
                       verticalAreaSpaceWidth_ / 2) {
    return WindowArea::HEX;
  }

  if (real_pos_x < lineWidth_ - endMargin_ / 2) {
    return WindowArea::ASCII;
  }

  return WindowArea::OUTSIDE;
}

uint64_t HexEdit::byteValue(qint64 pos) const {
  return edit_engine_.byteValue(pos);
}

uint64_t HexEdit::originalByteValue(qint64 pos) const {
  return edit_engine_.originalByteValue(pos);
}

void HexEdit::setBytesValues(qint64 pos, const data::BinData& new_data) {
  edit_engine_.changeBytes(pos, new_data);
  emit editStateChanged(edit_engine_.hasChanges(), edit_engine_.hasUndo());
}

void HexEdit::discardChanges() {
  edit_engine_.clear();
  emit editStateChanged(edit_engine_.hasChanges(), edit_engine_.hasUndo());
  viewport()->update();
}

qint64 HexEdit::selectionStart() {
  if (selection_size_ < 0) {
    return current_position_ + selection_size_ + 1;
  }
  return current_position_;
}

qint64 HexEdit::selectionEnd() {
  if (selection_size_ < 0) {
    return current_position_ + 1;
  }
  return current_position_ + selection_size_;
}

qint64 HexEdit::selectionSize() { return qAbs(selection_size_); }

QString HexEdit::hexRepresentationFromByte(uint64_t byte_val) {
  // This seems to be much faster than QString::number() + rightJustified()
  // (according to profiling results).
  auto res = QString(byteCharsCount_, '0');
  for (int i = byteCharsCount_ - 1; i >= 0; i--) {
    res[i] = "0123456789abcdef"[byte_val % 16];
    byte_val /= 16;
  }
  assert(byte_val == 0);
  return res;
}

QString HexEdit::addressAsText(qint64 pos) {
  return QString::number(pos + startOffset_, 16)
      .rightJustified(addressBytes_ * 2, '0');
}

QString HexEdit::asciiRepresentationFromByte(uint64_t byte_val) {
  if (byte_val >= 0x20 && byte_val < 0x7f) {
    return QChar::fromLatin1(byte_val);
  }
  return ".";
}

QColor HexEdit::byteTextColorFromByteValue(uint64_t byte_val) {
  // TODO(mkow): better support for non 8 bit bytes.
  return util::settings::theme::byteColor(byte_val & 0xff);
}

QColor HexEdit::byteBackroundColorFromPos(qint64 pos) {
  if (pos >= selectionStart() && pos < selectionEnd() && selectionSize() > 1) {
    return selectionColor_;
  }

  if (byteValue(pos) != originalByteValue(pos)) {
    return util::settings::theme::editedBackground();
  }

  auto index = dataModel_->indexFromPos(pos, selectedChunk().parent());

  if (!index.isValid()) {
    index =
        dataModel_->indexFromPos(pos, dataModel_->index(0, 0, QModelIndex()));
  }

  if (index.isValid()) {
    QVariant maybeColor = index.data(Qt::DecorationRole);
    if (maybeColor.canConvert<QColor>()) {
      return maybeColor.value<QColor>();
    }
  }

  return {};
}

void HexEdit::drawBorder(qint64 start, qint64 size, bool asciiArea,
                         bool dotted) {
  QPainter painter(viewport());

  auto oldPen = painter.pen();
  auto newPen = QPen(oldPen.color());
  if (dotted) {
    newPen.setStyle(Qt::DashLine);
  }
  painter.setPen(newPen);

  // top Line
  for (qint64 i = 0; i < qMin(size, bytesPerRow_); ++i) {
    auto rect = bytePosToRect(start + i, asciiArea);
    if (rect.isEmpty()) {
      continue;
    }
    painter.drawLine(rect.topLeft(), rect.topRight());
  }

  // bootom Line
  for (qint64 i = 0; i < qMin(size, bytesPerRow_); ++i) {
    auto rect = bytePosToRect(start + size - i - 1, asciiArea);
    if (rect.isEmpty()) {
      continue;
    }
    painter.drawLine(rect.bottomLeft(), rect.bottomRight());
  }

  // left line for first byte
  auto rect = bytePosToRect(start, asciiArea);
  if (!rect.isEmpty()) {
    painter.drawLine(rect.topLeft(), rect.bottomLeft());
  }

  // right line for last byte
  rect = bytePosToRect(start + size - 1, asciiArea);
  if (!rect.isEmpty()) {
    painter.drawLine(rect.topRight(), rect.bottomRight());
  }

  if (size > bytesPerRow_) {
    // rest of left line
    for (qint64 i = bytesPerRow_ - start % bytesPerRow_; i < size;
         i += bytesPerRow_) {
      auto rect = bytePosToRect(start + i, asciiArea);
      if (rect.isEmpty()) {
        continue;
      }
      painter.drawLine(rect.topLeft(), rect.bottomLeft());
    }

    // rest of left line
    for (qint64 i = bytesPerRow_ - start % bytesPerRow_ - 1; i < size;
         i += bytesPerRow_) {
      auto rect = bytePosToRect(start + i, asciiArea);
      if (rect.isEmpty()) {
        continue;
      }
      painter.drawLine(rect.topRight(), rect.bottomRight());
    }
  }

  painter.setPen(oldPen);
}

void HexEdit::getRangeFromIndex(const QModelIndex& index, qint64* start,
                                qint64* size) {
  if (index.isValid()) {
    auto posIndex = dataModel_->index(
        index.row(), FileBlobModel::COLUMN_INDEX_POS, index.parent());
    *start = posIndex.data(FileBlobModel::ROLE_BEGIN).toInt();
    *size = posIndex.data(FileBlobModel::ROLE_END).toInt() - *start;
  } else {
    *start = 0;
    *size = 0;
  }
}

void HexEdit::paintEvent(QPaintEvent* event) {
  QPainter painter(viewport());
  auto invalidated_rect = event->rect();

  auto old_pen = painter.pen();
  painter.setPen(QPen(viewport()->palette().color(QPalette::Shadow)));
  auto separator_offset =
      startMargin_ + addressWidth_ - verticalAreaSpaceWidth_ / 2 - startPosX_;
  auto separator_length =
      rowsOnScreen_ * charHeight_ + horizontalAreaSpaceWidth_;
  auto address_bar_area_rect =
      QRect(-startPosX_, 0, separator_offset + startPosX_, separator_length);
  painter.fillRect(address_bar_area_rect,
                   viewport()->palette().color(QPalette::AlternateBase));
  painter.drawLine(separator_offset, 0, separator_offset, separator_length);
  separator_offset = startMargin_ + addressWidth_ + hexAreaWidth_ -
                     verticalAreaSpaceWidth_ / 2 - startPosX_;
  painter.drawLine(separator_offset, 0, separator_offset, separator_length);
  separator_offset = lineWidth_ - endMargin_ / 2 - startPosX_;
  painter.drawLine(separator_offset, 0, separator_offset, viewport()->height());
  painter.drawLine(-startPosX_, separator_length,
                   lineWidth_ - endMargin_ / 2 - startPosX_, separator_length);
  painter.setPen(old_pen);

  auto end_row = std::min(startRow_ + rowsOnScreen_, rowsCount_);
  auto start_byte = startRow_ * bytesPerRow_;
  auto size_to_paint =
      std::min(rowsOnScreen_ * bytesPerRow_, dataBytesCount_ - start_byte);

  veles::data::BinData bytes_values =
      edit_engine_.bytesValues(start_byte, size_to_paint);

  // Draw background.
  // This code will be optimized in another commit to reduce number of calls to
  // fillRect().
  for (auto row_num = startRow_; row_num < end_row; ++row_num) {
    auto first_byte_rect = bytePosToRect(row_num * bytesPerRow_);
    if (first_byte_rect.bottom() < invalidated_rect.y() ||
        first_byte_rect.y() >
            invalidated_rect.y() + invalidated_rect.height()) {
      // We don't have to redraw current line.
      continue;
    }

    // Draw current row.
    for (auto byte_idx = row_num * bytesPerRow_;
         byte_idx < std::min((row_num + 1) * bytesPerRow_, dataBytesCount_);
         byte_idx++) {
      auto hex_rect = bytePosToRect(byte_idx);
      auto ascii_rect = bytePosToRect(byte_idx, true);
      auto bgc = byteBackroundColorFromPos(byte_idx);
      bool redraw_hex = invalidated_rect.intersects(hex_rect);
      bool redraw_ascii = invalidated_rect.intersects(ascii_rect);
      if (redraw_hex && bgc.isValid()) {
        painter.fillRect(hex_rect, bgc);
      }
      if (redraw_ascii && bgc.isValid()) {
        painter.fillRect(ascii_rect, bgc);
      }
    }
  }

  // Draw all text.
  for (auto row_num = startRow_; row_num < end_row; ++row_num) {
    auto y_pos = (row_num - startRow_ + 1) * charHeight_;

    auto first_byte_rect = bytePosToRect(row_num * bytesPerRow_);
    if (first_byte_rect.bottom() < invalidated_rect.y() ||
        first_byte_rect.y() >
            invalidated_rect.y() + invalidated_rect.height()) {
      // We don't have to redraw current line.
      continue;
    }

    auto bytes_offset = std::min(row_num * bytesPerRow_, dataBytesCount_);
    painter.drawText(startMargin_ - startPosX_, y_pos,
                     addressAsText(bytes_offset));

    old_pen = painter.pen();
    for (auto column_num = 0; column_num < bytesPerRow_; ++column_num) {
      auto byte_num = row_num * bytesPerRow_ + column_num;
      if (byte_num < dataBytesCount_) {
        auto hex_rect = bytePosToRect(byte_num);
        auto ascii_rect = bytePosToRect(byte_num, true);
        bool redraw_hex = invalidated_rect.intersects(hex_rect);
        bool redraw_ascii = invalidated_rect.intersects(ascii_rect);

        if (redraw_hex || redraw_ascii) {
          auto byte_val = bytes_values.element64(byte_num - start_byte);
          painter.setPen(QPen(byteTextColorFromByteValue(byte_val)));
          // We use drawStaticText() where possible, as it's much faster than
          // drawText(). Unfortunately, both functions use different
          // coordinates, so we have to adjust them.
          if (redraw_hex) {
            auto x_pos =
                (byteCharsCount_ * charWidth_ + spaceAfterByte_) * column_num +
                addressWidth_ + startMargin_ - startPosX_;
            if (byte_val < array_size(hex_text_cache_)) {
              painter.drawStaticText(x_pos, y_pos - fontMetrics().ascent(),
                                     hex_text_cache_[byte_val]);
            } else {
              painter.drawText(x_pos, y_pos,
                               hexRepresentationFromByte(byte_val));
            }
          }
          if (redraw_ascii) {
            auto x_pos = (charWidth_ + spaceAfterAsciiByte_) * column_num +
                         addressWidth_ + hexAreaWidth_ + startMargin_ -
                         startPosX_;
            if (byte_val < array_size(ascii_text_cache_)) {
              painter.drawStaticText(x_pos, y_pos - fontMetrics().ascent(),
                                     ascii_text_cache_[byte_val]);
            } else {
              painter.drawText(x_pos, y_pos,
                               asciiRepresentationFromByte(byte_val));
            }
          }
        }
      }
    }
    painter.setPen(old_pen);
  }

  // border around selected chunk
  if (selectedChunk().isValid()) {
    qint64 start, size;
    getRangeFromIndex(selectedChunk(), &start, &size);
    drawBorder(start, size);
    drawBorder(start, size, true);
  }

  // border around parent of selected chunk
  if (selectedChunk().parent().isValid()) {
    qint64 start, size;
    getRangeFromIndex(selectedChunk().parent(), &start, &size);
    drawBorder(start, size, false, true);
    drawBorder(start, size, true, true);
  }

  // cursor
  if ((cursor_visible_ || !hasFocus()) && dataBytesCount_ > 0) {
    bool in_ascii_area = current_area_ == WindowArea::ASCII;
    drawBorder(current_position_, 1, true, !in_ascii_area);
    auto rect = bytePosToRect(current_position_, false, cursor_pos_in_byte_);
    if (!rect.isEmpty()) {
      QPainter cursor_painter(viewport());
      auto old_pen = cursor_painter.pen();
      auto new_pen = QPen(old_pen.color());
      if (in_ascii_area) {
        new_pen.setStyle(Qt::DashLine);
      }
      cursor_painter.setPen(new_pen);

      // We have to adjust the size because drawRect semantics differs from the
      // one used by QRect methods.
      cursor_painter.drawRect(rect.adjusted(0, 0, -1, -1));
      cursor_painter.setPen(old_pen);
    }
  }
}

void HexEdit::adjustBytesPerRowToWindowSize() {
  bytesPerRow_ = 1;
  do {
    bytesPerRow_++;
    recalculateValues();
  } while (lineWidth_ <= viewport()->width());
  bytesPerRow_--;
  recalculateValues();
}

void HexEdit::setBytesPerRow(int bytes_count, bool automatic) {
  autoBytesPerRow_ = automatic;
  bytesPerRow_ = bytes_count;
  if (autoBytesPerRow_) {
    adjustBytesPerRowToWindowSize();
  }
  recalculateValues();
  viewport()->update();
}

void HexEdit::setAutoBytesPerRow(bool automatic) {
  autoBytesPerRow_ = automatic;
  if (autoBytesPerRow_) {
    adjustBytesPerRowToWindowSize();
  }
  recalculateValues();
  viewport()->update();
}

void HexEdit::resetCursor() {
  cursor_visible_ = true;
  cursor_timer_.start();
  viewport()->update();
}

void HexEdit::setSelection(qint64 start, qint64 size, bool set_visible) {
  if (start < 0 || dataBytesCount_ == 0) {
    start = 0;
  } else if (start >= dataBytesCount_) {
    start = dataBytesCount_ - 1;
  }

  if (size < 0 && -size > start + 1) {
    size = -start - 1;
  }

  if (size > 0 && size > dataBytesCount_ - start) {
    size = dataBytesCount_ - start;
  }

  if (size == 0 || size == 1) {
    size = -1;
  }

  selection_size_ = size;
  current_position_ = start;
  cursor_pos_in_byte_ = 0;
  createChunkDialog_->setRange(selectionStart(), selectionEnd());

  if (set_visible) {
    scrollToByte(current_position_, /*minimal_change=*/true);
  }

  resetCursor();
  emit selectionChanged(selectionStart(), selectionSize());
}

void HexEdit::contextMenuEvent(QContextMenuEvent* event) {
  bool selectionActive = selectionSize() > 0;
  bool isEditable = selectedChunk().isValid() &&
                    (selectedChunk().flags() & Qt::ItemNeverHasChildren) == 0;
  createChunkAction_->setEnabled(selectionActive);
  createChildChunkAction_->setEnabled(isEditable);
  removeChunkAction_->setEnabled(dataModel_->isRemovable(selectedChunk()));
  selectChunkAction_->setEnabled(selectedChunk().isValid());
  saveChunkAction_->setEnabled(selectedChunk().isValid());

  saveSelectionAction_->setEnabled(selectionActive);
  parsers_menu_.setEnabled(selectionActive);

  menu_.exec(event->globalPos());
}

void HexEdit::setByteValue(qint64 pos, uint64_t byte_value) {
  setBytesValues(pos, data::BinData(bindata_width_, {byte_value}));
}

void HexEdit::applyChanges() {
  edit_engine_.applyChanges();
  emit editStateChanged(edit_engine_.hasChanges(), edit_engine_.hasUndo());
}

void HexEdit::undo() {
  if (!edit_engine_.hasUndo()) {
    return;
  }
  setSelection(edit_engine_.undo(), 1, /*set_visible=*/true);
  emit editStateChanged(edit_engine_.hasChanges(), edit_engine_.hasUndo());
}

void HexEdit::processEditEvent(QKeyEvent* event) {
  uint8_t key = static_cast<uint8_t>(event->text()[0].toLatin1());

  if (current_area_ == WindowArea::ASCII) {
    if (key < 0x20 || key > 0x7e) {
      return;
    }
    setByteValue(current_position_, key);
    setSelection(current_position_ + 1, 0);
  } else {
    uint64_t nibble_val = 0;
    if (key >= '0' && key <= '9') {
      nibble_val = key - '0';
    } else if (key >= 'a' && key <= 'f') {
      nibble_val = key - 'a' + 10;
    } else if (key >= 'A' && key <= 'F') {
      nibble_val = key - 'A' + 10;
    } else {
      return;
    }

    auto current_val = byteValue(current_position_);
    uint8_t shift = (byteCharsCount_ - 1 - cursor_pos_in_byte_) * 4;
    uint64_t new_val = current_val & ~(0xfULL << shift);
    new_val = new_val | (nibble_val << shift);
    if (new_val > byte_max_value_) {
      new_val = byte_max_value_;
    }
    setByteValue(current_position_, new_val);

    cursor_pos_in_byte_ += 1;
    if (cursor_pos_in_byte_ == byteCharsCount_) {
      setSelection(current_position_ + 1, 0);
    } else {
      resetCursor();
    }
    scrollToByte(current_position_, /*minimal_change=*/true);
  }

  viewport()->update();
}

void HexEdit::keyPressEvent(QKeyEvent* event) { processEditEvent(event); }

bool HexEdit::focusNextPrevChild(bool /*next*/) {
  if (current_area_ == WindowArea::ASCII) {
    current_area_ = WindowArea::HEX;
  } else if (current_area_ == WindowArea::HEX) {
    current_area_ = WindowArea::ASCII;
  }
  resetCursor();
  return true;
}

void HexEdit::setSelectionEnd(qint64 bytePos) {
  if (bytePos < 0) {
    bytePos = 0;
  }

  if (bytePos >= dataBytesCount_) {
    bytePos = dataBytesCount_ - 1;
  }

  auto change_size = bytePos - current_position_;

  if (change_size == 0) {
    return;
  }

  if (change_size < 0 && selection_size_ < 2 &&
      selection_size_ - change_size > -1) {
    setSelection(bytePos, selection_size_ - change_size + 2);
  } else if (change_size > 0 && selection_size_ > -1 &&
             selection_size_ - change_size < 2) {
    setSelection(bytePos, selection_size_ - change_size - 2);
  } else {
    setSelection(bytePos, selection_size_ - change_size);
  }
}

void HexEdit::mouseMoveEvent(QMouseEvent* event) {
  setSelectionEnd(pointToBytePos(event->pos()));
}

void HexEdit::copyToClipboard(util::encoders::IEncoder* enc) {
  if (enc == nullptr) {
    if (current_area_ == WindowArea::ASCII) {
      enc = textEncoder_.data();
    } else {
      enc = hexEncoder_.data();
    }
  }
  auto selectedData =
      edit_engine_.bytesValues(selectionStart(), selectionSize());

  QClipboard* clipboard = QApplication::clipboard();
  // TODO(mwk): convert encoders to use BinData.
  clipboard->setText(enc->encode(
      QByteArray(reinterpret_cast<const char*>(selectedData.rawData()),
                 static_cast<int>(selectedData.octets()))));
}

void HexEdit::pasteFromClipboard(util::encoders::IDecoder* enc) {
  if (enc == nullptr) {
    if (current_area_ == WindowArea::ASCII) {
      enc = textEncoder_.data();
    } else {
      enc = hexEncoder_.data();
    }
  }

  QClipboard* clipboard = QApplication::clipboard();
  auto data = enc->decode(clipboard->text());

  qint64 paste_start_position = current_position_;
  if (selectionSize() > 1) {
    paste_start_position = selectionStart();
  }

  auto paste_size = data.size();
  if (dataBytesCount_ - paste_start_position < paste_size) {
    paste_size = dataBytesCount_ - paste_start_position;
  }

  data::BinData new_data(bindata_width_, paste_size);
  for (int i = 0; i < paste_size; ++i) {
    new_data.setElement64(i, static_cast<unsigned char>(data[i]));
  }
  setBytesValues(paste_start_position, new_data);
  setSelection(paste_start_position, paste_size);
}

void HexEdit::setSelectedChunk(QModelIndex newSelectedChunk) {
  chunkSelectionModel_->clear();
  chunkSelectionModel_->setCurrentIndex(newSelectedChunk,
                                        QItemSelectionModel::SelectCurrent);
  viewport()->update();
}

void HexEdit::mousePressEvent(QMouseEvent* event) {
  auto clickedByteNum = pointToBytePos(event->pos());
  auto area = pointToWindowArea(event->pos());

  auto newSelectedChunk =
      dataModel_->indexFromPos(clickedByteNum, selectedChunk().parent());
  if (newSelectedChunk.isValid()) {
    setSelectedChunk(newSelectedChunk);
  }

  if (event->button() == Qt::LeftButton &&
      (area == WindowArea::ASCII || area == WindowArea::HEX)) {
    current_area_ = area;
    if (QApplication::keyboardModifiers() == Qt::ShiftModifier) {
      setSelectionEnd(pointToBytePos(event->pos()));
    } else {
      setSelection(clickedByteNum, 0);
    }
  }
}

void HexEdit::mouseDoubleClickEvent(QMouseEvent* event) {
  if (event->button() != Qt::LeftButton) {
    return;
  }
  auto clickedByteNum = pointToBytePos(event->pos());
  // try to find exact match
  auto newSelectedChunk =
      dataModel_->indexFromPos(clickedByteNum, selectedChunk());
  // if clicked on nonchunk position go back
  if (!dataModel_->indexFromPos(clickedByteNum, selectedChunk().parent())
           .isValid()) {
    newSelectedChunk = selectedChunk().parent();
  } else if (!newSelectedChunk.isValid()) {
    // if has childrens select first one
    if (dataModel_->hasChildren(selectedChunk())) {
      newSelectedChunk = selectedChunk().child(0, 0);
    } else {
      newSelectedChunk = selectedChunk();
    }
  }

  setSelectedChunk(newSelectedChunk);
}

bool HexEdit::isRangeVisible(qint64 start, qint64 size) {
  qint64 visible_start = startRow_ * bytesPerRow_;
  qint64 visible_end = (startRow_ + rowsOnScreen_) * bytesPerRow_;

  return !(start >= visible_end || (start + size) <= visible_start);
}

bool HexEdit::isByteVisible(qint64 bytePos) {
  return isRangeVisible(bytePos, 1);
}

void HexEdit::scrollToByte(qint64 bytePos, bool minimal_change) {
  if (isByteVisible(bytePos) && minimal_change) {
    return;
  }

  auto byte_row = bytePos / bytesPerRow_;
  auto current_top_row = verticalScrollBar()->value();

  auto new_top_row = byte_row;
  if (minimal_change && byte_row > current_top_row) {
    new_top_row = byte_row - rowsOnScreen_ + 1;
  }

  verticalScrollBar()->setValue(new_top_row);
  recalculateValues();

  viewport()->update();
}

void HexEdit::scrollRows(qint64 num_rows) {
  verticalScrollBar()->setValue(verticalScrollBar()->value() + num_rows);
  recalculateValues();

  viewport()->update();
}

void HexEdit::newBinData() {
  if (bindata_width_ != dataModel_->binData().width()) {
    bindata_width_ = dataModel_->binData().width();
    edit_engine_ = util::EditEngine(dataModel_);
    emit editStateChanged(edit_engine_.hasChanges(), edit_engine_.hasUndo());
  }
  recalculateValues();
  goToAddressDialog_->setRange(startOffset_, startOffset_ + dataBytesCount_);
  qint64 new_position = current_position_;
  if (new_position >= dataBytesCount_) {
    new_position = dataBytesCount_ - 1;
  }
  setSelection(new_position, 1);
  viewport()->update();
}

void HexEdit::dataChanged() { viewport()->update(); }

void HexEdit::modelSelectionChanged() {
  scrollToCurrentChunk();
  viewport()->update();
  emit selectionChanged(startOffset_ + selectionStart(), selectionSize());
}

void HexEdit::scrollToCurrentChunk() {
  qint64 start, size;
  getRangeFromIndex(selectedChunk(), &start, &size);
  if (size != 0 && !isRangeVisible(start, size)) {
    scrollToByte(start, /*minimal_change=*/true);
  }
}

void HexEdit::saveSelectionToFile(const QString& path) {
  if (path.isEmpty()) {
    return;
  }

  qint64 byte_offset = selectionStart();
  qint64 size = selectionSize();

  if (size == 0) {
    return;
  }

  saveDataToFile(byte_offset, size, path);
}

void HexEdit::saveChunkToFile(const QString& path) {
  if (path.isEmpty() || !selectedChunk().isValid()) {
    return;
  }

  qint64 byte_offset;
  qint64 size;

  getRangeFromIndex(selectedChunk(), &byte_offset, &size);

  if (size == 0) {
    return;
  }

  saveDataToFile(byte_offset, size, path);
}

void HexEdit::saveDataToFile(qint64 byte_offset, qint64 size,
                             const QString& path) {
  if (byte_offset + size > dataBytesCount_) {
    size = dataBytesCount_ - byte_offset;
  }

  auto data_to_save = edit_engine_.bytesValues(byte_offset, size);

  QFile file(path);
  if (!file.open(QIODevice::WriteOnly)) {
    QMessageBox::information(this, tr("Unable to open file"),
                             file.errorString());
    return;
  }

  QDataStream stream(&file);
  stream.writeRawData(reinterpret_cast<const char*>(data_to_save.rawData()),
                      static_cast<int>(data_to_save.octets()));
}

void HexEdit::setParserIds(const QStringList& ids) {
  parsers_ids_ = ids;
  initParseMenu();
}

void HexEdit::parse(QAction* action) {
  QString parser_id = action->text();
  if (parser_id == "auto") {
    parser_id = "";
  }
  QModelIndex parent;
  qint64 byte_offset = selectionStart();
  qint64 size = selectionSize();

  // try to use selected chunk
  if (size == 0 && selectedChunk().isValid()) {
    getRangeFromIndex(selectedChunk(), &byte_offset, &size);
    parent = selectedChunk();
  }

  if (size == 0) {
    return;
  }
  dataModel_->parse(parser_id, byte_offset, parent);
}

void HexEdit::flipCursorVisibility() {
  if (hasFocus() || !cursor_visible_) {
    cursor_visible_ = !cursor_visible_;
    auto cursor_hex_rect =
        bytePosToRect(current_position_, /*ascii=*/false, cursor_pos_in_byte_);
    viewport()->update(cursor_hex_rect);
    auto cursor_ascii_rect =
        bytePosToRect(current_position_, /*ascii=*/true, cursor_pos_in_byte_);
    viewport()->update(cursor_ascii_rect);
  }
}

}  // namespace ui
}  // namespace veles
