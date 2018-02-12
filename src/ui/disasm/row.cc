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

#include "ui/disasm/row.h"

namespace veles {
namespace ui {
namespace disasm {

Row::Row(const Entry& e) {
  layout_ = new QHBoxLayout();
  layout_->setSpacing(0);
  layout_->setMargin(0);

  text_ = new QLabel;
  comment_ = new QLabel;
  address_ = new QLabel;

  address_->setMaximumWidth(100);

  switch (e.type()) {
    case EntryType::CHUNK_BEGIN: {
      auto* ent = reinterpret_cast<EntryChunkBegin const*>(&e);
      address_->setText(QString("%1").arg(ent->chunk->addr_begin, 8, 16, QChar('0')));
      text_->setText(QString(ent->chunk->text_repr->string() + " {"));
      comment_->setText("; " + ent->chunk->comment);
      break;
    }
    case EntryType::CHUNK_END: {
      auto* ent = reinterpret_cast<EntryChunkEnd const*>(&e);
      address_->setText(QString("%1").arg(ent->chunk->addr_end, 8, 16, QChar('0')));
      text_->setText("}");
      break;
    }
    case EntryType::OVERLAP: {
      text_->setText("Overlap");
      break;
    }
    case EntryType::FIELD: {
      auto* ent = reinterpret_cast<EntryField const*>(&e);
      auto cent = const_cast<EntryField*>(ent);
      text_->setText(QString(EntryFieldStringRepresentation(cent).c_str()));
      break;
    }
    default: { break; }
  }

  layout_->addWidget(address_);
  layout_->addWidget(text_);
  layout_->addWidget(comment_);

  setLayout(layout_);
}

}  // namespace disasm
}  // namespace ui
}  // namespace veles
