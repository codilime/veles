/*
 * Copyright 2017 Marek Rusinowski
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
#include <limits>
#include <string>

#include "parser/unogg.h"

#include "parser/stream.h"

namespace veles {
namespace parser {

enum {
  OGG_HEADER_TYPE_CONTINUED_PACKET = 0x1,
  OGG_HEADER_TYPE_FIRST_PAGE = 0x2,
  OGG_HEADER_TYPE_LAST_PAGE = 0x4
};

static std::string formGroup = "FORM", listGroup = "LIST", catGroup = "CAT ";

static void parseChunkGroup(StreamParser* parser, size_t size) {
  size_t pos = 0;
  while (pos < size && !parser->eof()) {
    parser->startChunk("chunk", "chunk");
    auto bytes = parser->getString("id", 4);
    uint32_t chunk_size = parser->getLe32("size");
    if (bytes == formGroup || bytes == listGroup || bytes == catGroup) {
      parseChunkGroup(parser, chunk_size);
    } else {
      parser->getBytes("data", chunk_size);
    }
    parser->endChunk();
  }
}

void unIffFileBlob(const dbif::ObjectHandle& blob, uint64_t start,
                   const dbif::ObjectHandle& parent_chunk) {
  StreamParser parser(blob, start, parent_chunk);
  parseChunkGroup(&parser, std::numeric_limits<size_t>::max());
}

}  // namespace parser
}  // namespace veles
