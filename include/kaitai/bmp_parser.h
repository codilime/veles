#include "kaitai/bmp.h"
#include "parser/parser.h"
namespace veles {
namespace kaitai {
class BmpParser : public parser::Parser {
 public:
  BmpParser() : parser::Parser("bmp (ksy)") {}
  void parse(const dbif::ObjectHandle& blob, uint64_t start,
             const dbif::ObjectHandle& parent_chunk) override {
    try {
      auto stream = kaitai::kstream(blob, start, parent_chunk);
      auto parser = kaitai::bmp::bmp_t(&stream);
      parser.image();
    } catch (const std::exception&) {
    }
  }
};

}  // namespace kaitai
}  // namespace veles
