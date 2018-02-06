/*
 * Copyright 2018 CodiLime
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

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
