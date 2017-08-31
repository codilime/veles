#include "kaitai/microsoft_pe.h"
#include "parser/parser.h"
namespace veles {
namespace kaitai {
class Microsoft_peParser : public parser::Parser {
 public:
  Microsoft_peParser() : parser::Parser("microsoft_pe (ksy)") {}
  void parse(const dbif::ObjectHandle& blob, uint64_t start,
             const dbif::ObjectHandle& parent_chunk) override {
    try {
      auto stream = kaitai::kstream(blob, start, parent_chunk);
      auto parser = kaitai::microsoft_pe::microsoft_pe_t(&stream);
    } catch (const std::exception&) {
    }
  }
};

}  // namespace kaitai
}  // namespace veles
