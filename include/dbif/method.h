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

#include <cstdint>
#include <vector>

#include <QString>

#include "data/bindata.h"
#include "data/field.h"
#include "dbif/types.h"

namespace veles {
namespace dbif {

// Requests

struct MethodRequest {
  virtual void key();
  virtual ~MethodRequest() = default;

 protected:
  MethodRequest() = default;
};

struct CreatedReply;
struct NullReply;

struct RootCreateFileBlobFromDataRequest : MethodRequest {
  data::BinData data;
  QString path;
  explicit RootCreateFileBlobFromDataRequest(const data::BinData& data,
                                             const QString& path)
      : data(data), path(path) {}
  explicit RootCreateFileBlobFromDataRequest(data::BinData&& data,
                                             const QString& path)
      : data(data), path(path) {}
  using ReplyType = CreatedReply;
};

struct ChunkCreateRequest : MethodRequest {
  QString name;
  QString chunk_type;
  ObjectHandle parent_chunk;
  uint64_t start;
  uint64_t end;
  explicit ChunkCreateRequest(const QString& name, const QString& chunk_type,
                              ObjectHandle parent_chunk, uint64_t start,
                              uint64_t end)
      : name(name),
        chunk_type(chunk_type),
        parent_chunk(parent_chunk),
        start(start),
        end(end) {}
  using ReplyType = CreatedReply;
};

struct ChunkCreateSubBlobRequest : MethodRequest {
  data::BinData data;
  QString name;
  explicit ChunkCreateSubBlobRequest(const data::BinData& data,
                                     const QString& name)
      : data(data), name(name) {}
  explicit ChunkCreateSubBlobRequest(data::BinData&& data, const QString& name)
      : data(data), name(name) {}
  using ReplyType = CreatedReply;
};

struct DeleteRequest : MethodRequest {
  using ReplyType = NullReply;
};

struct SetNameRequest : MethodRequest {
  QString name;
  explicit SetNameRequest(const QString& name) : name(name) {}
  using ReplyType = NullReply;
};

struct SetCommentRequest : MethodRequest {
  QString comment;
  explicit SetCommentRequest(const QString& comment) : comment(comment) {}
  using ReplyType = NullReply;
};

struct ChangeDataRequest : MethodRequest {
  uint64_t start;
  uint64_t end;
  data::BinData data;
  ChangeDataRequest(uint64_t start, uint64_t end, const data::BinData& data)
      : start(start), end(end), data(data) {}
  ChangeDataRequest(uint64_t start, uint64_t end, data::BinData&& data)
      : start(start), end(end), data(data) {}
  using ReplyType = NullReply;
};

struct SetChunkBoundsRequest : MethodRequest {
  const uint64_t start;
  const uint64_t end;
  explicit SetChunkBoundsRequest(uint64_t start, uint64_t end)
      : start(start), end(end) {}
  using ReplyType = NullReply;
};

struct SetChunkParseRequest : MethodRequest {
  uint64_t start;
  uint64_t end;
  std::vector<data::ChunkDataItem> items;
  SetChunkParseRequest(uint64_t start, uint64_t end,
                       std::vector<data::ChunkDataItem> items)
      : start(start), end(end), items(items) {}
  using ReplyType = NullReply;
};

struct BlobParseRequest : MethodRequest {
  QString parser_id;
  uint64_t start;
  ObjectHandle parent_chunk;
  explicit BlobParseRequest(QString parser_id = "", uint64_t start = 0,
                            ObjectHandle parent_chunk = ObjectHandle())
      : parser_id(parser_id), start(start), parent_chunk(parent_chunk) {}
  using ReplyType = NullReply;
};

// Replies

struct MethodReply {
  virtual ~MethodReply() = default;

 protected:
  MethodReply() = default;
};

struct NullReply : MethodReply {};

struct CreatedReply : MethodReply {
  const ObjectHandle object;
  explicit CreatedReply(ObjectHandle object) : object(object) {}
};

}  // namespace dbif
}  // namespace veles
