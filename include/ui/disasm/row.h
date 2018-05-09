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

#pragma once

#include <QMouseEvent>
#include <QObject>
#include <QWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>

#include "ui/disasm/disasm.h"

namespace veles {
namespace ui {
namespace disasm {

class Widget;

/**
 * Row consists of three elements: address label, text widget, comment label.
 * They are placed in this order in horizontal box layout.
 *
 * Implementation of TextRepr:
 *
 * Row content is set (by main widget) via overloaded setEntry(*) methods.
 * Text repr interpretation is done whenever EntryChunkCollapsed is provided,
 * because for this moment the most of visually important chunks are of this
 * type.
 *
 * Result of text repr interpretation is stored as labels in text_layout_.
 * The very first QLabel in this layout is for making indent and
 * it is not deleted by calling clearText(). The indent is zeroed of course.
 *
 * The interpretation is done inside method generateTextLabels, which
 * establishes
 * subclass type of given TextRepr and appends new label with appropriate style
 * to given layout (text_layout_). In case of Sublist (subclass of TextRepr) it
 * calls itself recursively for all children.
 *
 * Adding text_layout_ directly to row's main horizontal layout results in
 * misaligned
 * comment labels. Therefore text_layout_ is enclosed with text_widget_ so we
 * can set
 * fixed width and therefore align all comment labels. Unfortunately
 * text_widget_
 * causes much worse visual effect. See comment in Row ctor and debug frames
 * (see paintEvent).
 *
 * To implement clickable elements (registers) we need to create class
 * inheriting QLabel
 * and implement handlers for enterEvent and leaveEvent which will change
 * appearance of the label.
 * To change cursor to prompt clickability following code may be used:
 * label->setCursor(Qt::PointingHandCursor);
 * It will cause that cursor will automatically change whenever cursor is above
 * this label.
 * The same solution was used on branch wip/disasm/clickables
 *
 * Doc written: 28 March 2018, prybicki
 */

#define ROW_HEIGHT 18

class Label : public QLabel {
  Q_OBJECT

  Q_PROPERTY(bool highlight READ highlight WRITE setHighlight)

 public:
  explicit Label(TextRepr* repr, QWidget* parent = nullptr);
  static bool sameReprClass(const TextRepr& lhs, const TextRepr& rhs);
  void setHighlight(bool new_value);
  bool highlight() const;

 public slots:
  void syncHighlight(const TextRepr* new_repr);
  void resetHighlight();

 protected:
  void mouseReleaseEvent(QMouseEvent* event) override;

 private:
  ::veles::ui::disasm::Widget* getWidget();

  bool highlight_ = false;
  TextRepr* repr_;
};

class Row : public QWidget {
  Q_OBJECT

 signals:
  void chunkCollapse(const ChunkID& id);

 public:
  enum ColumnName { Address, Chunks, Comments };

  explicit Row();

  void setEntry(const EntryChunkCollapsed* entry);
  void setEntry(const EntryChunkBegin* entry);
  void setEntry(const EntryChunkEnd* entry);
  void setEntry(const EntryOverlap* entry);
  void setEntry(const EntryField* entry);

  void setIndent(int level);
  void toggleColumn(ColumnName column_name);

 protected:
  void mouseDoubleClickEvent(QMouseEvent* event) override;

 private:
  ChunkID id_;

  QLabel* address_;
  QWidget* text_widget_;
  QHBoxLayout* text_layout_;
  QLabel* comment_;

  QHBoxLayout* layout_;

  // Remove all text labels in middle column
  void clearText();

  // Traverse repr tree and append labels with correct style to the layout
  void generateTextLabels(TextRepr* repr, QBoxLayout* layout);

  // Wrapper for dynamic_cast to T
  template <typename T>
  static T* to(const TextRepr* ptr);

  // Wrapper for checking if we can dynamic_cast to T
  template <typename T>
  static bool is(const TextRepr* ptr);

  friend class ::veles::ui::disasm::Label;
};

}  // namespace disasm
}  // namespace ui
}  // namespace veles
