#pragma once
#include "ui/disasm/disasm.h"
#include "ui/disasm/mocks.h"
namespace veles {
namespace ui {
namespace disasm {
namespace mocks {

class Mock_test_map : public MockBackend {
 public:
  explicit Mock_test_map();
  ChunkMeta* make_chunk(ChunkID id, ChunkID parent,
                                        Bookmark pos_begin, Bookmark pos_end,
                                        Address addr_begin, Address addr_end,
                                        QString type, ChunkType meta_type,
                                        QString display_name,
                                        TextRepr* text_repr,
                                        QString comment);

  ChunkNode* gibRoot();

 private:
  ChunkNode* root_;
};
}
}
}
}
