#include "parser/parser.h"
#include "kaitai/microsoft_pe.h"
namespace veles {
namespace kaitai {
class Microsoft_peParser : public parser::Parser {
public:
    Microsoft_peParser() : parser::Parser("microsoft_pe (ksy)") {}
    void parse(dbif::ObjectHandle blob, uint64_t start = 0, 
    dbif::ObjectHandle parent_chunk = dbif::ObjectHandle()) override {
        auto stream = kaitai::kstream(blob, start, parent_chunk);
        auto parser = kaitai::microsoft_pe::microsoft_pe_t(&stream);
    }
};

} // namespace kaitai
} // namespace veles
