#pragma once

#include <memory>

#include <QFuture>
#include <QObject>
#include <QtConcurrent/QtConcurrent>
#include <QtGlobal>

#include "data/bindata.h"
#include "data/repack.h"

namespace veles {
namespace ui {
namespace disasm {

using Bookmark = QByteArray;
using Address = uint64_t;
using ChunkID = QByteArray;

struct Chunk {
  ChunkID id;
  ChunkID parent_id;

  Bookmark pos_begin;
  Bookmark pos_end;

  Address addr_begin;
  Address addr_end;

  QString display_name;
  QString type;
  QString text_repr;
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
