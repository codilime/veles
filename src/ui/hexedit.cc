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
#include "util/settings/theme.h"

namespace veles {
namespace ui {

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
  // minus one for status bar
  rowsOnScreen_ =
      (viewport()->height() - horizontalAreaSpaceWidth_ * 2) / charHeight_ - 1;

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
      selectionStart_(0),
      selectionSize_(0) {
  setFont(util::settings::theme::font());

  connect(dataModel_, &FileBlobModel::newBinData, [this]() {
    recalculateValues();
    goToAddressDialog_->setRange(startOffset_, startOffset_ + dataBytesCount_);
    viewport()->update();
  });

  connect(dataModel_, &FileBlobModel::dataChanged,
          [this]() { viewport()->update(); });

  if (chunkSelectionModel_) {
    connect(chunkSelectionModel_, &QItemSelectionModel::currentChanged,
            [this]() { viewport()->update(); });
  }

  recalculateValues();

  connect(verticalScrollBar(), &QAbstractSlider::valueChanged, this,
          &HexEdit::recalculateValues);
  connect(horizontalScrollBar(), &QAbstractSlider::valueChanged, this,
          &HexEdit::recalculateValues);

  createChunkDialog_ = new CreateChunkDialog(dataModel_, this);

  createChunkAction_ = new QAction(tr("&Create chunk"), this);
  connect(createChunkAction_, &QAction::triggered, [this]() {
    createChunkDialog_->setParent(selectedChunk().parent());
    createChunkDialog_->show();
  });

  createChildChunkAction_ = new QAction(tr("&Create child chunk"), this);
  connect(createChildChunkAction_, &QAction::triggered, [this]() {
    createChunkDialog_->setParent(selectedChunk());
    createChunkDialog_->show();
  });

  goToAddressDialog_ = new GoToAddressDialog(this);
  connect(goToAddressDialog_, &QDialog::accepted, [this]() {
    scrollToByte(goToAddressDialog_->address() - startOffset_);
  });

  goToAddressAction_ = new QAction(tr("&Go to address"), this);
  connect(goToAddressAction_, &QAction::triggered,
          [this]() { goToAddressDialog_->show(); });

  removeChunkAction_ = new QAction("Remove chunk", this);
  connect(removeChunkAction_, &QAction::triggered, [this]() {
    auto selectedChunk = chunkSelectionModel_->currentIndex();
    auto result = QMessageBox::warning(
        this, tr("remove chunk"),
        tr("Remove chunk %1 ?").arg(selectedChunk.data().toString()),
        QMessageBox::Yes | QMessageBox::No);
    if (result != QMessageBox::Yes) {
      return;
    }
    dataModel_->removeRow(selectedChunk.row(), selectedChunk.parent());
  });
  removeChunkAction_->setEnabled(false);

  saveSelectionAction_ = new QAction(tr("&Save to file"), this);
  connect(saveSelectionAction_, &QAction::triggered, [this]() {
    saveSelectionToFile(QFileDialog::getSaveFileName(this, tr("Save File")));
  });

  menu_.addAction(createChunkAction_);
  menu_.addAction(createChildChunkAction_);
  menu_.addAction(goToAddressAction_);
  menu_.addAction(removeChunkAction_);
  menu_.addAction(saveSelectionAction_);

  auto copyMenu = menu_.addMenu("Copy");

  menu_.addSeparator();

  for (auto &id : util::encoders::EncodersFactory::keys()) {
    QSharedPointer<util::encoders::Encoder> encoder(
        util::encoders::EncodersFactory::create(id));

    auto copyAction = new QAction(encoder->displayName(false), this);
    connect(copyAction, &QAction::triggered,
            [this, encoder] { copyToClipboard(encoder.data()); });

    copyMenu->addAction(copyAction);
  }

  hexEncoder_.reset(new util::encoders::HexEncoder());
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

qint64 HexEdit::pointToBytePos(QPoint pos) {
  qint64 rowNum =
      (pos.y() - verticalByteBorderMargin_) / charHeight_ + startRow_;
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

qint64 HexEdit::byteValue(qint64 pos) {
  return dataModel_->binData()[pos].element64();
}

qint64 HexEdit::selectionStart() {
  if (selectionSize_ < 0) {
    return selectionStart_ + selectionSize_ + 1;
  }
  return selectionStart_;
}

qint64 HexEdit::selectionEnd() {
  if (selectionSize_ < 0) {
    return selectionStart_ + 1;
  }
  return selectionStart_ + selectionSize_;
}

qint64 HexEdit::selectionSize() { return qAbs(selectionSize_); }

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

  if (pos >= selectionStart() && pos < selectionEnd()) {
    return selectionColor;
  }

  auto index = dataModel_->indexFromPos(pos, selectedChunk().parent());
  if (index.isValid()) {
    QVariant maybeColor = index.data(Qt::DecorationRole);
    if (maybeColor.canConvert<QColor>()) {
      return maybeColor.value<QColor>();
    }
  }

  return QColor();
}

QString HexEdit::statusBarText() {
  return QString("current selection: %1:%2 (%3 bytes)")
      .arg(addressAsText(selectionStart()))
      .arg(addressAsText(selectionEnd()))
      .arg(selectionSize());
}

void HexEdit::drawBorder(qint64 start, qint64 size, bool asciiArea,
                         bool doted) {
  QPainter painter(viewport());

  if (size == 0) {
    return;
  }

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

  auto separatorOffset =
      startMargin_ + addressWidth_ - verticalAreaSpaceWidth_ / 2 - startPosX_;
  auto separatorLength =
      rowsOnScreen_ * charHeight_ + horizontalAreaSpaceWidth_;
  auto addressBarAreaRect =
      QRect(-startPosX_, 0, separatorOffset + startPosX_, separatorLength);
  painter.fillRect(addressBarAreaRect,
                   viewport()->palette().color(QPalette::AlternateBase));
  painter.drawLine(separatorOffset, 0, separatorOffset, separatorLength);
  separatorOffset = startMargin_ + addressWidth_ + hexAreaWidth_ -
                    verticalAreaSpaceWidth_ / 2 - startPosX_;
  painter.drawLine(separatorOffset, 0, separatorOffset, separatorLength);
  separatorOffset = lineWidth_ - endMargin_ / 2 - startPosX_;
  painter.drawLine(separatorOffset, 0, separatorOffset, viewport()->height());
  painter.drawLine(-startPosX_, separatorLength,
                   lineWidth_ - endMargin_ / 2 - startPosX_, separatorLength);
  separatorLength += charHeight_ + horizontalAreaSpaceWidth_;

  painter.drawText(startMargin_ - startPosX_,
                   separatorLength - horizontalAreaSpaceWidth_,
                   statusBarText());

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

        auto oldPen = painter.pen();

        painter.setPen(QPen(byteTextColorFromPos(byteNum)));
        painter.drawText(xPos, yPos, hexRepresentationFromBytePos(byteNum));
        xPos = charWidht_ * columnNum + addressWidth_ + hexAreaWidth_ +
               startMargin_ - startPosX_;
        painter.drawText(xPos, yPos, asciiRepresentationFromBytePos(byteNum));

        painter.setPen(oldPen);
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

void HexEdit::setSelection(qint64 start, qint64 size, bool setVisable) {

  if (size < 0 && -size > start + 1) {
    size = -start - 1;
  }

  if (size > 0 && size > dataBytesCount_ - start) {
    size = dataBytesCount_ - start;
  }

  selectionSize_ = size;
  selectionStart_ = start;
  createChunkDialog_->setRange(selectionStart(), selectionEnd());

  if (setVisable) {
    scrollToByte(selectionStart(), true);
  }

  viewport()->update();
}

void HexEdit::contextMenuEvent(QContextMenuEvent *event) {
  bool selectionActive = selectionSize() > 0;
  bool isEditable = selectedChunk().isValid() &&
                    (selectedChunk().flags() & Qt::ItemNeverHasChildren) == 0;
  createChunkAction_->setEnabled(selectionActive);
  createChildChunkAction_->setEnabled(isEditable);
  removeChunkAction_->setEnabled(dataModel_->isRemovable(selectedChunk()));

  saveSelectionAction_->setEnabled(selectionActive ||
                                   selectedChunk().isValid());

  menu_.exec(event->globalPos());
}

void HexEdit::keyPressEvent(QKeyEvent *event) {
  if (event->matches(QKeySequence::Copy)) {
    copyToClipboard();
  }
}

void HexEdit::setSelectionEnd(qint64 bytePos) {
  auto selectionSize = bytePos - selectionStart_;
  if (selectionSize >= 0) {
    selectionSize += 1;
  } else {
    selectionSize -= 1;
  }
  setSelection(selectionStart_, selectionSize);
}

void HexEdit::mouseMoveEvent(QMouseEvent *event) {
  setSelectionEnd(pointToBytePos(event->pos()));
}

void HexEdit::copyToClipboard(util::encoders::Encoder *enc) {
  if (enc == nullptr) {
    enc = hexEncoder_.data();
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

  auto newSelectedChunk =
      dataModel_->indexFromPos(clickedByteNum, selectedChunk().parent());
  if (newSelectedChunk.isValid()) {
    setSelectedChunk(newSelectedChunk);
  }

  if (event->button() == Qt::LeftButton) {
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

bool HexEdit::isByteVisible(qint64 bytePos) {
  return (bytePos >= startRow_ * bytesPerRow_) &&
         (bytePos < (startRow_ + rowsOnScreen_) * bytesPerRow_);
}

void HexEdit::scrollToByte(qint64 bytePos, bool doNothingIfVisable) {
  if (isByteVisible(bytePos) && doNothingIfVisable) {
    return;
  }

  verticalScrollBar()->setValue(bytePos / bytesPerRow_);
  recalculateValues();

  viewport()->update();
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

}  // namespace ui
}  // namespace veles
