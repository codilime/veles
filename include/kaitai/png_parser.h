#include "parser/parser.h"
#include "kaitai/png.h"
namespace veles {
namespace kaitai {
class PngParser : public parser::Parser {
public:
    PngParser() : parser::Parser("png (ksy)") {}
    void parse(const dbif::ObjectHandle& blob, uint64_t start = 0, 
               const dbif::ObjectHandle& parent_chunk = dbif::ObjectHandle()) override {
        try {
            auto stream = kaitai::kstream(blob, start, parent_chunk);
            auto parser = kaitai::png::png_t(&stream);
        } catch(std::exception) {}
    }
};

}  // namespace kaitai
}  // namespace veles
