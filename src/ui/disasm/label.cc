#include "ui/disasm/label.h"

namespace veles {
namespace ui {


DisasmLabel::DisasmLabel(const disasm::Entry& e) : QLabel() {
  switch (e.type()) {
    case disasm::EntryType::CHUNK_BEGIN: {
      auto* ent = reinterpret_cast<disasm::EntryChunkBegin const *>(&e);
      setText(QString(ent->chunk->text_repr.get()->string()));
      break;
    }
    case disasm::EntryType::CHUNK_END: {
      auto* ent = reinterpret_cast<disasm::EntryChunkEnd const *>(&e);
      setText(QString(ent->chunk->id));
      break;
    }
    case disasm::EntryType::OVERLAP: {
      setText("Overlap");
      break;
    }
    case disasm::EntryType::FIELD: {
      auto* ent = reinterpret_cast<disasm::EntryField const *>(&e);
      auto cent = const_cast<disasm::EntryField *>(ent);
      setText(QString(disasm::EntryFieldStringRepresentation(cent).c_str()));
      break;
    }
    default: {
      break;
    }
  }
}

}
}
