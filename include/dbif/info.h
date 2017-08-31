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

struct InfoRequest {
  virtual ~InfoRequest() = default;

 protected:
  InfoRequest() = default;
};

struct DescriptionReply;
struct ChildrenReply;
struct ParsersListReply;
struct BlobDataReply;
struct ChunkDataReply;

struct DescriptionRequest : InfoRequest {
  typedef DescriptionReply ReplyType;
};

struct ChildrenRequest : InfoRequest {
  typedef ChildrenReply ReplyType;
};

struct ParsersListRequest : InfoRequest {
  typedef ParsersListReply ReplyType;
};

struct BlobDataRequest : InfoRequest {
  const uint64_t start;
  const uint64_t end;
  explicit BlobDataRequest(uint64_t start, uint64_t end)
      : start(start), end(end) {}
  typedef BlobDataReply ReplyType;
};

struct ChunkDataRequest : InfoRequest {
  typedef ChunkDataReply ReplyType;
};

// Replies

struct InfoReply {
  virtual ~InfoReply() = default;

 protected:
  InfoReply() = default;
};

struct DescriptionReply : InfoReply {
  const QString name;
  const QString comment;
  explicit DescriptionReply(const QString& name, const QString& comment)
      : name(name), comment(comment) {}
};

struct BlobDescriptionReply : DescriptionReply {
  const uint64_t base;
  const uint64_t size;
  const int width;
  explicit BlobDescriptionReply(const QString& name, const QString& comment,
                                uint64_t base, uint64_t size, int width)
      : DescriptionReply(name, comment), base(base), size(size), width(width) {}
};

struct FileBlobDescriptionReply : BlobDescriptionReply {
  const QString path;
  explicit FileBlobDescriptionReply(const QString& name, const QString& comment,
                                    uint64_t base, uint64_t size, int width,
                                    const QString& path)
      : BlobDescriptionReply(name, comment, base, size, width), path(path) {}
};

struct SubBlobDescriptionReply : BlobDescriptionReply {
  ObjectHandle parent;
  explicit SubBlobDescriptionReply(const QString& name, const QString& comment,
                                   uint64_t base, uint64_t size, int width,
                                   const ObjectHandle& parent)
      : BlobDescriptionReply(name, comment, base, size, width),
        parent(parent) {}
};

struct ChunkDescriptionReply : DescriptionReply {
  const ObjectHandle blob;
  const ObjectHandle parent_chunk;
  const uint64_t start;
  const uint64_t end;
  const QString chunk_type;
  explicit ChunkDescriptionReply(const QString& name, const QString& comment,
                                 const ObjectHandle blob,
                                 const ObjectHandle parent_chunk,
                                 uint64_t start, uint64_t end,
                                 const QString& chunk_type)
      : DescriptionReply(name, comment),
        blob(blob),
        parent_chunk(parent_chunk),
        start(start),
        end(end),
        chunk_type(chunk_type) {}
  uint64_t size() const { return end - start; }
};

struct ChildrenReply : InfoReply {
  const std::vector<ObjectHandle> objects;
  explicit ChildrenReply(const std::vector<ObjectHandle>& objects)
      : objects(objects) {}
};

struct ParsersListReply : InfoReply {
  const QStringList parserIds;
  explicit ParsersListReply(const QStringList& ids) : parserIds(ids) {}
};

struct BlobDataReply : InfoReply {
  data::BinData data;
  explicit BlobDataReply(const data::BinData& data) : data(data) {}
  explicit BlobDataReply(data::BinData&& data) : data(data) {}
};

struct ChunkDataReply : InfoReply {
  std::vector<data::ChunkDataItem> items;
  explicit ChunkDataReply(const std::vector<data::ChunkDataItem>& items)
      : items(items) {}
};

}  // namespace dbif
}  // namespace veles
