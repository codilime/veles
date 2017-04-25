#include "parser/parser.h"
#include "kaitai/gif.h"
namespace veles {
namespace kaitai {
class GifParser : public parser::Parser {
public:
    GifParser() : parser::Parser("gif (ksy)") {}
    void parse(dbif::ObjectHandle blob, uint64_t start = 0, 
    dbif::ObjectHandle parent_chunk = dbif::ObjectHandle()) override {
        try {
            auto stream = kaitai::kstream(blob, start, parent_chunk);
            auto parser = kaitai::gif::gif_t(&stream);
        } catch(std::exception) {}
    }
};

}  // namespace kaitai
}  // namespace veles
