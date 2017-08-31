#include "kaitai/avi.h"
#include "parser/parser.h"
namespace veles {
namespace kaitai {
class AviParser : public parser::Parser {
 public:
  AviParser() : parser::Parser("avi (ksy)") {}
  void parse(const dbif::ObjectHandle& blob, uint64_t start,
             const dbif::ObjectHandle& parent_chunk) override {
    try {
      auto stream = kaitai::kstream(blob, start, parent_chunk);
      auto parser = kaitai::avi::avi_t(&stream);
    } catch (const std::exception&) {
    }
  }
};

}  // namespace kaitai
}  // namespace veles
