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
#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <QScrollBar>

#include "ui/hexedit.h"
#include "util/encoders/factory.h"
#include "util/settings/shortcuts.h"
#include "util/settings/theme.h"

namespace veles {
namespace ui {

using util::settings::shortcuts::ShortcutsModel;

static const qint64 verticalAreaSpaceWidth_ = 15;
static const qint64 horizontalAreaSpaceWidth_ = 5;
static const qint64 startMargin_ = 10;
static const qint64 endMargin_ = 10;

void HexEdit::recalculateValues() {
  charWidht_ = fontMetrics().width(QLatin1Char('2'));
  charHeight_ = fontMetrics().height();

  verticalByteBorderMargin_ = charHeight_ / 5;
  dataBytesCount_ = dataModel_->binData().size();
  byteCharsCount_ = (dataModel_->binData().width() + 3) / 4;

  addressBytes_ = 4;
  if (dataBytesCount_ + startOffset_ >= 0x100000000LL) {
    addressBytes_ = 8;
  }
  addressWidth_ = addressBytes_ * 2 * charWidht_ + verticalAreaSpaceWidth_;

  // plus one for only addr
  rowsCount_ = (dataBytesCount_ + bytesPerRow_ - 1) / bytesPerRow_ + 1;
  rowsOnScreen_ =
      (viewport()->height() - horizontalAreaSpaceWidth_ * 2) / charHeight_;

  spaceAfterByte_ = charWidht_ / 2;
  hexAreaWidth_ = bytesPerRow_ * byteCharsCount_ * charWidht_ +
                  (bytesPerRow_ - 1) * spaceAfterByte_ +
                  verticalAreaSpaceWidth_;
  asciiWidth_ = bytesPerRow_ * charWidht_;
  lineWidth_ =
      startMargin_ + addressWidth_ + hexAreaWidth_ + asciiWidth_ + endMargin_;

  verticalScrollBar()->setRange(0, rowsCount_ - rowsOnScreen_);
  verticalScrollBar()->setPageStep(rowsOnScreen_);
  startRow_ = verticalScrollBar()->value();

  horizontalScrollBar()->setRange(0, lineWidth_ - viewport()->width());
  startPosX_ = horizontalScrollBar()->value();
}

void HexEdit::resizeEvent(QResizeEvent *event) {
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
  for (auto id : parsers_ids_) {
    parsers_menu_.addAction(id);
  }

}

HexEdit::HexEdit(FileBlobModel *dataModel, QItemSelectionModel *selectionModel,
                 QWidget *parent)
    : QAbstractScrollArea(parent),
      dataModel_(dataModel),
      chunkSelectionModel_(selectionModel),
      dataBytesCount_(0),
      bytesPerRow_(16),
      autoBytesPerRow_(false),
      startOffset_(0),
      byteCharsCount_(0),
      current_position_(0),
      selection_size_(0),
      current_area_(WindowArea::HEX),
      cursor_pos_in_byte_(0),
      cursor_visible_(false) {
  setFont(util::settings::theme::font());

  connect(dataModel_, &FileBlobModel::newBinData,
      this, &HexEdit::newBinData);
  connect(dataModel_, &FileBlobModel::dataChanged,
      this, &HexEdit::dataChanged);

  if (chunkSelectionModel_) {
    connect(chunkSelectionModel_, &QItemSelectionModel::currentChanged,
        this, &HexEdit::modelSelectionChanged);
  }

  recalculateValues();

  connect(verticalScrollBar(), &QAbstractSlider::valueChanged, this,
          &HexEdit::recalculateValues);
  connect(horizontalScrollBar(), &QAbstractSlider::valueChanged, this,
          &HexEdit::recalculateValues);

  createChunkDialog_ = new CreateChunkDialog(dataModel_, chunkSelectionModel_, this);

  createChunkAction_ = ShortcutsModel::getShortcutsModel()->createQAction(
        util::settings::shortcuts::CREATE_CHUNK, this, Qt::WidgetWithChildrenShortcut);
  connect(createChunkAction_, &QAction::triggered, [this]() {
    createChunkDialog_->updateParent();
    createChunkDialog_->show();
  });

  createChildChunkAction_ = ShortcutsModel::getShortcutsModel()->createQAction(
        util::settings::shortcuts::CREATE_CHILD_CHUNK, this, Qt::WidgetWithChildrenShortcut);
  connect(createChildChunkAction_, &QAction::triggered, [this]() {
    createChunkDialog_->updateParent(true);
    createChunkDialog_->show();
  });

  goToAddressDialog_ = new GoToAddressDialog(this);
  connect(goToAddressDialog_, &QDialog::accepted, [this]() {
    scrollToByte(goToAddressDialog_->address() - startOffset_);
  });
  goToAddressDialog_->setRange(startOffset_, startOffset_ + dataBytesCount_);

  goToAddressAction_ = ShortcutsModel::getShortcutsModel()->createQAction(
        util::settings::shortcuts::GO_TO_ADDRESS, this, Qt::WidgetWithChildrenShortcut);
  connect(goToAddressAction_, &QAction::triggered,
          [this]() { goToAddressDialog_->show(); });

  removeChunkAction_ = ShortcutsModel::getShortcutsModel()->createQAction(
        util::settings::shortcuts::REMOVE_CHUNK, this, Qt::WidgetWithChildrenShortcut);
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
  auto removeChunkPassiveAction = ShortcutsModel::getShortcutsModel()->createQAction(
        util::settings::shortcuts::REMOVE_CHUNK, this, Qt::WidgetWithChildrenShortcut);
  connect(removeChunkPassiveAction, &QAction::triggered, remove_chunk_lambda);

  saveSelectionAction_ = ShortcutsModel::getShortcutsModel()->createQAction(
        util::settings::shortcuts::SAVE_SELECTION_TO_FILE, this, Qt::WidgetWithChildrenShortcut);
  connect(saveSelectionAction_, &QAction::triggered, [this]() {
    saveSelectionToFile(QFileDialog::getSaveFileName(this, tr("Save File")));
  });

  auto action = ShortcutsModel::getShortcutsModel()->createQAction(
        util::settings::shortcuts::COPY, this, Qt::WidgetWithChildrenShortcut);
  connect(action, &QAction::triggered, [this] () {
    copyToClipboard();
  });

  addAction(action);
  addAction(createChunkAction_);
  addAction(createChildChunkAction_);
  addAction(goToAddressAction_);
  addAction(removeChunkPassiveAction);
  addAction(saveSelectionAction_);
  menu_.addAction(createChunkAction_);
  menu_.addAction(createChildChunkAction_);
  menu_.addAction(goToAddressAction_);
  menu_.addAction(removeChunkAction_);
  menu_.addAction(saveSelectionAction_);

  auto copyMenu = menu_.addMenu("Copy as");

  menu_.addSeparator();

  for (auto &id : util::encoders::EncodersFactory::keys()) {
    QSharedPointer<util::encoders::IEncoder> encoder(
        util::encoders::EncodersFactory::create(id));

    auto copyAction = new QAction(encoder->encodingDisplayName(), this);
    connect(copyAction, &QAction::triggered,
            [this, encoder] { copyToClipboard(encoder.data()); });

    copyMenu->addAction(copyAction);
  }

  hexEncoder_.reset(new util::encoders::HexEncoder());
  textEncoder_.reset(new util::encoders::TextEncoder());
  initParseMenu();
  parsers_menu_.setEnabled(false);
  menu_.addMenu(&parsers_menu_);

  connect(&parsers_menu_, &QMenu::triggered, this, &HexEdit::parse);

  cursor_timer_.setInterval(700);
  cursor_timer_.start();
  connect(&cursor_timer_, &QTimer::timeout, this, &HexEdit::flipCursorVisibility);
}

QModelIndex HexEdit::selectedChunk() {
  if (chunkSelectionModel_ == nullptr) {
    return QModelIndex();
  }
  return chunkSelectionModel_->currentIndex();
}

QRect HexEdit::bytePosToRect(qint64 pos, bool ascii) {
  qint64 columnNum = pos % bytesPerRow_;
  qint64 rowNum = pos / bytesPerRow_ - startRow_;

  if (rowNum < 0 || rowNum >= rowsOnScreen_) {
    return QRect();
  }

  qint64 xPos;
  qint64 width;
  if (ascii) {
    xPos = charWidht_ * columnNum + addressWidth_ + hexAreaWidth_ +
           startMargin_ - startPosX_ + 1;
    width = charWidht_ + 1;
  } else {
    xPos = (byteCharsCount_ * charWidht_ + spaceAfterByte_) * columnNum +
           addressWidth_ + startMargin_ - startPosX_;
    width = charWidht_ * byteCharsCount_ + spaceAfterByte_;
  }
  qint64 yPos = (rowNum + 1) * charHeight_;
  return QRect(xPos - spaceAfterByte_ / 2,
               yPos - charHeight_ + verticalByteBorderMargin_, width,
               charHeight_);
}

qint64 HexEdit::pointToRowNum(QPoint pos) {
  return (pos.y() - verticalByteBorderMargin_) / charHeight_ + startRow_;
}

qint64 HexEdit::pointToColumnNum(QPoint pos) {
  qint64 columnNum = 0;
  qint64 realPosX = pos.x() + startPosX_;

  if (realPosX > addressWidth_ + hexAreaWidth_ + startMargin_ -
                     verticalAreaSpaceWidth_ / 2) {
    auto asciAreaPos =
        realPosX - (addressWidth_ + hexAreaWidth_ + startMargin_);
    columnNum = (asciAreaPos + spaceAfterByte_ / 2 - 1) / charWidht_;
  } else {
    auto hexAreaPos = realPosX - (addressWidth_ + startMargin_);
    columnNum = (hexAreaPos + spaceAfterByte_ / 2) /
                (byteCharsCount_ * charWidht_ + spaceAfterByte_);
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

  if (real_pos_x < startMargin_ + addressWidth_ + hexAreaWidth_ - verticalAreaSpaceWidth_ / 2) {
    return WindowArea::HEX;
  }

  if (real_pos_x < lineWidth_ - endMargin_ / 2) {
    return WindowArea::ASCII;
  }

  return WindowArea::OUTSIDE;
}

qint64 HexEdit::byteValue(qint64 pos) {
  return dataModel_->binData()[pos].element64();
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

QString HexEdit::hexRepresentationFromBytePos(qint64 pos) {
  return QString::number(byteValue(pos), 16)
      .rightJustified(byteCharsCount_, '0');
}

QString HexEdit::addressAsText(qint64 pos) {
  return QString::number(pos + startOffset_, 16)
      .rightJustified(addressBytes_ * 2, '0');
}

QString HexEdit::asciiRepresentationFromBytePos(qint64 pos) {
  auto x = byteValue(pos);
  if (x >= 0x20 && x < 0x7f) {
    return QChar::fromLatin1(x);
  }
  return ".";
}

QColor HexEdit::byteTextColorFromPos(qint64 pos) {
  auto x = byteValue(pos);
  // TODO: better support for non 8 bit bytes
  return util::settings::theme::byteColor(x & 0xff);
}

QColor HexEdit::byteBackroundColorFromPos(qint64 pos) {
  auto selectionColor = viewport()->palette().color(QPalette::Highlight);

  if (pos >= selectionStart() && pos < selectionEnd() && selectionSize() > 1) {
    return selectionColor;
  }

  auto index = dataModel_->indexFromPos(pos, selectedChunk().parent());

  if (!index.isValid()) {
    index = dataModel_->indexFromPos(pos,
        dataModel_->index(0, 0, QModelIndex()));
  }

  if (index.isValid()) {
    QVariant maybeColor = index.data(Qt::DecorationRole);
    if (maybeColor.canConvert<QColor>()) {
      return maybeColor.value<QColor>();
    }
  }

  return QColor();
}

void HexEdit::drawBorder(qint64 start, qint64 size, bool asciiArea,
                         bool doted) {
  QPainter painter(viewport());

  auto oldPen = painter.pen();
  auto newPen = QPen(oldPen.color());
  if (doted) {
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

void HexEdit::getRangeFromIndex(QModelIndex index, qint64 *start,
                                qint64 *size) {
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

void HexEdit::paintEvent(QPaintEvent *event) {
  QPainter painter(viewport());

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
  separator_length += charHeight_ + horizontalAreaSpaceWidth_;

  painter.setPen(old_pen);

  for (auto rowNum = startRow_;
       rowNum < qMin(startRow_ + rowsOnScreen_, rowsCount_); ++rowNum) {
    auto yPos = (rowNum - startRow_ + 1) * charHeight_;
    auto bytesOffset = rowNum * bytesPerRow_;
    if (bytesOffset > dataBytesCount_) {
      bytesOffset = dataBytesCount_;
    }
    painter.drawText(startMargin_ - startPosX_, yPos,
                     addressAsText(bytesOffset));

    for (auto columnNum = 0; columnNum < bytesPerRow_; ++columnNum) {
      auto xPos = (byteCharsCount_ * charWidht_ + spaceAfterByte_) * columnNum +
                  addressWidth_ + startMargin_ - startPosX_;
      auto byteNum = rowNum * bytesPerRow_ + columnNum;
      if (byteNum < dataBytesCount_) {
        auto bgc = byteBackroundColorFromPos(byteNum);
        if (bgc.isValid()) {
          painter.fillRect(bytePosToRect(byteNum), bgc);
          painter.fillRect(bytePosToRect(byteNum, true), bgc);
        }

        old_pen = painter.pen();

        painter.setPen(QPen(byteTextColorFromPos(byteNum)));
        painter.drawText(xPos, yPos, hexRepresentationFromBytePos(byteNum));
        xPos = charWidht_ * columnNum + addressWidth_ + hexAreaWidth_ +
               startMargin_ - startPosX_;
        painter.drawText(xPos, yPos, asciiRepresentationFromBytePos(byteNum));

        painter.setPen(old_pen);
      }
    }
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
    drawBorder(current_position_, 1, false, in_ascii_area);
    drawBorder(current_position_, 1, true, !in_ascii_area);
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

void HexEdit::setBytesPerRow(int bytesCount, bool automatic) {
  autoBytesPerRow_ = automatic;
  bytesPerRow_ = bytesCount;
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
  createChunkDialog_->setRange(selectionStart(), selectionEnd());

  if (set_visible ) {
    scrollToByte(selectionStart(), true);
  }

  resetCursor();
  emit selectionChanged(startOffset_ + selectionStart(), selectionSize());
}

void HexEdit::contextMenuEvent(QContextMenuEvent *event) {
  bool selectionActive = selectionSize() > 0;
  bool isEditable = selectedChunk().isValid() &&
                    (selectedChunk().flags() & Qt::ItemNeverHasChildren) == 0;
  bool selectionOrChunkActive = selectionActive || selectedChunk().isValid();
  createChunkAction_->setEnabled(selectionActive);
  createChildChunkAction_->setEnabled(isEditable);
  removeChunkAction_->setEnabled(dataModel_->isRemovable(selectedChunk()));

  saveSelectionAction_->setEnabled(selectionOrChunkActive);
  parsers_menu_.setEnabled(selectionOrChunkActive);

  menu_.exec(event->globalPos());
}

void HexEdit::processMoveEvent(QKeyEvent *event) {
  if (event->matches(QKeySequence::MoveToNextChar)) {
    setSelection(current_position_ + 1, selection_size_);
    if (!isByteVisible(current_position_)) {
      scrollRows(1);
    }
  }

  if (event->matches(QKeySequence::MoveToPreviousChar)) {
    setSelection(current_position_ - 1, selection_size_);
    if (!isByteVisible(current_position_)) {
      scrollRows(-1);
    }
  }

  if (event->matches(QKeySequence::MoveToNextLine)) {
    setSelection(current_position_ + bytesPerRow_, selection_size_);
    if (!isByteVisible(current_position_)) {
      scrollRows(1);
    }
  }

  if (event->matches(QKeySequence::MoveToPreviousLine)) {
    setSelection(current_position_ - bytesPerRow_, selection_size_);
    if (!isByteVisible(current_position_)) {
      scrollRows(-1);
    }
  }

  if (event->matches(QKeySequence::MoveToNextPage)) {
    setSelection(current_position_ + bytesPerRow_ * rowsOnScreen_, selection_size_);
    scrollRows(rowsOnScreen_);
  }

  if (event->matches(QKeySequence::MoveToPreviousPage)) {
    setSelection(current_position_ - bytesPerRow_ * rowsOnScreen_, selection_size_);
    scrollRows(-rowsOnScreen_);
  }

  if (event->matches(QKeySequence::MoveToStartOfLine)) {
    setSelection(current_position_ - current_position_ % bytesPerRow_, selection_size_);
  }

  if (event->matches(QKeySequence::MoveToEndOfLine)) {
    setSelection(current_position_ + bytesPerRow_ - (current_position_ % bytesPerRow_) - 1,
                 selection_size_);
  }

  if (event->matches(QKeySequence::MoveToStartOfDocument)) {
    setSelection(0, selection_size_, /*set_visible=*/true);
  }

  if (event->matches(QKeySequence::MoveToEndOfDocument)) {
    setSelection(dataBytesCount_ - 1, selection_size_, /*set_visible=*/true);
  }

}

void HexEdit::processSelectionChangeEvent(QKeyEvent *event)
{
  if (event->key() == Qt::Key_Escape) {
     setSelection(current_position_, 0);
  }

  if (event->matches(QKeySequence::SelectAll)) {
     setSelection(0, dataBytesCount_);
  }

  if (event->matches(QKeySequence::SelectNextChar)) {
    setSelectionEnd(current_position_ + 1);
    if (!isByteVisible(current_position_)) {
      scrollRows(1);
    }
  }

  if (event->matches(QKeySequence::SelectPreviousChar)) {
    setSelectionEnd(current_position_ - 1);
    if (!isByteVisible(current_position_)) {
      scrollRows(-1);
    }
  }

  if (event->matches(QKeySequence::SelectNextLine)) {
    setSelectionEnd(current_position_ + bytesPerRow_);
    if (!isByteVisible(current_position_)) {
      scrollRows(1);
    }
  }

  if (event->matches(QKeySequence::SelectPreviousLine)) {
    setSelectionEnd(current_position_ - bytesPerRow_);
    if (!isByteVisible(current_position_)) {
      scrollRows(-1);
    }
  }

  if (event->matches(QKeySequence::SelectNextPage)) {
    setSelectionEnd(current_position_ + bytesPerRow_ * rowsOnScreen_);
    scrollRows(rowsOnScreen_);
  }

  if (event->matches(QKeySequence::SelectPreviousPage)) {
    setSelectionEnd(current_position_ - bytesPerRow_ * rowsOnScreen_);
    scrollRows(-rowsOnScreen_);
  }

  if (event->matches(QKeySequence::SelectStartOfLine)) {
    setSelectionEnd(current_position_ - (current_position_ % bytesPerRow_));
  }

  if (event->matches(QKeySequence::SelectEndOfLine)) {
    setSelectionEnd(current_position_ + bytesPerRow_ - (current_position_ % bytesPerRow_) - 1);
  }

  if (event->matches(QKeySequence::SelectStartOfDocument)) {
    setSelectionEnd(0);
    scrollToByte(current_position_, true);
  }

  if (event->matches(QKeySequence::SelectEndOfDocument)) {
    setSelectionEnd(dataBytesCount_ - 1);
    scrollToByte(current_position_, true);
  }

}

void HexEdit::keyPressEvent(QKeyEvent *event) {
  processMoveEvent(event);
  processSelectionChangeEvent(event);
}

bool HexEdit::focusNextPrevChild(bool next) {
  if (current_area_ == WindowArea::ASCII) {
    current_area_ = WindowArea::HEX;
  } else if (current_area_ == WindowArea::HEX) {
    current_area_ = WindowArea::ASCII;
  }
  resetCursor();
  return false;
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

  if (change_size < 0 && selection_size_ < 2 && selection_size_ - change_size > -1) {
    setSelection(bytePos, selection_size_ - change_size + 2);
  } else if (change_size > 0 && selection_size_ > -1 && selection_size_ - change_size < 2) {
    setSelection(bytePos, selection_size_ - change_size - 2);
  } else {
    setSelection(bytePos, selection_size_ - change_size);
  }
}

void HexEdit::mouseMoveEvent(QMouseEvent *event) {
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
      dataModel_->binData().data(selectionStart(), selectionEnd());
  QClipboard *clipboard = QApplication::clipboard();
  // TODO: convert encoders to use BinData
  clipboard->setText(enc->encode(QByteArray(
      (const char *)selectedData.rawData(), (int)selectedData.octets())));
}

void HexEdit::setSelectedChunk(QModelIndex newSelectedChunk) {
  chunkSelectionModel_->clear();
  chunkSelectionModel_->setCurrentIndex(newSelectedChunk,
                                        QItemSelectionModel::SelectCurrent);
  viewport()->update();
}

void HexEdit::mousePressEvent(QMouseEvent *event) {
  auto clickedByteNum = pointToBytePos(event->pos());
  auto area = pointToWindowArea(event->pos());

  auto newSelectedChunk =
      dataModel_->indexFromPos(clickedByteNum, selectedChunk().parent());
  if (newSelectedChunk.isValid()) {
    setSelectedChunk(newSelectedChunk);
  }

  if (event->button() == Qt::LeftButton && (area == WindowArea::ASCII ||
       area == WindowArea::HEX)) {
    current_area_ = area;
    if (QApplication::keyboardModifiers() == Qt::ShiftModifier) {
      setSelectionEnd(pointToBytePos(event->pos()));
    } else {
      setSelection(clickedByteNum, 0);
    }
  }
}

void HexEdit::mouseDoubleClickEvent(QMouseEvent *event) {
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

void HexEdit::scrollToByte(qint64 bytePos, bool doNothingIfVisable) {
  if (isByteVisible(bytePos) && doNothingIfVisable) {
    return;
  }

  verticalScrollBar()->setValue(bytePos / bytesPerRow_);
  recalculateValues();

  viewport()->update();
}

void HexEdit::scrollRows(qint64 num_rows) {
  verticalScrollBar()->setValue(verticalScrollBar()->value() + num_rows);
  recalculateValues();

  viewport()->update();
}

void HexEdit::newBinData() {
  recalculateValues();
  goToAddressDialog_->setRange(startOffset_, startOffset_ + dataBytesCount_);
  setSelection(0, 1);
  viewport()->update();
}

void HexEdit::dataChanged() {
  viewport()->update();
}

void HexEdit::modelSelectionChanged() {
  scrollToCurrentChunk();
  viewport()->update();
  emit selectionChanged(startOffset_ + selectionStart(), selectionSize());
}

void HexEdit::scrollToCurrentChunk() {
  qint64 start, size;
  getRangeFromIndex(selectedChunk(), &start, &size);
  if (size && !isRangeVisible(start, size)) {
    scrollToByte(start, true);
  }
}

void HexEdit::saveSelectionToFile(QString path) {

  if (path.isEmpty()) {
    return;
  }

  qint64 byteOffset = selectionStart();
  qint64 size = selectionSize();

  // try to use selected chunk
  if (size == 0 && selectedChunk().isValid()) {
    getRangeFromIndex(selectedChunk(), &byteOffset, &size);
  }

  if (size == 0) {
    return;
  }

  if (byteOffset + size > dataBytesCount_) {
    size = dataBytesCount_ - byteOffset;
  }

  auto dataToSave = dataModel_->binData().data(byteOffset, byteOffset + size);

  QFile file(path);
  if (!file.open(QIODevice::WriteOnly)) {
    QMessageBox::information(this, tr("Unable to open file"),
                             file.errorString());
    return;
  }

  QDataStream stream(&file);
  stream.writeRawData((const char *)dataToSave.rawData(),
                      static_cast<int>(dataToSave.octets()));
}

void HexEdit::setParserIds(QStringList ids) {
  parsers_ids_ = ids;
  initParseMenu();
}

void HexEdit::parse(QAction *action) {
  QString parser_id = action->text();
  if (parser_id == "auto") {
    parser_id = "";
  }
  QModelIndex parent;
  qint64 byteOffset = selectionStart();
  qint64 size = selectionSize();

  // try to use selected chunk
  if (size == 0 && selectedChunk().isValid()) {
    getRangeFromIndex(selectedChunk(), &byteOffset, &size);
    parent = selectedChunk();
  }

  if (size == 0) {
    return;
  }
  dataModel_->parse(parser_id, byteOffset, parent);
}

void HexEdit::flipCursorVisibility() {
  cursor_visible_ = !cursor_visible_;
  viewport()->update();
}

}  // namespace ui
}  // namespace veles
