/*
 * Copyright 2016 CodiLime
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#ifndef VELES_PARSER_STREAM_H
#define VELES_PARSER_STREAM_H

#include "dbif/types.h"
#include "dbif/universe.h"
#include "dbif/info.h"
#include "data/repack.h"

namespace veles {
namespace parser {

class StreamParser {
  dbif::ObjectHandle blob_;
  uint64_t pos_;

  struct WorkChunk {
    dbif::ObjectHandle chunk;
    uint64_t start;
    QString type;
    QString name;
    std::vector<data::ChunkDataItem> items;
  };

  std::vector<WorkChunk> stack_;
  unsigned width_;
  size_t blob_size_;

 public:
  StreamParser(dbif::ObjectHandle blob, uint64_t start) :
    blob_(blob), pos_(start) {
    auto desc = blob_->syncGetInfo<dbif::DescriptionRequest>();
    width_ = desc.dynamicCast<dbif::BlobDescriptionReply>()->width;
    blob_size_ = desc.dynamicCast<dbif::BlobDescriptionReply>()->size;
  }

  dbif::ObjectHandle startChunk(const QString &type, const QString &name) {
    dbif::ObjectHandle parent;
    if (stack_.size())
      parent = stack_.back().chunk;
    dbif::ObjectHandle chunk = blob_->syncRunMethod<dbif::ChunkCreateRequest>(
      name, type, parent, pos_, pos_)->object;
    stack_.push_back(WorkChunk{chunk, pos_, type, name, std::vector<data::ChunkDataItem>()});
    return chunk;
  }

  dbif::ObjectHandle endChunk() {
    auto &top = stack_.back();
    auto res = top.chunk;
    res->syncRunMethod<dbif::SetChunkParseRequest>(top.start, pos_, top.items);
    if (stack_.size() > 1) {
      stack_[stack_.size() - 2].items.push_back(
        data::ChunkDataItem::subchunk(top.start, pos_, top.name, top.chunk)
      );
    }
    stack_.pop_back();
    return res;
  }

  data::BinData getData(
      const QString &name,
      const data::RepackFormat &repack,
      size_t num_elements,
      const data::FieldHighType &high_type) {
    size_t src_sz = data::repackSize(width_, repack, num_elements);
    if (pos_ >= blob_size_)
      return data::BinData();
    auto data = blob_->syncGetInfo<veles::dbif::BlobDataRequest>(pos_, pos_+src_sz);
    pos_ += src_sz;
    data::BinData res = data::repack(data->data, repack, 0, num_elements);
    stack_.back().items.push_back(data::ChunkDataItem::field(
      pos_ - src_sz, pos_, name,
      repack, num_elements, high_type, res
    ));
    return res;
  }

  uint32_t getLe32(
      const QString &name,
      data::FieldHighType::FieldSignMode sign_mode = data::FieldHighType::UNSIGNED) {
    auto data = getData(
      name, data::RepackFormat{data::RepackEndian::LITTLE, 32}, 1,
      data::FieldHighType::fixed(sign_mode));
    if (!data.size())
      return 0;
    return data.element64();
  }

  uint32_t getBe32(
      const QString &name,
      data::FieldHighType::FieldSignMode sign_mode = data::FieldHighType::UNSIGNED) {
    auto data = getData(
      name, data::RepackFormat{data::RepackEndian::BIG, 32}, 1,
      data::FieldHighType::fixed(sign_mode));
    if (!data.size())
      return 0;
    return data.element64();
  }

  std::vector<uint8_t> getBytes(const QString &name, uint64_t len) {
    auto data = getData(
      name, data::RepackFormat{data::RepackEndian::LITTLE, 8}, len,
      data::FieldHighType());
    std::vector<uint8_t> res;
    for (size_t i = 0; i < data.size(); i++) {
      res.push_back(data.element64(i));
    }
    return res;
  }

  uint8_t getByte(const QString &name) {
    auto data = getData(
      name, data::RepackFormat{data::RepackEndian::LITTLE, 8}, 1,
      data::FieldHighType());
    if (!data.size())
      return 0;
    return data.element64();
  }

  std::vector<uint16_t> getLe16(const QString &name, uint64_t num) {
    std::vector<uint16_t> res;
    auto data = getData(
      name, data::RepackFormat{data::RepackEndian::LITTLE, 16}, num,
      data::FieldHighType());
    for (size_t i = 0; i < data.size(); i++) {
      res.push_back(data.element64(i));
    }
    return res;
  }

  bool eof() {
    return pos_ >= blob_size_;
  }

  uint64_t pos() { return pos_; }

};

};
};

#endif
