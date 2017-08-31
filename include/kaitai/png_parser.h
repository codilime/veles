#include "kaitai/png.h"
#include "parser/parser.h"
namespace veles {
namespace kaitai {
class PngParser : public parser::Parser {
 public:
  PngParser() : parser::Parser("png (ksy)") {}
  void parse(const dbif::ObjectHandle& blob, uint64_t start,
             const dbif::ObjectHandle& parent_chunk) override {
    try {
      auto stream = kaitai::kstream(blob, start, parent_chunk);
      auto parser = kaitai::png::png_t(&stream);
    } catch (const std::exception&) {
    }
  }
};

}  // namespace kaitai
}  // namespace veles
