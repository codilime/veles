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
  setupMocks();
  getEntrypoint();

  setWidgetResizable(true);
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

  auto split_view = new QWidget;
  split_view->setLayout(split_layout);
  setWidget(split_view);
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

void Widget::getWindow() {
  Bookmark entrypoint = entrypoint_.result();
  window_ = blob_->createWindow(entrypoint, 2, 10);
  connect(window_.get(), &Window::dataChanged, this, &Widget::updateRows);

  auto entries = window_->entries();
  generateRows(entries);
}

void Widget::updateRows() { generateRows(window_->entries()); }

void Widget::generateRows(std::vector<std::shared_ptr<Entry>> entries) {
  while (rows_.size() < entries.size()) {
    auto r = new Row();
    rows_layout_->addWidget(r, 0, Qt::AlignTop);
    rows_.push_back(r);

    connect(r, &Row::chunkCollapse, this, &Widget::chunkCollapse);
  }

  Row* row;
  while (rows_.size() > entries.size()) {
    row = rows_.back();
    rows_.pop_back();
    rows_layout_->removeWidget(row);
    delete row;
  }

  int indent_level = 0;
  for (size_t i = 0; i < entries.size(); i++) {
    auto entry = entries[i];
    row = static_cast<Row*>(rows_layout_->itemAt(i)->widget());

    switch (entry->type()) {
      case EntryType::CHUNK_COLLAPSED: {
        auto* ent = static_cast<EntryChunkCollapsed const*>(entry.get());
        row->setEntry(ent);
        row->setIndent(indent_level);
        break;
      }
      case EntryType::CHUNK_BEGIN: {
        auto* ent = static_cast<EntryChunkBegin const*>(entry.get());
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

  // TODO(zpp) row_attach_points_ should be updated when toggling chunk
  std::vector<int> row_attach_points;
  for (auto rowPtr : rows_) {
    row_attach_points.push_back(static_cast<int>(rowPtr->y()) +
                                rowPtr->height() / 2);
  }

  auto& g = veles::util::g_mersenne_twister;
  std::uniform_int_distribution<int> arrow_count_range(5, 30);
  std::uniform_int_distribution<int> max_level_range(1, 30);
  int arrow_count = arrow_count_range(g);
  int max_level = max_level_range(g);
  std::uniform_int_distribution<int> level_range(1, max_level);

  std::uniform_int_distribution<int> row_range(0, rows_.size());
  std::vector<Arrow> arrows_vec;
  for (int i = 0; i < arrow_count; i++) {
    arrows_vec.emplace_back(row_range(g), row_range(g), level_range(g));
  }

  arrows_->updateArrows(row_attach_points, arrows_vec);
  update();
}
void Widget::toggleColumn(Row::ColumnName column_name) {
  auto rows = this->findChildren<Row*>();
  std::for_each(rows.begin(), rows.end(),
                [&column_name](Row* row) { row->toggleColumn(column_name); });
}

void Widget::chunkCollapse(const ChunkID& id) {
  window_->chunkCollapseToggle(id);
}

}  // namespace disasm
}  // namespace ui
}  // namespace veles
