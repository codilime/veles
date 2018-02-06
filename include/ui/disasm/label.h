#pragma once

#include <QObject>
#include <QtWidgets/QLabel>

#include "ui/disasm/disasm.h"

namespace veles {
namespace ui {
namespace disasm {

class Label : public QLabel {
  Q_OBJECT

 public:
  Label(const disasm::Entry& entry);
};

}  // namespace disasm
}  // namespace ui
}  // namespace veles
