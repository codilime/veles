#include "kaitai/zip.h"
#include "parser/parser.h"
namespace veles {
namespace kaitai {
class ZipParser : public parser::Parser {
 public:
  ZipParser() : parser::Parser("zip (ksy)") {}
  void parse(const dbif::ObjectHandle& blob, uint64_t start,
             const dbif::ObjectHandle& parent_chunk) override {
    try {
      auto stream = kaitai::kstream(blob, start, parent_chunk);
      auto parser = kaitai::zip::zip_t(&stream);
    } catch (const std::exception&) {
    }
  }
};

}  // namespace kaitai
}  // namespace veles
