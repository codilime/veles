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

// mwk style
const QString Row::address_style_ = "color: purple; font: bold";
const QString Row::opcode_style_ = "color: green";
const QString Row::modifier_style_ = "color: cyan";
const QString Row::label_style_ = "color: purple";
const QString Row::text_style_ = "color: cyan";
const QString Row::text_style_highlighted_ = "color: cyan; font: bold";
const QString Row::register_style_ = "color: red";
const QString Row::comment_style_ = "color: blue; font: italic bold";
const QString Row::number_style_ = "color: #dd0";
const QString Row::blank_style_ = "color: white";
const QString Row::string_style_ = "color: #dd0";

Row::Row() {
  layout_ = new QHBoxLayout();
  layout_->setSpacing(10);
  layout_->setMargin(0);

  address_ = new QLabel;
  address_->setStyleSheet(address_style_);
  address_->setMaximumWidth(100);

  comment_ = new QLabel;
  comment_->setStyleSheet(comment_style_);

  text_layout_ = new QHBoxLayout();
  indent_label_ = new QLabel;
  indent_label_->setSizePolicy(QSizePolicy::Policy::Fixed,
                               QSizePolicy::Policy::Fixed);
  text_layout_->addWidget(indent_label_);
  clearText();  // clean text_layout_

  text_widget_ = new QWidget;
  text_widget_->setFixedWidth(42 * 20);
  text_widget_->setLayout(text_layout_);

  // here is magic: try to change following number and observe what happens
  // i think reason may be that we have somewhat ill-formed hierarchy of widgets and
  // layouts
  // starting in main widget, through columns, rows, text and its elements
  // i think we need to learn very well which objects influences the size of the
  // others
  // and then construct this hierarchy properly
  // ** other reason (more likely imho) is that we have missing sizeHint() in a few places,
  // namely here in Row and in text_widget_
  // lack of sizeHint() may cause weird qt behavior
  text_widget_->setFixedHeight(42);

  layout_->addWidget(address_);
  layout_->addWidget(text_widget_);
  layout_->addWidget(comment_);

  setLayout(layout_);
}

void Row::setIndent(int level) {
  dynamic_cast<QLabel*>(text_layout_->itemAt(0)->widget())
      ->setIndent(level * 20);
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
  QLayoutItem* item;
  while ((item = text_layout_->takeAt(1)) != nullptr) delete item;
  setIndent(0);
}

void Row::generateTextLabels(TextRepr* repr, QBoxLayout& layout) {
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
  label->setText(repr->string());
  label->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);

  if (is<Keyword>(repr)) {
    Keyword* keyword = to<Keyword>(repr);

    if (keyword->keywordType() == KeywordType::OPCODE)
      label->setStyleSheet(opcode_style_);
    if (keyword->keywordType() == KeywordType::MODIFIER)
      label->setStyleSheet(modifier_style_);
    if (keyword->keywordType() == KeywordType::LABEL)
      label->setStyleSheet(label_style_);
    if (keyword->keywordType() == KeywordType::REGISTER)
      label->setStyleSheet(register_style_);
    layout.addWidget(label, Qt::AlignLeft);
    return;
  }

  if (is<Text>(repr)) {
    Text* text = to<Text>(repr);
    label->setStyleSheet(text->highlight() ? text_style_highlighted_
                                           : text_style_);
    layout.addWidget(label, Qt::AlignLeft);
    return;
  }

  if (is<Blank>(repr)) {
    label->setStyleSheet(blank_style_);
    layout.addWidget(label, Qt::AlignLeft);
    return;
  }

  if (is<Number>(repr)) {
    label->setStyleSheet(number_style_);
    layout.addWidget(label, Qt::AlignLeft);
    return;
  }

  if (is<String>(repr)) {
    // assuming String::string() returns representation already enclosed in
    // quotes
    label->setStyleSheet(string_style_);
    layout.addWidget(label, Qt::AlignLeft);
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
  generateTextLabels(text_repr, *text_layout_);
}

void Row::setEntry(const EntryChunkBegin* entry) {
  address_->setText(
      QString("%1").arg(entry->chunk->addr_begin, 8, 16, QChar('0')));
  //  comment_->setText("; " + entry->chunk->comment);
  dynamic_cast<QLabel*>(text_layout_->itemAt(0)->widget())
      ->setText(QString(entry->chunk->display_name + "::" + entry->chunk->type +
                        " {"));
}

void Row::setEntry(const EntryChunkEnd* entry) {
  address_->setText(
      QString("%1").arg(entry->chunk->addr_end, 8, 16, QChar('0')));
  dynamic_cast<QLabel*>(text_layout_->itemAt(0)->widget())->setText("}");
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
void Row::paintEvent(QPaintEvent* event) {
  QPainter painter(this);

  // debug frame around text_widget_
  // really worth to see it
//  auto rect = event->rect();
//  rect.setWidth(rect.width() - 1);
//  rect.setHeight(rect.height() - 1);
//  painter.setBrush(Qt::NoBrush);
//  painter.setPen(Qt::blue);
//  painter.drawRect(rect);
}

}  // namespace disasm
}  // namespace ui
}  // namespace veles
