#include "ui/disasm/disasmtab.h"

namespace veles {
namespace ui {

DisasmTab::DisasmTab() : IconAwareView("Kotek1", "Kotek2") {
  //  auto* dock_address_column = new DockWidget;
  //  dock_address_column->setWidget(&disasm_widget);
  //  addDockWidget(Qt::LeftDockWidgetArea, dock_address_column);
  setCentralWidget(&disasm_widget);
}

}  // namespace ui
}  // namespace veles