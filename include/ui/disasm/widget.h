#pragma once

#include <QObject>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>

#include "ui/disasm/disasm.h"
#include "ui/disasm/label.h"

#include "ui/disasm/mocks.h"

namespace veles {
namespace ui {
namespace disasm {

/*
 * Container for component widgets (address column, hex column, etc).
 * Responsible for rendering them in right position.
 */
class Widget : public QWidget {
  Q_OBJECT
  QHBoxLayout main_layout;
  QVBoxLayout sub_layout;

 public slots:

  void getWindow();

 public:
  Widget();

  void setupMocks();

  void getEntrypoint();

 private:
  QFuture<disasm::Bookmark> entrypoint_;

  std::unique_ptr<disasm::Blob> blob_;
  std::unique_ptr<disasm::Window> window_;

  QFutureWatcher<disasm::Bookmark> watcher_;
};

}  // namespace disasm
}  // namespace ui
}  // namespace veles
