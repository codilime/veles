#include "ui/disasm/disasmwidget.h"

namespace veles {
namespace ui {

DisasmWidget::DisasmWidget() {
  address_column.setAddressVector({1,  2,  3,  4,  5,  6,  7,  8,
                                   9,  10, 11, 12, 13, 14, 15, 16,
                                   17, 18, 19, 20, 21, 22, 23, 24});

  address_column.setHintMap({{13, 2}, {23, 5}});
  address_column.setFixedWidth(100);  // todo(zpp) fix it somewhen

  main_layout.addWidget(&address_column);
  main_layout.addWidget(&rich_text_widget);
  setLayout(&main_layout);
}

}  // namespace ui
}  // namespace veles
