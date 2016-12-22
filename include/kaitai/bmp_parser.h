#include "parser/parser.h"
#include "kaitai/bmp.h"
namespace veles {
namespace kaitai {
class BmpParser : public parser::Parser {
public:
    BmpParser() : parser::Parser("bmp (ksy)") {}
    void parse(dbif::ObjectHandle blob, uint64_t start = 0, 
    dbif::ObjectHandle parent_chunk = dbif::ObjectHandle()) override {
        auto stream = kaitai::kstream(blob, start, parent_chunk);
        auto parser = kaitai::bmp::bmp_t(&stream);
        parser.image();
    }
};

} // namespace kaitai
} // namespace veles
