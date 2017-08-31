#include "kaitai/gif.h"
#include "parser/parser.h"
namespace veles {
namespace kaitai {
class GifParser : public parser::Parser {
 public:
  GifParser() : parser::Parser("gif (ksy)") {}
  void parse(const dbif::ObjectHandle& blob, uint64_t start,
             const dbif::ObjectHandle& parent_chunk) override {
    try {
      auto stream = kaitai::kstream(blob, start, parent_chunk);
      auto parser = kaitai::gif::gif_t(&stream);
    } catch (const std::exception&) {
    }
  }
};

}  // namespace kaitai
}  // namespace veles
