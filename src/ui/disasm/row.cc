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
#include <QtWidgets/QHBoxLayout>

namespace veles {
namespace ui {
namespace disasm {

Row::Row(const Entry& e) {
  label = new QLabel;
  layout = new QHBoxLayout();

  switch (e.type()) {
    case EntryType::CHUNK_BEGIN: {
      auto* ent = reinterpret_cast<EntryChunkBegin const*>(&e);
      label->setText(QString(ent->chunk->text_repr->string()));
      break;
    }
    case EntryType::CHUNK_END: {
      auto* ent = reinterpret_cast<EntryChunkEnd const*>(&e);
      label->setText(QString(ent->chunk->id));
      break;
    }
    case EntryType::OVERLAP: {
      label->setText("Overlap");
      break;
    }
    case EntryType::FIELD: {
      auto* ent = reinterpret_cast<EntryField const*>(&e);
      auto cent = const_cast<EntryField*>(ent);
      label->setText(QString(EntryFieldStringRepresentation(cent).c_str()));
      break;
    }
    default: { break; }
  }

  layout->addWidget(label);
  setLayout(layout);
}

}  // namespace disasm
}  // namespace ui
}  // namespace veles
