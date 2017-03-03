#include "parser/parser.h"
#include "kaitai/png.h"
namespace veles {
namespace kaitai {
class PngParser : public parser::Parser {
public:
    PngParser() : parser::Parser("png (ksy)") {}
    void parse(dbif::ObjectHandle blob, uint64_t start = 0, 
    dbif::ObjectHandle parent_chunk = dbif::ObjectHandle()) override {
        auto stream = kaitai::kstream(blob, start, parent_chunk);
        auto parser = kaitai::png::png_t(&stream);
    }
};

}  // namespace kaitai
}  // namespace veles
