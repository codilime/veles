#include "kaitai/quicktime_mov.h"
#include "parser/parser.h"
namespace veles {
namespace kaitai {
class Quicktime_movParser : public parser::Parser {
 public:
  Quicktime_movParser() : parser::Parser("quicktime_mov (ksy)") {}
  void parse(
      const dbif::ObjectHandle& blob, uint64_t start = 0,
      const dbif::ObjectHandle& parent_chunk = dbif::ObjectHandle()) override {
    try {
      auto stream = kaitai::kstream(blob, start, parent_chunk);
      auto parser = kaitai::quicktime_mov::quicktime_mov_t(&stream);
    } catch (std::exception) {
    }
  }
};

}  // namespace kaitai
}  // namespace veles
