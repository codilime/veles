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

#include "ui/disasm/widget.h"
#include "ui/disasm/gf100.h"

namespace veles {
namespace ui {
namespace disasm {

ChunkID get_target(TextRepr* repr) {
  if (dynamic_cast<Sublist*>(repr) != nullptr) {
    auto sublist = dynamic_cast<Sublist*>(repr);
    for (const auto& chld : sublist->children()) {
      auto ret = get_target(chld.get());
      if (ret != "") return ret;
    }
  } else if (dynamic_cast<Keyword*>(repr) != nullptr) {
    auto keyword = dynamic_cast<Keyword*>(repr);
    return keyword->chunkID();
  }
  return "";
}

Widget::Widget() {
  setFont(util::settings::theme::font());

  arrows_ = new ArrowsWidget(this);

  rows_layout_ = new QVBoxLayout();
  rows_layout_->setSpacing(0);
  rows_layout_->setContentsMargins(0, 0, 0, 0);

  auto rows_with_stretch = new QVBoxLayout();
  rows_with_stretch->setSpacing(0);
  rows_with_stretch->setContentsMargins(0, 0, 0, 0);
  rows_with_stretch->addLayout(rows_layout_);
  rows_with_stretch->addStretch();

  auto split_layout = new QHBoxLayout;
  split_layout->setSpacing(0);
  split_layout->setMargin(0);
  split_layout->setMargin(0);
  split_layout->addWidget(arrows_, 0, Qt::AlignTop);
  split_layout->addLayout(rows_with_stretch, 0);

  split_layout->setSizeConstraint(QLayout::SetDefaultConstraint);

  auto split_view = new QWidget;
  split_view->setLayout(split_layout);
  setViewport(split_view);

  scroll_bar_ = new QScrollBar();
  setVerticalScrollBar(scroll_bar_);

  setupMocks();
  getEntrypoint();
}

void Widget::scrollbarChanged(int scroll_index) {
  scroll_bar_index_ = getIndexByScrollbarValue(scroll_index);
  entriesRenderAtIndex(scroll_index);
}

void Widget::entriesRenderAtIndex(int scroll_index) {
  if (entriesFetchMaybe(scroll_index)) {
    return;
  }

  renderRows();
}

void Widget::goToChunk(const ChunkID& id) {
  auto x = blob_->getPositionByChunk(id);
  x.waitForFinished();
  entriesSeek(x.result());
}

std::pair<int, int> Widget::entriesRange(int index, int extend_factor) {
  std::pair<int, int> entries_index_range;

  auto rows_count = static_cast<unsigned int>(rowsCount());
  int prev_n = (rows_count / 2) * extend_factor;
  int next_n = (rows_count / 2) * extend_factor;

  prev_n += 1;
  next_n += 1;

  if (index < prev_n) {
    next_n += prev_n - index;
    entries_index_range.first = 0;
  } else {
    entries_index_range.first = index - prev_n;
  }

  if (window_max_index_ < index + next_n) {
    entries_index_range.second = window_max_index_;
  } else {
    entries_index_range.second = index + next_n;
  }

  return entries_index_range;
};

std::pair<int, int> Widget::entriesRangeToPrevNext(int index,
                                                   std::pair<int, int> range) {
  range.first = index - range.first;
  range.second = range.second - index;
  return range;
};

bool Widget::entriesFetchMaybe(int index) {
  auto max_difference = rowsCount() * ENTRIES_PREFETCH_DIFFERENCE;

  if (abs(window_index_ - index) > max_difference) {
    window_index_ = index;
    entriesSeekByIndex(index);
    return true;
  }

  return false;
}

void Widget::entriesSeekByIndex(int index) {
  auto pos = blob_->getPosition(index);
  pos.waitForFinished();

  entriesSeek(pos.result());
}

void Widget::entriesSeek(const Bookmark& bookmark) {
  auto rows_count = static_cast<unsigned int>(rowsCount());
  auto prefetch_count = rows_count * ENTRIES_PREFETCH_FACTOR;

  window_->seek(bookmark, prefetch_count, prefetch_count);
}

void Widget::entriesFetch() {
  window_max_index_ = static_cast<int>(window_->maxScrollbarIndex() - 1);
  window_index_ = static_cast<int>(window_->currentScrollbarIndex());
  scroll_bar_max_index_ = getScrollbarMaxValue(window_max_index_);
  scroll_bar_index_ = getScrollbarValueByIndex(window_index_);

  entries_ = window_->entries();

  scroll_bar_->setValue(scroll_bar_index_);
  scroll_bar_->setRange(0, scroll_bar_max_index_);

  auto entries_range_ =
      entriesRange(window_index_, ENTRIES_PREFETCH_DIFFERENCE);
  entriesRangeUpdate(window_index_, entries_range_);

  auto pos = window_->currentPosition();
  bool found = false;

  for (size_t i = 0; i < entries_.size() && !found; i++) {
    if (entries_[i]->pos == pos) {
      entries_window_index_ = i;
      found = true;
    }
  }
  if (!found) {
    entries_window_index_ = 0;
  }
}

void Widget::setupMocks() {
  auto mockmap = std::make_shared<mocks::Mock_test_map>();
  auto mb = std::make_unique<mocks::MockBlob>(std::unique_ptr<mocks::ChunkNode>(mockmap->gibRoot()));
  blob_ = std::unique_ptr<Blob>(std::move(mb));
}

void Widget::getEntrypoint() {
  entrypoint_ = blob_->getEntrypoint();
  entrypoint_watcher_.setFuture(entrypoint_);
  connect(&entrypoint_watcher_, &QFutureWatcher<Bookmark>::finished, this,
          &Widget::getWindow);
}

void Widget::resizeEvent(QResizeEvent*) {
  scroll_bar_max_index_ = getScrollbarMaxValue(window_max_index_);
  scroll_bar_->setRange(0, scroll_bar_max_index_);

  entriesRenderAtIndex(scroll_bar_index_);
}

void Widget::dataChanged() {
  entriesFetch();
  renderRows();
}

void Widget::getWindow() {
  entrypoint_.waitForFinished();
  Bookmark entrypoint = entrypoint_.result();

  auto prefetch_count = prefetchCount(ENTRIES_PREFETCH_FACTOR);
  window_ = blob_->createWindow(entrypoint, prefetch_count, prefetch_count);

  setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
  scroll_bar_->setSingleStep(1);
  scroll_bar_->setPageStep(10);
  scroll_bar_->setTracking(false);

  entriesFetch();
  renderRows();

  connect(window_.get(), &Window::dataChanged, this, &Widget::dataChanged);
  connect(verticalScrollBar(), &QScrollBar::valueChanged, this,
          &Widget::scrollbarChanged);
}

void Widget::renderRows() {
  auto index = scroll_bar_index_;
  auto range = entriesRange(index, 1);

  auto rows_count = range.second - range.first + 1;
  if (entries_.size() < rows_count) {
    rows_count = static_cast<int>(entries_.size());
  }

  while (rows_.size() < rows_count) {
    auto r = new Row();
    rows_layout_->addWidget(r, 0, Qt::AlignTop);
    rows_.push_back(r);

    connect(r, &Row::chunkCollapse, this, &Widget::chunkCollapse);
  }

  Row* row;
  while (rows_.size() > rows_count) {
    row = rows_.back();
    rows_.pop_back();
    rows_layout_->removeWidget(row);
    delete row;
  }

  std::map<ChunkID, int> row_mapping;

  auto relative_range = entriesRangeToPrevNext(index, range);
  int i = (static_cast<int>(entries_window_index_) -
           (window_index_ - scroll_bar_index_)) -
          relative_range.first;

  int indent_level = 0;
  if (0 <= i && i < entries_.size()) {
    indent_level = window_->entryIndentation(entries_[i]->pos);
  }

  int item_i = 0;
  for (; 0 <= i && i < entries_.size() && item_i < rows_.size();
       i++, item_i++) {
    auto entry = entries_[i];
    row = static_cast<Row*>(rows_layout_->itemAt(item_i)->widget());

    switch (entry->type()) {
      case EntryType::CHUNK_COLLAPSED: {
        auto* ent = static_cast<EntryChunkCollapsed const*>(entry.get());
        row->setEntry(ent);
        row->setIndent(indent_level);
        break;
      }
      case EntryType::CHUNK_BEGIN: {
        auto* ent = static_cast<EntryChunkBegin const*>(entry.get());
        row_mapping[ent->chunk->id] = item_i;
        row->setEntry(ent);
        row->setIndent(indent_level);
        indent_level++;
        break;
      }
      case EntryType::CHUNK_END: {
        if (0 < item_i)
          indent_level--;
        auto* ent = static_cast<EntryChunkEnd const*>(entry.get());
        row->setEntry(ent);
        row->setIndent(indent_level);
        break;
      }
      case EntryType::OVERLAP: {
        auto* ent = static_cast<EntryOverlap const*>(entry.get());
        row->setEntry(ent);
        row->setIndent(indent_level);
        break;
      }
      case EntryType::FIELD: {
        auto* ent = static_cast<EntryField const*>(entry.get());
        row->setEntry(ent);
        row->setIndent(indent_level);
        break;
      }
      default: { break; }
    }
  }

  i = (static_cast<int>(entries_window_index_) -
           (window_index_ - scroll_bar_index_)) -
          relative_range.first;
  item_i = 0;

  std::vector<Arrow> arrows_vec;
  for (; 0 <= i &&i  < entries_.size() && item_i < rows_.size();
         i++, item_i++) {
    auto entry = entries_[i];
    if (entry->type() == EntryType::CHUNK_COLLAPSED) {
      auto* ent = static_cast<EntryChunkCollapsed const*>(entry.get());
      auto target = get_target(ent->chunk->text_repr.get());
      if (target == "") continue;
      if (row_mapping.find(target) == row_mapping.end()) continue;
      auto arr = Arrow(item_i, row_mapping[target]);
      arrows_vec.emplace_back(arr);
    }
  }

  // TODO(zpp) row_attach_points_ should be updated when toggling chunk
  std::vector<int> row_attach_points;
  for (auto rowPtr : rows_) {
    row_attach_points.push_back(rowPtr->y() + rowPtr->height() / 2);
  }
  arrows_->updateArrows(row_attach_points, arrows_vec);

  viewport()->update();

  auto pos = blob_->getPosition(scroll_bar_index_);
  pos.waitForFinished();
  window_->seekPosition(pos);
}

void Widget::toggleColumn(Row::ColumnName column_name) {
  auto rows = this->findChildren<Row*>();
  std::for_each(rows.begin(), rows.end(),
                [&column_name](Row* row) { row->toggleColumn(column_name); });
}

void Widget::chunkCollapse(const ChunkID& id) {
  auto pos = blob_->getPositionByChunk(id);
  pos.waitForFinished();

  window_->seekPosition(pos.result());
  window_->chunkCollapseToggle(id);
}

int Widget::rowsCount() { return viewport()->height() / ROW_HEIGHT; }

void Widget::selectionChange(const TextRepr* repr) {
  current_selection_ = repr;
  emit labelSelectionChange(repr);
}

const TextRepr* Widget::current_selection() { return current_selection_; }

void Widget::entriesRangeUpdate(int index, std::pair<int, int> range) {
  entries_range_index_.first = index - range.first;
  entries_range_index_.second = index + range.second;

  if (entries_range_index_.first < 0) {
    entries_range_index_.first = 0;
  }
  if (window_max_index_ < entries_range_index_.second) {
    entries_range_index_.second = window_max_index_;
  }
}

unsigned int Widget::prefetchCount(int factor) {
  return static_cast<unsigned int>(rowsCount()) * factor;
}

int Widget::getScrollbarValueByIndex(int index) {
  auto value = index;
  if (value > scroll_bar_max_index_) {
    value = scroll_bar_max_index_;
  }

  value = value - (rowsCount() / 2) - 1;
  if (value < 0) {
    value = 0;
  }
  return value;
}

int Widget::getIndexByScrollbarValue(int scrollbar_index) {
  auto index = scrollbar_index;
  if (index > window_max_index_) {
    index = window_max_index_;
  }

  index = index + (rowsCount() / 2) + 1;

  if (index < 0) {
    index = 0;
  }

  return index;
}

int Widget::getScrollbarMaxValue(int max_window_index) {
  auto index = max_window_index - rowsCount() - 1;
  if (index < 0) {
    index = 0;
  }
  return index;
}

}  // namespace disasm
}  // namespace ui
}  // namespace veles
