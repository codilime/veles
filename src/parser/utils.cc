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
#include "dbif/universe.h"
#include "parser/utils.h"

namespace veles {
namespace parser {

dbif::ObjectHandle findSubChunk(dbif::ObjectHandle parent, const QString &name) {
  auto parsed = parent->syncGetInfo<dbif::ChunkDataRequest>();
  for (auto &x : parsed->items) {
    if (x.type == data::ChunkDataItem::SUBCHUNK && x.name == name) {
      return x.ref[0];
    }
  }
  return dbif::ObjectHandle();
}

data::ChunkDataItem findField(dbif::ObjectHandle parent, const QString &name) {
  auto parsed = parent->syncGetInfo<dbif::ChunkDataRequest>();
  for (auto &x : parsed->items) {
    if ((x.type == data::ChunkDataItem::FIELD || x.type == data::ChunkDataItem::COMPUTED ||
        x.type == data::ChunkDataItem::BITFIELD) && x.name == name) {
      return x;
    }
  }
  return data::ChunkDataItem();
}

dbif::ObjectHandle makeSubBlob(dbif::ObjectHandle parent, const QString &name, const data::BinData &data) {
  return parent->syncRunMethod<dbif::ChunkCreateSubBlobRequest>(data, name)->object;
}

}
}
