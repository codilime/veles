#include "ui/disasm/disasm.h"

namespace veles {
namespace ui {
namespace disasm {

Entry::~Entry() {}

EntryChunkBegin::EntryChunkBegin(std::shared_ptr<Chunk> c)
    : chunk{std::move(c)} {}

EntryChunkEnd::EntryChunkEnd(std::shared_ptr<Chunk> c) : chunk{std::move(c)} {}

EntryType EntryChunkBegin::type() const { return EntryType::CHUNK_BEGIN; }
EntryType EntryChunkEnd::type() const { return EntryType::CHUNK_END; }
EntryType EntryOverlap::type() const { return EntryType::OVERLAP; }
EntryType EntryLooseData::type() const { return EntryType::LOOSE_DATA; }
EntryType EntryField::type() const { return EntryType::FIELD; }
EntryType EntryComputedField::type() const { return EntryType::COMPUTED_FIELD; }
EntryType EntryBitField::type() const { return EntryType::BIT_FIELD; }

EntryField::EntryField(std::shared_ptr<Chunk> c) {
  begin = c->addr_begin;
  end = c->addr_end;
}

FieldValueString::FieldValueString(const QString& s) : value{s} {}

}  // namespace disasm
}  // namespace ui
}  // namespace veles
