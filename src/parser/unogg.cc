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
#include "parser/unogg.h"

#include "parser/stream.h"

namespace veles {
namespace parser {

enum {
  OGG_HEADER_TYPE_CONTINUED_PACKET = 0x1,
  OGG_HEADER_TYPE_FIRST_PAGE = 0x2,
  OGG_HEADER_TYPE_LAST_PAGE = 0x4
};

void unOggFileBlob(const dbif::ObjectHandle& blob, uint64_t start,
                   const dbif::ObjectHandle& parent_chunk) {
  StreamParser parser(blob, start, parent_chunk);
  std::vector<unsigned> packets_size;
  for (unsigned ogg_page_num = 0; !parser.eof(); ++ogg_page_num) {
    parser.startChunk("ogg_page", QString("page[%1]").arg(ogg_page_num));
    parser.getBytes("sig", 4);
    parser.getByte("version");
    uint8_t header_type_flags = parser.getByte("header_type_flags");
    QStringList page_comment;
    if ((header_type_flags & OGG_HEADER_TYPE_CONTINUED_PACKET) != 0) {
      page_comment << "continued packet";
    }
    if ((header_type_flags & OGG_HEADER_TYPE_FIRST_PAGE) != 0) {
      page_comment << "first page";
    }
    if ((header_type_flags & OGG_HEADER_TYPE_LAST_PAGE) != 0) {
      page_comment << "last page";
    }
    parser.setComment(page_comment.join(", "));
    parser.getLe64("granule_position");
    parser.getLe32("stream_id");
    parser.getLe32("page_num");
    parser.getLe32("crc32");
    uint8_t page_segments = parser.getByte("page_segments");

    parser.startChunk("ogg_segment_table", "segment_table");
    packets_size.clear();
    for (unsigned segment = 0; segment < page_segments;) {
      unsigned packet_size = 0;
      parser.getBytesUntilCond(
          QString("packet_size[%1]").arg(packets_size.size()),
          [&, page_segments](data::BinData c) {
            uint8_t v = c.element64();
            packet_size += v;
            ++segment;
            return v < 255 || segment >= page_segments;
          });
      packets_size.push_back(packet_size);
    }
    parser.endChunk();

    parser.startChunk("ogg_packet_data", "packet_data");
    for (size_t i = 0; i < packets_size.size(); ++i) {
      parser.getBytes(QString("packet[%1]").arg(i), packets_size[i]);
    }
    parser.endChunk();

    parser.endChunk();
  }
}

}  // namespace parser
}  // namespace veles
