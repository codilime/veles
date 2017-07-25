#include "parser/parser.h"
#include "kaitai/microsoft_pe.h"
namespace veles {
namespace kaitai {
class Microsoft_peParser : public parser::Parser {
public:
    Microsoft_peParser() : parser::Parser("microsoft_pe (ksy)") {}
    void parse(const dbif::ObjectHandle& blob, uint64_t start = 0,
               const dbif::ObjectHandle& parent_chunk = dbif::ObjectHandle()) override {
        try {
            auto stream = kaitai::kstream(blob, start, parent_chunk);
            auto parser = kaitai::microsoft_pe::microsoft_pe_t(&stream);
        } catch(std::exception) {}
    }
};

}  // namespace kaitai
}  // namespace veles
