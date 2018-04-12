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
#include <QtGui/QPainter>
#include <QtWidgets/QLayout>

namespace veles {
namespace ui {
namespace disasm {


Row::Row() {
  setSizePolicy(QSizePolicy::Policy::Ignored, QSizePolicy::Policy::Fixed);
  setFixedHeight(20);

  layout_ = new QHBoxLayout();
  layout_->setSpacing(0);
  layout_->setMargin(0);

  address_ = new QLabel;
  address_->setFixedWidth(120);
  address_->setObjectName("address");

  comment_ = new QLabel;
  comment_->setObjectName("comment");

  text_layout_ = new QHBoxLayout();
  text_layout_->setSpacing(0);
  clearText();  // clean text_layout_

  layout_->addWidget(address_);
  layout_->addLayout(text_layout_);
  layout_->addStretch();
  layout_->addWidget(comment_);

  setLayout(layout_);
}

void Row::setIndent(int level) {
  text_layout_->setContentsMargins(level * 20,0,0,0);
}

template <typename T>
T* Row::to(TextRepr* ptr) {
  return dynamic_cast<T*>(ptr);
}

template <typename T>
bool Row::is(TextRepr* ptr) {
  return to<T>(ptr) != nullptr;
}

void Row::clearText() {
  while (text_layout_->count() > 0) {
      auto item = text_layout_->takeAt(0);
      if (item->widget()) {
          delete item->widget();
      }
      delete item;
  }
  setIndent(0);
}

void Row::generateTextLabels(TextRepr* repr, QBoxLayout *layout) {
  if (repr == nullptr)
    std::cerr << "Row is sad, row can't into null pointers in TextRepr ;("
              << std::endl;

  if (is<Sublist>(repr)) {
    for (auto& elemUniquePtr : to<Sublist>(repr)->children())
      if (elemUniquePtr != nullptr)
        generateTextLabels(elemUniquePtr.get(), layout);
    return;
  }

  QLabel* label = new QLabel;
  label->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
  label->setText(repr->string());

  if (is<Keyword>(repr)) {
    Keyword* keyword = to<Keyword>(repr);

    if (keyword->keywordType() == KeywordType::OPCODE) {
      label->setObjectName("opcode");
    }
    if (keyword->keywordType() == KeywordType::MODIFIER) {
      label->setObjectName("modifier");
    }
    if (keyword->keywordType() == KeywordType::LABEL) {
      label->setObjectName("label");
    }
    if (keyword->keywordType() == KeywordType::REGISTER) {
      label->setObjectName("register");
    }
    layout->addWidget(label, Qt::AlignLeft);
    return;
  }

  if (is<Text>(repr)) {
    Text* text = to<Text>(repr);
    label->setObjectName("text");
    layout->addWidget(label, Qt::AlignLeft);
    return;
  }

  if (is<Blank>(repr)) {
    label->setObjectName("blank");
    layout->addWidget(label, Qt::AlignLeft);
    return;
  }

  if (is<Number>(repr)) {
    label->setObjectName("number");
    layout->addWidget(label, Qt::AlignLeft);
    return;
  }

  if (is<String>(repr)) {
    label->setObjectName("string");
    layout->addWidget(label, Qt::AlignLeft);
    return;
  }

  label->setText(":(");
  std::cerr << "Please add implementation for missing TextRepr subclasses."
            << std::endl;
}

void Row::setEntry(const EntryChunkCollapsed* entry) {
  address_->setText(
      QString("%1").arg(entry->chunk->addr_begin, 8, 16, QChar('0')));
  comment_->setText("; " + entry->chunk->comment);
  clearText();

  TextRepr* text_repr = entry->chunk->text_repr.get();
  generateTextLabels(text_repr, text_layout_);
  this->id_ = entry->chunk->id;
}

void Row::setEntry(const EntryChunkBegin* entry) {
  clearText();
  address_->setText(
      QString("%1").arg(entry->chunk->addr_begin, 8, 16, QChar('0')));
  comment_->setText("; " + entry->chunk->comment);
  auto label = new QLabel();
  label->setText(QString(entry->chunk->display_name + "::" + entry->chunk->type + " {"));
  text_layout_->addWidget(label);
  this->id_ = entry->chunk->id;
}

void Row::setEntry(const EntryChunkEnd* entry) {
  address_->setText(
      QString("%1").arg(entry->chunk->addr_end, 8, 16, QChar('0')));
  clearText();
  auto label = new QLabel();
  label->setText("}");
  text_layout_->addWidget(label);
  this->id_ = entry->chunk->id;
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
      dynamic_cast<QLabel*>(text_layout_->itemAt(0)->widget())
          ->setVisible(
              !(dynamic_cast<QLabel*>(text_layout_->itemAt(0)->widget())
                    ->isVisible()));
      break;
    case Row::ColumnName::Comments:
      //      comment_->setVisible(!comment_->isVisible());
      break;
    default:
      break;
  }
}

}  // namespace disasm
}  // namespace ui
}  // namespace veles
