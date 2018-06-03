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
#include "util/settings/theme.h"

namespace veles {
namespace ui {
namespace disasm {

// ENTRIES_PREFETCH_FACTOR defines how much more entries should be fetched
// from backend and kept in cache.
const unsigned int ENTRIES_PREFETCH_FACTOR = 20;
// ENTRIES_PREFETCH_DIFFERENCE defines when to prefetch more entries.
// 1 means that when difference between window_index and current scroll index is
// less
// or equal to the viewport height.
const int ENTRIES_PREFETCH_DIFFERENCE = 5;

/*
 * Container for component widgets (arrows column,
 * disassembled code column, etc).
 * Responsible for rendering them in right position.
 */
class Widget : public QAbstractScrollArea {
  Q_OBJECT

 signals:
  void labelSelectionChange(const TextRepr*);

 public slots:
  void dataChanged();
  void getWindow();

  void chunkCollapse(const ChunkID& id);
  void toggleColumn(Row::ColumnName column_name);

 public:
  Widget();

  void setupMocks();
  void getEntrypoint();
  void selectionChange(const TextRepr* repr);
  const TextRepr* current_selection();
  void resizeEvent(QResizeEvent* event) override;
  void goToChunk(const ChunkID& id);

 protected:
  void renderRows();

  // scrollbarChanged should be called whenever scroll bar index has changed
  void scrollbarChanged(int scroll_index);

  // entriesSeek sets window at given index
  void entriesSeek(const Bookmark& position);
  void entriesSeekByIndex(int index);

  // entriesFetch fetches entries from backend and then computes variables
  void entriesFetch();
  // entriesFetchMaybe contains a logic if entries_ should be updated or not
  // if yes, calls entriesFetch
  bool entriesFetchMaybe(int index);
  // entriesRenderAtIndex renders entries at given scrollbar index
  // all variables (like viewport's height) are handled within this
  // method logic
  void entriesRenderAtIndex(int scroll_index);

  // entriesRangeUpdate updates range of scrollbar indexes which
  // match current vector of entries
  // range.first == scroll bar index should be the first entry in the entries_
  void entriesRangeUpdate(int index, std::pair<int, int> range);
  // entriesRange computes range of scroll indexes considering current
  // viewport height
  std::pair<int, int> entriesRange(int index, int extend_factor);
  // entriesRangeToPrevNext converts real scroll indexes to relative
  // prev_n and next_n compared to the given index
  std::pair<int, int> entriesRangeToPrevNext(int index,
                                             std::pair<int, int> range);

  // rowsCount returns number of potentially visible rows in the viewport
  int rowsCount();
  unsigned int prefetchCount(int factor);

 private:
  int getScrollbarValueByIndex(int index);
  int getScrollbarMaxValue(int max_index);
  int getIndexByScrollbarValue(int scrollbar_index);

  QFuture<Bookmark> entrypoint_;
  QFutureWatcher<Bookmark> entrypoint_watcher_;

  std::unique_ptr<Blob> blob_;
  std::unique_ptr<Window> window_;

  ArrowsWidget* arrows_;

  std::vector<Row*> rows_;
  QVBoxLayout* rows_layout_;

  QScrollBar* scroll_bar_;

  const TextRepr* current_selection_ = nullptr;

  // defines index for which entry is equivalent
  // for window_index at entries_ vector
  size_t entries_window_index_;

  std::pair<int, int> entries_range_index_;
  std::vector<std::shared_ptr<Entry>> entries_;

  int window_index_;
  int window_max_index_;
  int scroll_bar_index_;
  int scroll_bar_max_index_;

  std::mutex mutex_;
};

}  // namespace disasm
}  // namespace ui
}  // namespace veles
