#pragma once

#include "ui/disasm/widget.h"
#include "ui/dockwidget.h"

namespace veles {
namespace ui {
namespace disasm {

class Tab : public IconAwareView {
 Q_OBJECT

  Widget disasm_widget;

 public:
  Tab();
};

}  // namespace disasm
}  // namespace ui
}  // namespace veles
