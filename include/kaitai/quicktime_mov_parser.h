#include "parser/parser.h"
#include "kaitai/quicktime_mov.h"
namespace veles {
namespace kaitai {
class Quicktime_movParser : public parser::Parser {
public:
    Quicktime_movParser() : parser::Parser("quicktime_mov (ksy)") {}
    void parse(dbif::ObjectHandle blob, uint64_t start = 0, 
    dbif::ObjectHandle parent_chunk = dbif::ObjectHandle()) override {
        auto stream = kaitai::kstream(blob, start, parent_chunk);
        auto parser = kaitai::quicktime_mov::quicktime_mov_t(&stream);
    }
};

} // namespace kaitai
} // namespace veles
