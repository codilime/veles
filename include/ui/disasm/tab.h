#pragma once

#include "ui/disasm/widget.h"
#include "ui/dockwidget.h"

namespace veles {
namespace ui {

class DisasmTab : public IconAwareView {
  Q_OBJECT

  DisasmWidget disasm_widget;

 public:
  DisasmTab();
};

}  // namespace ui
}  // namespace veles
