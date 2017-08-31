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
#pragma once

#include <cassert>

#include "data/repack.h"
#include "dbif/info.h"
#include "dbif/types.h"
#include "dbif/universe.h"

namespace veles {
namespace parser {

class StreamParser {
  dbif::ObjectHandle blob_;
  dbif::ObjectHandle parent_chunk_;
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
  StreamParser(dbif::ObjectHandle blob, uint64_t start,
               dbif::ObjectHandle parent_chunk = dbif::ObjectHandle())
      : blob_(blob), parent_chunk_(parent_chunk), pos_(start) {
    auto desc = blob_->syncGetInfo<dbif::DescriptionRequest>();
    width_ = desc.dynamicCast<dbif::BlobDescriptionReply>()->width;
    blob_size_ = desc.dynamicCast<dbif::BlobDescriptionReply>()->size;
  }

  dbif::ObjectHandle startChunk(const QString& type, const QString& name) {
    dbif::ObjectHandle parent = parent_chunk_;
    if (!stack_.empty()) {
      parent = stack_.back().chunk;
    }
    dbif::ObjectHandle chunk = blob_
                                   ->syncRunMethod<dbif::ChunkCreateRequest>(
                                       name, type, parent, pos_, pos_)
                                   ->object;
    stack_.push_back(
        WorkChunk{chunk, pos_, type, name, std::vector<data::ChunkDataItem>()});
    return chunk;
  }

  dbif::ObjectHandle endChunk() {
    auto& top = stack_.back();
    auto res = top.chunk;
    res->syncRunMethod<dbif::SetChunkParseRequest>(top.start, pos_, top.items);
    if (stack_.size() > 1) {
      stack_[stack_.size() - 2].items.push_back(
          data::ChunkDataItem::subchunk(top.start, pos_, top.name, top.chunk));
    }
    stack_.pop_back();
    return res;
  }

  data::BinData getData(const QString& name, const data::Repacker& repack,
                        size_t num_elements,
                        const data::FieldHighType& high_type) {
    size_t src_sz = repack.repackSize(num_elements);
    if (pos_ >= blob_size_) {
      return data::BinData();
    }
    auto data =
        blob_->syncGetInfo<veles::dbif::BlobDataRequest>(pos_, pos_ + src_sz);
    pos_ += src_sz;
    data::BinData res = repack.repack(data->data, 0, num_elements);
    stack_.back().items.push_back(data::ChunkDataItem::field(
        pos_ - src_sz, pos_, name, repack, num_elements, high_type, res));
    return res;
  }

  data::BinData getDataUntil(const QString& name, const data::Repacker& repack,
                             data::BinData termination,
                             const data::FieldHighType& high_type,
                             bool include_termination = true) {
    assert(termination.size() == 1);
    data::BinData res(repack.to_width, 0);
    size_t num_elements = 1;
    size_t src_size = repack.repackSize(num_elements);
    size_t bytes_read = 0;
    bool found = false;
    while (!found && pos_ + bytes_read < blob_size_) {
      if (pos_ + src_size > blob_size_) {
        src_size = blob_size_ - pos_;
      }
      auto data = blob_
                      ->syncGetInfo<veles::dbif::BlobDataRequest>(
                          pos_ + bytes_read, pos_ + bytes_read + src_size)
                      ->data;

      data = repack.repack(data, 0, num_elements);

      for (size_t dataIndex = 0; dataIndex < data.size(); ++dataIndex) {
        if (data[dataIndex] == termination) {
          if (include_termination) {
            dataIndex += 1;
          }
          data = data.data(0, dataIndex);
          found = true;
          break;
        }
      }

      res = res + data;
      num_elements *= 2;
      bytes_read += repack.repackSize(data.size());
      src_size = repack.repackSize(num_elements);
    }

    pos_ += bytes_read;
    stack_.back().items.push_back(data::ChunkDataItem::field(
        pos_ - bytes_read, pos_, name, repack, res.size(), high_type, res));
    return res;
  }

  float getFloat32(const QString& name, data::Endian endian) {
    auto data = getData(
        name, data::Repacker{endian, 8, 32}, 1,
        data::FieldHighType::floating(data::FieldHighType::IEEE754_SINGLE));
    if (data.size() == 0) {
      return 0.0;
    }

    static_assert(sizeof(uint32_t) == sizeof(float), "Unsupported float size");
    uint32_t res = data.element64();
    float ret;
    memcpy(&ret, &res, sizeof(float));
    return ret;
  }

  float getFloat32Le(const QString& name) {
    return getFloat32(name, data::Endian::LITTLE);
  }

  float getFloat32Be(const QString& name) {
    return getFloat32(name, data::Endian::BIG);
  }

  double getFloat64(const QString& name, data::Endian endian) {
    auto data = getData(
        name, data::Repacker{endian, 8, 64}, 1,
        data::FieldHighType::floating(data::FieldHighType::IEEE754_DOUBLE));

    if (data.size() == 0) {
      return 0.0;
    }
    static_assert(sizeof(uint64_t) == sizeof(double),
                  "Unsupported double size");
    uint64_t res = data.element64();
    double ret;
    memcpy(&ret, &res, sizeof(double));
    return ret;
  }

  double getFloat64Le(const QString& name) {
    return getFloat64(name, data::Endian::LITTLE);
  }

  double getFloat64Be(const QString& name) {
    return getFloat64(name, data::Endian::BIG);
  }

  uint32_t get32(const QString& name,
                 data::FieldHighType::FieldSignMode sign_mode,
                 data::Endian endian) {
    auto data = getData(name, data::Repacker{endian, 8, 32}, 1,
                        data::FieldHighType::fixed(sign_mode));
    if (data.size() == 0) {
      return 0;
    }
    return data.element64();
  }

  uint32_t getLe32(const QString& name,
                   data::FieldHighType::FieldSignMode sign_mode =
                       data::FieldHighType::UNSIGNED) {
    return get32(name, sign_mode, data::Endian::LITTLE);
  }

  uint32_t getBe32(const QString& name,
                   data::FieldHighType::FieldSignMode sign_mode =
                       data::FieldHighType::UNSIGNED) {
    return get32(name, sign_mode, data::Endian::BIG);
  }

  uint64_t get64(const QString& name,
                 data::FieldHighType::FieldSignMode sign_mode,
                 data::Endian endian) {
    auto data = getData(name, data::Repacker{endian, 8, 64}, 1,
                        data::FieldHighType::fixed(sign_mode));
    if (data.size() == 0) {
      return 0;
    }
    return data.element64();
  }

  uint64_t getLe64(const QString& name,
                   data::FieldHighType::FieldSignMode sign_mode =
                       data::FieldHighType::UNSIGNED) {
    return get64(name, sign_mode, data::Endian::LITTLE);
  }

  uint64_t getBe64(const QString& name,
                   data::FieldHighType::FieldSignMode sign_mode =
                       data::FieldHighType::UNSIGNED) {
    return get64(name, sign_mode, data::Endian::BIG);
  }

  std::vector<uint8_t> getBytes(const QString& name, uint64_t len) {
    auto data = getData(name, data::Repacker(), len, data::FieldHighType());
    std::vector<uint8_t> res;
    for (size_t i = 0; i < data.size(); i++) {
      res.push_back(static_cast<uint8_t>(data.element64(i)));
    }
    return res;
  }

  std::vector<uint8_t> getBytesUntil(const QString& name, uint8_t termination,
                                     bool include_termination = true) {
    auto data = getDataUntil(name, data::Repacker(),
                             data::BinData::fromRawData(8, {termination}),
                             data::FieldHighType(), include_termination);
    std::vector<uint8_t> res;
    for (size_t i = 0; i < data.size(); i++) {
      res.push_back(static_cast<uint8_t>(data.element64(i)));
    }
    return res;
  }

  uint8_t getByte(const QString& name) {
    auto data = getData(name, data::Repacker(), 1, data::FieldHighType());
    if (data.size() == 0) {
      return 0;
    }
    return data.element64();
  }

  std::vector<uint16_t> get16(const QString& name, uint64_t num,
                              data::Endian endian) {
    std::vector<uint16_t> res;
    auto data = getData(name, data::Repacker{endian, 8, 16}, num,
                        data::FieldHighType());
    for (size_t i = 0; i < data.size(); i++) {
      res.push_back(static_cast<uint16_t>(data.element64(i)));
    }
    return res;
  }

  std::vector<uint16_t> getLe16(const QString& name, uint64_t num) {
    return get16(name, num, data::Endian::LITTLE);
  }

  std::vector<uint16_t> getBe16(const QString& name, uint64_t num) {
    return get16(name, num, data::Endian::BIG);
  }

  bool eof() { return pos_ >= blob_size_; }

  uint64_t pos() { return pos_; }

  uint64_t bytesLeft() {
    if (eof()) {
      return 0;
    }
    return blob_size_ - pos_;
  }

  void skip(uint64_t bytes) { pos_ += bytes; }

  void setComment(const QString& comment) {
    auto& top = stack_.back();
    top.chunk->syncRunMethod<dbif::SetCommentRequest>(comment);
  }
};

}  // namespace parser
}  // namespace veles
