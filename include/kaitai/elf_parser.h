#include "kaitai/elf.h"
#include "parser/parser.h"
namespace veles {
namespace kaitai {
class ElfParser : public parser::Parser {
 public:
  ElfParser() : parser::Parser("elf (ksy)") {}
  void parse(const dbif::ObjectHandle& blob, uint64_t start,
             const dbif::ObjectHandle& parent_chunk) override {
    try {
      auto stream = kaitai::kstream(blob, start, parent_chunk);
      auto parser = kaitai::elf::elf_t(&stream);
      parser.program_headers();
      parser.section_headers();
      parser.strings();
    } catch (const std::exception&) {
    }
  }
};

}  // namespace kaitai
}  // namespace veles
