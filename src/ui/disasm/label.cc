#include "ui/disasm/label.h"

namespace veles {
namespace ui {
namespace disasm {

Label::Label(const Entry& e) : QLabel() {
  switch (e.type()) {
    case EntryType::CHUNK_BEGIN: {
      auto* ent = reinterpret_cast<EntryChunkBegin const*>(&e);
      setText(QString(ent->chunk->text_repr.get()->string()));
      break;
    }
    case EntryType::CHUNK_END: {
      auto* ent = reinterpret_cast<EntryChunkEnd const*>(&e);
      setText(QString(ent->chunk->id));
      break;
    }
    case EntryType::OVERLAP: {
      setText("Overlap");
      break;
    }
    case EntryType::FIELD: {
      auto* ent = reinterpret_cast<EntryField const*>(&e);
      auto cent = const_cast<EntryField*>(ent);
      setText(QString(EntryFieldStringRepresentation(cent).c_str()));
      break;
    }
    default: { break; }
  }
}
}
}
}
