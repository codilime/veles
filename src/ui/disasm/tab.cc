#include "ui/disasm/tab.h"

namespace veles {
namespace ui {
namespace disasm {

Tab::Tab() : IconAwareView("", "") {
  //  auto* dock_address_column = new DockWidget;
  //  dock_address_column->setWidget(&disasm_widget);
  //  addDockWidget(Qt::LeftDockWidgetArea, dock_address_column);
  setCentralWidget(&disasm_widget);
}

}  // namespace disasm
}  // namespace ui
}  // namespace veles