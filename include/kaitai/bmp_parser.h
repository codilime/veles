#include "kaitai/bmp.h"
#include "parser/parser.h"
namespace veles {
namespace kaitai {
class BmpParser : public parser::Parser {
 public:
  BmpParser() : parser::Parser("bmp (ksy)") {}
  void parse(
      const dbif::ObjectHandle& blob, uint64_t start = 0,
      const dbif::ObjectHandle& parent_chunk = dbif::ObjectHandle()) override {
    try {
      auto stream = kaitai::kstream(blob, start, parent_chunk);
      auto parser = kaitai::bmp::bmp_t(&stream);
      parser.image();
    } catch (std::exception) {
    }
  }
};

}  // namespace kaitai
}  // namespace veles
