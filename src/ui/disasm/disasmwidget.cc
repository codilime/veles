#include "ui/disasm/disasmwidget.h"

namespace veles {
namespace ui {

DisasmWidget::DisasmWidget() {
  address_column.setAddressVector({1,  2,  3,  4,  5,  6,  7,  8,
                                   9,  10, 11, 12, 13, 14, 15, 16,
                                   17, 18, 19, 20, 21, 22, 23, 24});

  address_column.setHintMap({{13, 2}, {23, 5}});
  address_column.setMaximumWidth(100);
  address_column.setMinimumHeight(800);

  main_layout.addWidget(&address_column);
  setLayout(&main_layout);
}

}  // namespace ui
}  // namespace veles
