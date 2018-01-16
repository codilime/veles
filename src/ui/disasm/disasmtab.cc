#include "ui/disasm/disasmtab.h"

namespace veles {
namespace ui {

DisasmTab::DisasmTab() : IconAwareView("", ""), tmp("Hello disasm!", this) {
  tmp.move(10, 10);
}

void DisasmTab::paintEvent(QPaintEvent* event) { tmp.update(); }

DisasmTab::~DisasmTab() {}

}  // namespace ui
}  // namespace veles
