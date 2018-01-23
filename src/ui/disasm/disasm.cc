#include "ui/disasm/disasm.h"

namespace veles {
namespace ui {
namespace disasm {

Text::Text(QString text, bool highlight) : text_{text}, highlight_{highlight} {}
QString Text::text() { return text_; }
bool Text::highlight() { return highlight_; }
QString Text::string() const { return text_; }

Keyword::Keyword(QString text, KeywordType kc, ChunkID link)
    : text_{text}, keyword_type_{kc}, link_{link} {}
QString Keyword::string() const { return text_; }
QString Keyword::text() const { return text_; }
KeywordType Keyword::keywordType() { return keyword_type_; }
const ChunkID Keyword::chunkID() const { return link_; }

QString Blank::string() const { return " "; }

Number::Number(qint64 value, unsigned bit_width, unsigned base)
    : value_{value}, bit_width_{bit_width}, base_{base} {}
QString Number::string() const { return QString::number(value_); };
const qint64 Number::value() { return value_; }
const unsigned Number::bit_width() { return bit_width_; }
const unsigned Number::base() { return base_; }

String::String(QString text) : text_{text} {}
QString String::string() const { return text_; }
QString String::text() { return text_; }

Sublist::Sublist(std::vector<std::unique_ptr<TextRepr>> children)
    : children_(std::move(children)) {}
Sublist::Sublist() : children_(){};

QString Sublist::string() const {
  QString s;
  for (const auto& c : children_) s += c.get()->string();
  return s;
}
void Sublist::addChild(std::unique_ptr<TextRepr> child) {
  children_.emplace_back(std::move(child));
};
const std::vector<std::unique_ptr<TextRepr>>& Sublist::children() {
  return children_;
}

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
