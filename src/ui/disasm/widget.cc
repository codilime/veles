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
#include "ui/disasm/row.h"

#include "util/settings/theme.h"

namespace veles {
namespace ui {
namespace disasm {

Widget::Widget() {
  setupMocks();
  getEntrypoint();

  setWidgetResizable(true);
  setFont(util::settings::theme::font());

  arrows_ = new Arrows;

  rows_ = new QVBoxLayout();
  rows_->setSpacing(0);
  rows_->setContentsMargins(0, 0, 0, 0);

  auto rows_with_stretch = new QVBoxLayout();
  rows_with_stretch->setSpacing(0);
  rows_with_stretch->setContentsMargins(0, 0, 0, 0);
  rows_with_stretch->addLayout(rows_);
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

  auto entries = window_->entries();
  generateRows(entries);
}

void Widget::generateRows(std::vector<std::shared_ptr<Entry>> entries) {
  int indent_level = 0;
  for (const auto& entry : entries) {
    switch (entry->type()) {
      case EntryType::CHUNK_COLLAPSED: {
        auto* ent = reinterpret_cast<EntryChunkCollapsed const*>(entry.get());
        auto r = new Row(indent_level);
        r->setEntry(ent);
        this->rows_->addWidget(r, 0, Qt::AlignTop);
        break;
      }
      case EntryType::CHUNK_BEGIN: {
        auto* ent = reinterpret_cast<EntryChunkBegin const*>(entry.get());
        auto r = new Row(indent_level);
        r->setEntry(ent);
        this->rows_->addWidget(r, 0, Qt::AlignTop);
        indent_level++;
        break;
      }
      case EntryType::CHUNK_END: {
        indent_level--;
        auto* ent = reinterpret_cast<EntryChunkEnd const*>(entry.get());
        auto r = new Row(indent_level);
        r->setEntry(ent);
        this->rows_->addWidget(r, 0, Qt::AlignTop);
        break;
      }
      case EntryType::OVERLAP: {
        auto* ent = reinterpret_cast<EntryOverlap const*>(entry.get());
        auto r = new Row(indent_level);
        r->setEntry(ent);
        this->rows_->addWidget(r, 0, Qt::AlignTop);
        break;
      }
      case EntryType::FIELD: {
        auto* ent = reinterpret_cast<EntryField const*>(entry.get());
        auto r = new Row(indent_level);
        r->setEntry(ent);
        this->rows_->addWidget(r, 0, Qt::AlignTop);
        break;
      }
      default: { break; }
    }
  }
}

}  // namespace disasm
}  // namespace ui
}  // namespace veles
