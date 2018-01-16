#pragma once

#include <QtWidgets/QHBoxLayout>
#include "ui/disasm/addresscolumn.h"

namespace veles {
namespace ui {

/*
 * Container for component widgets (address column, hex column, etc).
 * Responsible for rendering them in right position.
 */
class DisasmWidget : public QWidget {
  QHBoxLayout main_layout;
  AddressColumnWidget address_column;

 public:
  DisasmWidget();
};

}  // namespace ui
}  // namespace veles
