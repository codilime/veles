#pragma once

#include <QtWidgets/QHBoxLayout>
#include "richtextwidget.h"
#include "ui/disasm/addresscolumn.h"

namespace veles {
namespace ui {

/*
 * Container for component widgets (address column, hex column, etc).
 * Responsible for rendering them in right position.
 */
class DisasmWidget : public QWidget {
  Q_OBJECT
  QHBoxLayout main_layout;
  AddressColumnWidget address_column;
  RichTextWidget rich_text_widget;

 public:
  DisasmWidget();
};

}  // namespace ui
}  // namespace veles
