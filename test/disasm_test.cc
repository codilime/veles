#include <iomanip>
#include <iostream>

#include "ui/disasm/asmgen.h"
#include "ui/disasm/disasm.h"
#include "ui/disasm/mocks.h"

std::ostream& hex_print(std::ostream& os, int64_t x) {
  return os << "0x" << std::setfill('0') << std::setw(4) << std::hex << x
            << std::dec;
}

using veles::ui::disasm::Entry;
using veles::ui::disasm::EntryChunkBegin;
using veles::ui::disasm::EntryChunkEnd;
using veles::ui::disasm::EntryField;
using veles::ui::disasm::EntryType;
using veles::ui::disasm::FieldType;
using veles::ui::disasm::FieldValueString;
using veles::ui::disasm::Window;
using veles::ui::disasm::mocks::ChunkTreeFactory;
using veles::ui::disasm::mocks::ChunkType;
using veles::ui::disasm::mocks::ChunkNode;
using veles::ui::disasm::mocks::ChunkMeta;
using veles::ui::disasm::mocks::EntryFactory;
using veles::ui::disasm::mocks::MockWindow;
using veles::ui::disasm::mocks::MockBlob;

std::string fieldStringRepresentation(EntryField* field_entry) {
  switch (field_entry->field_type) {
    case FieldType::STRING: {
      auto* fvs = reinterpret_cast<FieldValueString*>(field_entry->value.get());
      return fvs->value.toStdString();
    }
    default: { break; }
  }
  return "[CANNOT DISPLAY AS STRING]";
}

std::ostream& operator<<(std::ostream& os, Entry* entry) {
  switch (entry->type()) {
    case EntryType::CHUNK_BEGIN: {
      auto* ent = reinterpret_cast<EntryChunkBegin*>(entry);
      hex_print(os, ent->chunk->addr_begin);
      os << " ChunkBegin(id: " << ent->chunk->id.toStdString()
         << ", type: " << ent->chunk->type.toStdString() << ") ";
      os << ent->chunk->text_repr->string().toStdString();
      break;
    }
    case EntryType::CHUNK_END: {
      auto* ent = reinterpret_cast<EntryChunkEnd*>(entry);
      hex_print(os, ent->chunk->addr_end);
      os << " ChunkEnd(id: " << ent->chunk->id.toStdString() << ")";
      break;
    }
    case EntryType::OVERLAP: {
      os << "Overlap()";
      break;
    }
    case EntryType::FIELD: {
      auto* ent = reinterpret_cast<EntryField*>(entry);
      os << fieldStringRepresentation(ent);
      break;
    }
    default:
      os << "[UNKNOWN]";
  }
  return os;
}

void entry_printer(const std::vector<std::shared_ptr<Entry>>& entries) {
  int indent = 0;
  for (auto& entry : entries) {
    auto e = entry.get();
    if (e->type() == EntryType::CHUNK_END) indent--;
    for (int i = 0; i < indent; i++) std::cerr << "  ";
    std::cerr << e << std::endl;
    if (e->type() == EntryType::CHUNK_BEGIN) indent++;
  }
}

int main() {
  ChunkTreeFactory ctf;

  std::unique_ptr<ChunkNode> root = ctf.generateTree(ChunkType::FILE);
  ctf.setAddresses(root.get(), 0, 0x1000);

  std::shared_ptr<ChunkNode> sroot{root.release()};

  MockBlob blob(sroot);

  auto entrypoint_future = blob.getEntrypoint();
  auto entrypoint = entrypoint_future.result();

  std::unique_ptr<Window> window = blob.createWindow(entrypoint, 2, 10);
  entry_printer(window->entries());

  std::cerr << "Scrollbar Index: " << window->currentScrollbarIndex() << " / "
            << window->maxScrollbarIndex() << std::endl;

  std::cout << "ACTION: COLLAPSE CHUNK ROOT" << std::endl;
  auto done = window->chunkCollapseToggle(sroot.get()->chunk()->id);
  done.waitForFinished();

  entry_printer(window->entries());

  std::cout << "ACTION: UNCOLLAPSE CHUNK ROOT" << std::endl;
  done = window->chunkCollapseToggle(sroot.get()->chunk()->id);
  done.waitForFinished();

  std::cout << "entries.size() = " << window->entries().size() << std::endl;
}
