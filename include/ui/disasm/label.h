#pragma once

#include <QObject>
#include <QtWidgets/QLabel>

#include "ui/disasm/disasm.h"

namespace veles {
namespace ui {

class DisasmLabel : public QLabel {
 Q_OBJECT

 public:
  DisasmLabel(const disasm::Entry& entry);
};

}  // namespace ui
}  // namespace veles
