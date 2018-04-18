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

namespace veles {
namespace ui {
namespace disasm {

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

void Widget::scrollbarChanged(int value) {
  if (value < 0) {
    return;
  }
  scroll_bar_current_ = value;

  auto pos = blob_->getPosition(value);
  pos.waitForFinished();

  auto rows_count = static_cast<unsigned int>((rowsCount() / 2) + 2);

  window_->seek(pos.result(), rows_count, rows_count);
  updateRows(window_->entries());
}

void Widget::setupMocks() {
  mocks::ChunkTreeFactory ctf;

  std::unique_ptr<mocks::ChunkNode> root =
      ctf.generateTree(mocks::ChunkType::FILE);
  ctf.setAddresses(root.get(), 0, 0x1000);

  std::shared_ptr<mocks::ChunkNode> sroot{root.release()};

  std::unique_ptr<mocks::MockBlob> mb =
      std::make_unique<mocks::MockBlob>(sroot);
  blob_ = std::unique_ptr<Blob>(std::move(mb));
}

void Widget::getEntrypoint() {
  entrypoint_ = blob_->getEntrypoint();
  entrypoint_watcher_.setFuture(entrypoint_);
  connect(&entrypoint_watcher_, &QFutureWatcher<Bookmark>::finished, this,
          &Widget::getWindow);
}

void Widget::resizeEvent(QResizeEvent*) {
  scrollbarChanged(scroll_bar_current_);
}

void Widget::getWindow() {
  entrypoint_.waitForFinished();
  Bookmark entrypoint = entrypoint_.result();

  auto rows_count = static_cast<unsigned int>(rowsCount());
  std::unique_ptr<Window> w =
      blob_->createWindow(entrypoint, rows_count, rows_count);
  window_ = std::make_unique<WindowCache>(std::move(w));

  connect(window_.get(), &WindowCache::dataChanged, this, &Widget::renderRows);

  scroll_bar_current_ = static_cast<int>(window_->currentScrollbarIndex());
  int max_index = static_cast<int>(window_->maxScrollbarIndex());

  auto entries = window_->entries();
  updateRows(entries);

  setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOn);
  scroll_bar_->setValue(scroll_bar_current_);
  scroll_bar_->setRange(0, max_index);
  scroll_bar_->setSingleStep(1);
  scroll_bar_->setPageStep(10);
  scroll_bar_->setTracking(false);

  connect(verticalScrollBar(), &QScrollBar::valueChanged, this,
          &Widget::scrollbarChanged);
}

void Widget::renderRows() {
  int max_index = static_cast<int>(window_->maxScrollbarIndex());
  scroll_bar_->setRange(0, max_index);
  updateRows(window_->entries());
}

ChunkID get_target(TextRepr *repr)
{
    if(dynamic_cast<Sublist*>(repr) != nullptr) {
        auto sublist = dynamic_cast<Sublist*>(repr);
        for (const auto& chld : sublist->children()) {
            auto ret = get_target(chld.get());
            if (ret != "")
                return ret;
        }
    } else if (dynamic_cast<Keyword*>(repr) != nullptr) {
        auto keyword = dynamic_cast<Keyword*>(repr);
        return keyword->chunkID();
    }
    return "";
}

void Widget::updateRows(std::vector<std::shared_ptr<Entry>> entries) {
  while (rows_.size() < entries.size()) {
    auto r = new Row();
    rows_layout_->addWidget(r, 0, Qt::AlignTop);
    rows_.push_back(r);

    connect(r, &Row::chunkCollapse, this, &Widget::chunkCollapse);
  }

  while (rows_.size() > entries.size()) {
    Row* row = rows_.back();
    rows_.pop_back();
    rows_layout_->removeWidget(row);
    delete row;
  }

  std::map<ChunkID, int> row_mapping;

  int indent_level = 0;
  for (size_t i = 0; i < entries.size(); i++) {
    auto entry = entries[i];
    auto row = static_cast<Row*>(rows_layout_->itemAt(i)->widget());

    switch (entry->type()) {
      case EntryType::CHUNK_COLLAPSED: {
        auto* ent = static_cast<EntryChunkCollapsed const*>(entry.get());
        row->setEntry(ent);
        row->setIndent(indent_level);
        break;
      }
      case EntryType::CHUNK_BEGIN: {
        auto* ent = static_cast<EntryChunkBegin const*>(entry.get());
        row_mapping[ent->chunk->id] = i;
        row->setEntry(ent);
        row->setIndent(indent_level);
        indent_level++;
        break;
      }
      case EntryType::CHUNK_END: {
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

  std::vector<Arrow> arrows_vec;
  for (size_t i = 0; i < entries.size(); i++) {
    auto entry = entries[i];
    if (entry->type() == EntryType::CHUNK_COLLAPSED) {
      auto* ent = static_cast<EntryChunkCollapsed const*>(entry.get());
      auto target = get_target(ent->chunk->text_repr.get());
      if (target == "") continue;
      if (row_mapping.find(target) == row_mapping.end())
          continue;
      auto arr = Arrow(i, row_mapping[target], rand() % 5 +1);
      std::cerr << arr << std::endl;
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
}

void Widget::toggleColumn(Row::ColumnName column_name) {
  auto rows = this->findChildren<Row*>();
  std::for_each(rows.begin(), rows.end(),
                [&column_name](Row* row) { row->toggleColumn(column_name); });
}

void Widget::chunkCollapse(const ChunkID& id) {
  window_->chunkCollapseToggle(id);
  scrollbarChanged(scroll_bar_current_);
}

int Widget::rowsCount() { return viewport()->height() / ROW_HEIGHT; }

void Widget::selectionChange(const TextRepr* repr) {
  current_selection_ = repr;
  emit labelSelectionChange(repr);
}
const TextRepr* Widget::current_selection() { return current_selection_; }

}  // namespace disasm
}  // namespace ui
}  // namespace veles
