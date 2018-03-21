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

#include <iostream>

#include <QObject>
#include <QScrollArea>
#include <QScrollBar>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>

#include "ui/disasm/arrows.h"
#include "ui/disasm/disasm.h"
#include "ui/disasm/mocks.h"
#include "ui/disasm/row.h"
#include "ui/disasm/window.h"
#include "util/settings/theme.h"

namespace veles {
namespace ui {
namespace disasm {

/*
 * Container for component widgets (arrows column,
 * disassembled code column, etc).
 * Responsible for rendering them in right position.
 */
class Widget : public QAbstractScrollArea {
  Q_OBJECT

 public slots:
  void getWindow();
  void renderRows();
  void chunkCollapse(const ChunkID& id);
  void toggleColumn(Row::ColumnName column_name);

 public:
  Widget();

  void setupMocks();
  void getEntrypoint();

  void resizeEvent(QResizeEvent* event) override;

 protected:
  void scrollbarChanged(int value);

 private:
  int rowsCount();
  void updateRows(std::vector<std::shared_ptr<Entry>> entries);

  QFuture<Bookmark> entrypoint_;
  QFutureWatcher<Bookmark> entrypoint_watcher_;

  std::unique_ptr<Blob> blob_;
  std::unique_ptr<WindowCache> window_;

  ArrowsWidget* arrows_;

  std::vector<Row*> rows_;
  QVBoxLayout* rows_layout_;

  QScrollBar* scroll_bar_;
  int scroll_bar_current_;
};

}  // namespace disasm
}  // namespace ui
}  // namespace veles
