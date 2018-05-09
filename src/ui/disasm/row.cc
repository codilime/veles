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
#include <ui/disasm/widget.h>
#include <iostream>
#include <QtGui/QPainter>
#include <QtWidgets/QLayout>

namespace veles {
namespace ui {
namespace disasm {

Label::Label(TextRepr* repr, QWidget* parent)
    : QLabel(repr->string(), parent), repr_(repr) {
  setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
  setText(repr->string());

  if (Row::is<Keyword>(repr)) {
    Keyword* keyword = Row::to<Keyword>(repr);

    switch (keyword->keywordType()) {
      case KeywordType::OPCODE:
        setObjectName("opcode");
        break;
      case KeywordType::MODIFIER:
        setObjectName("modifier");
        break;
      case KeywordType::LABEL:
        setObjectName("label");
        break;
      case KeywordType::REGISTER:
        setObjectName("register");
        break;
      default:
        break;
    }
    return;
  }

  if (Row::is<Text>(repr)) {
    setObjectName("text");
    return;
  }

  if (Row::is<Blank>(repr)) {
    setObjectName("blank");
    return;
  }

  if (Row::is<Number>(repr)) {
    setObjectName("number");
    return;
  }

  if (Row::is<String>(repr)) {
    setObjectName("string");
    return;
  }

  setText(":(");
  std::cerr << "Please add implementation for missing TextRepr subclasses."
            << std::endl;
}

Widget* Label::getWidget() {
  QWidget* par = this->parentWidget();
  while (par and !qobject_cast<Widget*>(par)) {
    par = par->parentWidget();
  }
  return qobject_cast<Widget*>(par);
}

void Label::mouseReleaseEvent(QMouseEvent* event) {
  getWidget()->selectionChange(repr_);
  QLabel::mouseReleaseEvent(event);
}

void Label::resetHighlight() {
  syncHighlight(getWidget()->current_selection());
}

void Label::syncHighlight(const TextRepr* repr) {
  if (!repr) return;
  if (highlight_ != sameReprClass(*repr, *repr_)) {
    setHighlight(!highlight());
    // https://wiki.qt.io/Dynamic_Properties_and_Stylesheets -> Limitations
    style()->unpolish(this);
    style()->polish(this);
  }
}

bool Label::sameReprClass(const TextRepr& lhs, const TextRepr& rhs) {
  return lhs.string() == rhs.string();
  if (Row::is<Keyword>(&lhs) and Row::is<Keyword>(&rhs)) {
    auto new_l = Row::to<Keyword>(&lhs);
    auto new_r = Row::to<Keyword>(&rhs);
    return new_l->keywordType() == new_r->keywordType();
  }
  return false;
}

void Label::setHighlight(bool new_value) { highlight_ = new_value; }

bool Label::highlight() const { return highlight_; }

Row::Row() {
  setSizePolicy(QSizePolicy::Policy::Ignored, QSizePolicy::Policy::Fixed);
  setFixedHeight(18);

  layout_ = new QHBoxLayout();
  layout_->setSpacing(10);
  layout_->setMargin(0);

  address_ = new QLabel;
  address_->setFixedWidth(80);
  address_->setObjectName("address");

  comment_ = new QLabel;
  comment_->setObjectName("comment");

  text_widget_ = new QWidget;
  text_layout_ = new QHBoxLayout();
  text_layout_->setSpacing(0);
  text_widget_->setLayout(text_layout_);
  text_widget_->setFixedWidth(400);
  clearText();  // clean text_layout_

  auto byte_label = new QLabel("ff dd cc aa");

  layout_->addWidget(address_);
  layout_->addWidget(text_widget_);
  layout_->addWidget(comment_);

  setLayout(layout_);
}

void Row::setIndent(int level) {
  text_layout_->setContentsMargins(level * 20, 0, 0, 0);
}

template <typename T>
T* Row::to(const TextRepr* ptr) {
  return dynamic_cast<T*>(const_cast<TextRepr*>(ptr));
}

template <typename T>
bool Row::is(const TextRepr* ptr) {
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

void Row::generateTextLabels(TextRepr* repr, QBoxLayout* layout) {
  if (repr == nullptr)
    std::cerr << "Row is sad, row can't into null pointers in TextRepr ;("
              << std::endl;

  if (is<Sublist>(repr)) {
    for (auto& elemUniquePtr : to<Sublist>(repr)->children())
      if (elemUniquePtr != nullptr)
        generateTextLabels(elemUniquePtr.get(), layout);
    return;
  }

  Label* label = new Label(repr);
  layout->addWidget(label, Qt::AlignLeft);
  label->resetHighlight();

  QWidget* par = this->parentWidget();
  while (par and !qobject_cast<Widget*>(par)) {
    par = par->parentWidget();
  }
  auto widget = qobject_cast<Widget*>(par);

  connect(widget, &Widget::labelSelectionChange, label, &Label::syncHighlight);
}

void Row::setEntry(const EntryChunkCollapsed* entry) {
  address_->setText(
      QString("%1").arg(entry->chunk->addr_begin, 8, 16, QChar('0')));
  if (!entry->chunk->comment.isEmpty()) {
    comment_->setText("; " + entry->chunk->comment);
  }

  clearText();
  TextRepr* text_repr = entry->chunk->text_repr.get();
  generateTextLabels(text_repr, text_layout_);
  text_layout_->addStretch();

  this->id_ = entry->chunk->id;
}

void Row::setEntry(const EntryChunkBegin* entry) {
  address_->setText(
      QString("%1").arg(entry->chunk->addr_begin, 8, 16, QChar('0')));
  if (!entry->chunk->comment.isEmpty()) {
    comment_->setText("; " + entry->chunk->comment);
  }

  clearText();
  auto label = new QLabel();
  label->setObjectName("chunkheader");
  label->setText(
      QString(entry->chunk->display_name + "::" + entry->chunk->type + " {"));
  text_layout_->addWidget(label);
  text_layout_->addStretch();

  this->id_ = entry->chunk->id;
}

void Row::setEntry(const EntryChunkEnd* entry) {
  address_->setText(
      QString("%1").arg(entry->chunk->addr_end, 8, 16, QChar('0')));
  comment_->clear();

  clearText();
  auto label = new QLabel();
  label->setText("}");
  text_layout_->addWidget(label);
  text_layout_->addStretch();

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
      text_widget_->setVisible(!text_widget_->isVisible());
      break;
    case Row::ColumnName::Comments:
      comment_->setVisible(!comment_->isVisible());
      break;
    default:
      break;
  }
}

}  // namespace disasm
}  // namespace ui
}  // namespace veles
