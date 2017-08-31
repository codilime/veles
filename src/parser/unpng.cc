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
#include "parser/unpng.h"

#include "parser/stream.h"
#include "parser/utils.h"

#include <cstdio>
#include <vector>

#include <zlib.h>

namespace veles {
namespace parser {

std::vector<uint8_t> do_inflate(const std::vector<uint8_t>& d) {
  std::vector<uint8_t> res;
  z_stream strm;
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = static_cast<uInt>(d.size());
  strm.next_in = const_cast<uint8_t*>(d.data());
  if (inflateInit(&strm) != Z_OK) {
    return res;
  }
  const unsigned BUFSZ = 0x4000;
  uint8_t buf[BUFSZ];
  while (true) {
    strm.avail_out = BUFSZ;
    strm.next_out = buf;
    auto ret = inflate(&strm, Z_NO_FLUSH);
    switch (ret) {
      case Z_STREAM_ERROR:
      case Z_BUF_ERROR:
      case Z_DATA_ERROR:
      case Z_NEED_DICT:
        // :(
        inflateEnd(&strm);
        return std::vector<uint8_t>();
    }
    res.insert(res.end(), buf, buf + BUFSZ - strm.avail_out);
    if (ret == Z_STREAM_END) {
      inflateEnd(&strm);
      return res;
    }
    if (strm.avail_out > 0) {
      // buffer not completely filled, but we have no more input stream - oops.
      inflateEnd(&strm);
      return std::vector<uint8_t>();
    }
  }
}

void unpngFileBlob(const dbif::ObjectHandle& blob, uint64_t start,
                   const dbif::ObjectHandle& parent_chunk) {
  StreamParser parser(blob, start, parent_chunk);
  parser.startChunk("png_file", "file");
  parser.startChunk("png_header", "header");
  parser.getBytes("sig", 8);
  parser.endChunk();
  std::vector<uint8_t> jointIdats;
  for (unsigned idx = 0; !parser.eof(); idx++) {
    parser.startChunk("png_chunk", QString("chunks[%1]").arg(idx));
    uint32_t len = parser.getBe32("length");
    auto type = parser.getBytes("type", 4);
    auto d = parser.getBytes("data", len);
    parser.getBe32("crc32");
    parser.endChunk();
    if (type[0] == 'I' && type[1] == 'D' && type[2] == 'A' && type[3] == 'T') {
      jointIdats.insert(jointIdats.end(), d.begin(), d.end());
    }
    if (type[0] == 'I' && type[1] == 'E' && type[2] == 'N' && type[3] == 'D') {
      break;
    }
  }
  auto png = parser.endChunk();
  makeSubBlob(png, "deflated_data",
              data::BinData(8, jointIdats.size(), jointIdats.data()));
  auto decompressed = do_inflate(jointIdats);
  if (!decompressed.empty()) {
    makeSubBlob(png, "inflated_data",
                data::BinData(8, decompressed.size(), decompressed.data()));
  }
}

}  // namespace parser
}  // namespace veles
