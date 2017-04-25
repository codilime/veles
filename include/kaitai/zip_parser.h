#include "parser/parser.h"
#include "kaitai/zip.h"
namespace veles {
namespace kaitai {
class ZipParser : public parser::Parser {
public:
    ZipParser() : parser::Parser("zip (ksy)") {}
    void parse(dbif::ObjectHandle blob, uint64_t start = 0, 
    dbif::ObjectHandle parent_chunk = dbif::ObjectHandle()) override {
        try {
            auto stream = kaitai::kstream(blob, start, parent_chunk);
            auto parser = kaitai::zip::zip_t(&stream);
        } catch(std::exception) {}
    }
};

}  // namespace kaitai
}  // namespace veles
