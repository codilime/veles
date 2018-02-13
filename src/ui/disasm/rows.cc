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

#include "ui/disasm/rows.h"

namespace veles {
namespace ui {
namespace disasm {

Rows::Rows() {
  layout_ = new QVBoxLayout;
  layout_->setSpacing(0);
  layout_->setMargin(0);

  setLayout(layout_);
}

void Rows::generate(std::vector<std::shared_ptr<Entry>> entries) {
  int indent_level = 0;
  for (const auto& entry : entries) {
    switch (entry->type()) {
      case EntryType::CHUNK_BEGIN: {
        auto* ent = reinterpret_cast<EntryChunkBegin const*>(entry.get());
        auto r = new Row(indent_level);
        r->setEntry(ent);
        this->layout_->addWidget(r, 0, Qt::AlignTop);

        indent_level++;
        break;
      }
      case EntryType::CHUNK_END: {
        indent_level--;
        auto* ent = reinterpret_cast<EntryChunkEnd const*>(entry.get());
        if (ent->chunk->collapsed) {
          break;
        }
        auto r = new Row(indent_level);
        r->setEntry(ent);
        this->layout_->addWidget(r, 0, Qt::AlignTop);
        break;
      }
      case EntryType::OVERLAP: {
        auto* ent = reinterpret_cast<EntryOverlap const*>(entry.get());
        auto r = new Row(indent_level);
        r->setEntry(ent);
        this->layout_->addWidget(r, 0, Qt::AlignTop);
        break;
      }
      case EntryType::FIELD: {
        auto* ent = reinterpret_cast<EntryField const*>(entry.get());
        auto r = new Row(indent_level);
        r->setEntry(ent);
        this->layout_->addWidget(r, 0, Qt::AlignTop);
        break;
      }
      default: { break; }
    }
  }
};

}  // namespace disasm
}  // namespace ui
}  // namespace veles
