/*
 * Copyright 2018 CodiLime
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

#include "ui/disasm/row.h"
#include <iostream>
#include <QtWidgets/QStyleOptionFocusRect>
#include <QtWidgets/QStylePainter>

namespace veles {
namespace ui {
namespace disasm {

Row::Row() {
  layout_ = new QHBoxLayout();
  layout_->setSpacing(0);
  layout_->setMargin(0);

  text_ = new QLabel;
  comment_ = new QLabel;
  address_ = new QLabel;

  address_->setMaximumWidth(100);

  layout_->addWidget(address_);
  layout_->addWidget(text_);
  layout_->addWidget(comment_);

  setLayout(layout_);
}

void Row::setIndent(int level) { text_->setIndent(level * 20); }

void Row::setEntry(const EntryChunkCollapsed* entry) {
  id_ = entry->chunk->id;
  address_->setText(
      QString("%1").arg(entry->chunk->addr_begin, 8, 16, QChar('0')));
  comment_->setText("; " + entry->chunk->comment);
  setText(QString(entry->chunk->text_repr->string()));
}

void Row::setEntry(const EntryChunkBegin* entry) {
  id_ = entry->chunk->id;
  address_->setText(
      QString("%1").arg(entry->chunk->addr_begin, 8, 16, QChar('0')));
  comment_->setText("; " + entry->chunk->comment);
  setText(
      QString(entry->chunk->display_name + "::" + entry->chunk->type + " {"));
}

void Row::setEntry(const EntryChunkEnd* entry) {
  id_ = entry->chunk->id;
  address_->setText(
      QString("%1").arg(entry->chunk->addr_end, 8, 16, QChar('0')));
  setText("}");
}

void Row::setEntry(const EntryOverlap* entry) {}

void Row::setEntry(const EntryField* entry) {}

void Row::mouseDoubleClickEvent(QMouseEvent* event) {
  emit chunkCollapse(this->id_);
}

void Row::toggleColumn(Row::ColumnName column_name) {
  switch (column_name) {
    case Row::ColumnName::Address:
      address_->setVisible(!address_->isVisible());
      break;
    case Row::ColumnName::Chunks:
      text_->setVisible(!text_->isVisible());
      break;
    case Row::ColumnName::Comments:
      comment_->setVisible(!comment_->isVisible());
      break;
    default:
      break;
  }
}
void Row::enterEvent(QEvent* event) {
  // TODO(prybicki): Make hover effect only on text_
  auto str = text_->text();
  text_->setText(str.prepend("<u>").append("</u>"));
  update();
}

void Row::leaveEvent(QEvent* event) {
  auto str = text_->text();
  text_->setText(str.remove("<u>").remove("</u>"));
  update();
}
void Row::paintEvent(QPaintEvent* event) {
  QPainter painter(this);
  QStylePainter stylePainter(this);
  if (clickable && underMouse()) {
    //    auto styleOption = QStyleOptionFocusRect();
    //    styleOption.rect = rect();
    //    stylePainter.drawPrimitive(QStyle::PE_FrameFocusRect, styleOption);
  }
}
void Row::setClickable(bool clickable) {
  this->clickable = clickable;
  if (clickable)
    text_->setCursor(Qt::PointingHandCursor);
  else
    text_->setCursor(Qt::ArrowCursor);
}

void Row::setText(QString str) {
  text_->setText(str);
  update();
}

}  // namespace disasm
}  // namespace ui
}  // namespace veles
