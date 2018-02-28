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
#include <QWidget>

namespace veles {
namespace ui {
namespace disasm {

Row::Row(int indent_level) : indent_level_{indent_level} {
  //  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  layout_ = new QHBoxLayout();
  layout_->setSpacing(0);
  layout_->setMargin(0);

  text_ = new QLabel;
  text_->setIndent(indent_level * 20);
  comment_ = new QLabel;
  address_ = new QLabel;

  address_->setMaximumWidth(100);

  layout_->addWidget(address_);
  layout_->addWidget(text_);
  layout_->addWidget(comment_);

  setLayout(layout_);
}

void Row::setEntry(const EntryChunkBegin* entry) {
  address_->setText(
      QString("%1").arg(entry->chunk->addr_begin, 8, 16, QChar('0')));
  comment_->setText("; " + entry->chunk->comment);
  if (entry->chunk->collapsed) {
    text_->setText(QString(entry->chunk->text_repr->string()));
  } else {
    text_->setText(
        QString(entry->chunk->display_name + "::" + entry->chunk->type + " {"));
  }
}

void Row::setEntry(const EntryChunkEnd* entry) {
  address_->setText(
      QString("%1").arg(entry->chunk->addr_end, 8, 16, QChar('0')));
  text_->setText("}");
}

void Row::setEntry(const EntryOverlap* entry) {}

void Row::setEntry(const EntryField* entry) {}
}  // namespace disasm
}  // namespace ui
}  // namespace veles
