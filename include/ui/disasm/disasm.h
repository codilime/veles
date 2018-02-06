#pragma once

#include <memory>

#include <QFuture>
#include <QObject>
#include <QtConcurrent/QtConcurrent>
#include <QtGlobal>

#include "data/bindata.h"
#include "data/repack.h"

//TODO REMOVE
#include <iostream>

namespace veles {
namespace ui {
namespace disasm {

using Bookmark = QByteArray;
using Address = uint64_t;
using ChunkID = QByteArray;

class TextRepr {
 public:
  virtual ~TextRepr() = default;
  virtual QString string() const = 0;

  TextRepr* parent;
};

class Text : public TextRepr {
 public:
  Text(QString text, bool highlight);
  Text() = default;

  QString string() const override;
  QString text();
  bool highlight();

 private:
  QString text_;
  bool highlight_ = false;
};

enum class KeywordType { OPCODE, MODIFIER, LABEL, REGISTER };

class Keyword : public TextRepr {
 public:
  Keyword(QString text, KeywordType kc, ChunkID link);

  QString string() const override;
  QString text() const;
  KeywordType keywordType();
  const ChunkID chunkID() const;

 private:
  QString text_;
  KeywordType keyword_type_;
  ChunkID link_;
};

class Blank : public TextRepr {
 public:
  QString string() const override;
};

class Number : public TextRepr {
 public:
  Number(qint64 value, unsigned bit_width, unsigned base);

  QString string() const override;
  const qint64 value();
  const unsigned bit_width();
  const unsigned base();

 private:
  // TODO(mkow): value - this is temporary
  qint64 value_;

  unsigned bit_width_;
  unsigned base_;
};

class String : public TextRepr {
 public:
  explicit String(QString text);

  QString string() const override;
  QString text();

 private:
  // We do only support QString. For now.
  // TODO(mkow)
  QString text_;
};

class Sublist : public TextRepr {
 public:
  explicit Sublist(std::vector<std::unique_ptr<TextRepr>> children);
  explicit Sublist();

  QString string() const override;
  void addChild(std::unique_ptr<TextRepr> child);
  const std::vector<std::unique_ptr<TextRepr>>& children();

 private:
  std::vector<std::unique_ptr<TextRepr>> children_;
};

struct Chunk {
  ChunkID id;
  ChunkID parent_id;

  Bookmark pos_begin;
  Bookmark pos_end;

  Address addr_begin;
  Address addr_end;

  QString type;
  QString display_name;
  std::unique_ptr<TextRepr> text_repr;

  QString comment;
  uint64_t flags;
};

enum class EntryType {
  CHUNK_BEGIN,
  CHUNK_END,
  OVERLAP,
  LOOSE_DATA,
  FIELD,
  COMPUTED_FIELD,
  BIT_FIELD
};

struct Entry {
  virtual EntryType type() const = 0;
  virtual ~Entry();

  Bookmark pos;
};

struct EntryChunkBegin : Entry {
  explicit EntryChunkBegin(std::shared_ptr<Chunk> c);
  EntryType type() const override;

  std::shared_ptr<Chunk> chunk;
};

struct EntryChunkEnd : Entry {
  explicit EntryChunkEnd(std::shared_ptr<Chunk> c);
  EntryType type() const override;

  std::shared_ptr<Chunk> chunk;
};

struct EntryOverlap : Entry {
  EntryType type() const override;

  Address begin;
  Address end;
};

struct EntryLooseData : Entry {
  EntryType type() const override;

  Address begin;
  Address end;
  veles::data::BinData data;
};

enum class FieldType {
  UNKNOWN,
  FIXED,
  FLOAT,
  STRING,
  REF,
  COMPOSED,
};

struct FieldValue {};
struct FieldValueInt : FieldValue {};
struct FieldValueFloat : FieldValue {};
struct FieldValueRef : FieldValue {};

struct FieldValueString : FieldValue {
  explicit FieldValueString(const QString& s);

  QString value;
};

struct EntryField : Entry {
  explicit EntryField(std::shared_ptr<Chunk> c);
  EntryType type() const override;

  QString name;
  Address begin;
  Address end;
  size_t num_elements;
  FieldType field_type;
  veles::data::BinData raw_bytes;

  veles::data::Repacker repacker;
  std::unique_ptr<FieldValue> value;
};

struct EntryComputedField : Entry {
  EntryType type() const override;

  std::unique_ptr<FieldValue> value;
};

struct EntryBitField : Entry {
  EntryType type() const override;

  std::unique_ptr<FieldValue> value;
};

std::string EntryFieldStringRepresentation(EntryField* field_entry);

using ScrollbarIndex = uint64_t;

class Window : public QObject {
  Q_OBJECT

 public:
  virtual void seek(const Bookmark& pos, unsigned prev_n, unsigned next_n) = 0;
  virtual Bookmark currentPosition() = 0;
  virtual ScrollbarIndex currentScrollbarIndex() = 0;
  virtual ScrollbarIndex maxScrollbarIndex() = 0;
  virtual const std::vector<Chunk>& breadcrumbs() = 0;
  virtual const std::vector<std::shared_ptr<Entry>> entries() = 0;
  virtual QFuture<void> chunkCollapseToggle(const ChunkID& id) = 0;

  virtual ~Window() {};

 signals:
  void updateScrollbar();
  void dataChanged();
};

class Blob {
 public:
  virtual std::unique_ptr<Window> createWindow(const Bookmark& pos,
                                               unsigned prev_n,
                                               unsigned next_n) = 0;
  virtual QFuture<Bookmark> getEntrypoint() = 0;
  virtual QFuture<Bookmark> getPosition(ScrollbarIndex index) = 0;
  virtual QFuture<Bookmark> getPositionByChunk(const ChunkID& chunk) = 0;

  virtual ~Blob(){};
};

}  // namespace disasm
}  // namespace ui
}  // namespace veles
