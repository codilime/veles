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
#include <typeinfo>
#include "db/db.h"
#include "dbif/info.h"
#include "dbif/method.h"
#include "dbif/promise.h"
#include "dbif/error.h"
#include "dbif/universe.h"
#include "parser/stream.h"
#include "parser/utils.h"
#include "data/field.h"
#include "parser/unpyc.h"

namespace veles {
namespace parser {

void parseCode(dbif::ObjectHandle code);

bool parseMarshal(StreamParser &parser, const QString &name) {
  parser.startChunk("marshal", name);
  uint8_t mtype = parser.getByte("type");
  // XXX: annotate type enum val & ref bit
  // XXX: store ref
  switch (mtype & 0x7f) {
  case '0': // NULL
  case 'N': // None
  case 'T': // True
  case 'F': // False
  case '.': // ...
  case 'S': // StopIteration (unused)
    break;
  case 'i': {
    // 32-bit signed int
    parser.getLe32("value", data::FieldHighType::SIGNED);
    break;
  }
  case 'l': {
    // big int
    // XXX: annotate with bignum value
    int32_t slen = parser.getLe32("len", data::FieldHighType::SIGNED);
    uint32_t len = slen < 0 ? -slen : slen;
    parser.getLe16("digits", len);
    break;
  }
  case 'g': {
    // XXX: get as 64-bit, annotate as double
    parser.getBytes("value", 8);
    break;
  }
  case 'y': {
    // XXX: get as 64-bit, annotate as double
    parser.getBytes("real", 8);
    parser.getBytes("imag", 8);
    break;
  }
  case 's': {
    // bytestring
    uint32_t len = parser.getLe32("length");
    // XXX: annotate as string, unless it's code / lnotab?
    parser.getBytes("bytes", len);
    break;
  }
  case 'u':
  case 't': {
    // UTF-8 unicode string, 32-bit length
    // XXX: annotate as number
    uint32_t len = parser.getLe32("length");
    parser.getBytes("chars", len);
    break;
  }
  case 'a':
  case 'A': {
    // 7-bit-only unicode string, 32-bit length
    // XXX: annotate as number
    uint32_t len = parser.getLe32("length");
    parser.getBytes("chars", len);
    break;
  }
  case 'z':
  case 'Z': {
    // 7-bit-only unicode string, 8-bit length
    // XXX: annotate as number
    uint32_t len = parser.getByte("length");
    parser.getBytes("chars", len);
    break;
  }
  case '(': {
    // tuple, 32-bit length
    // XXX: annotate as number
    uint32_t len = parser.getLe32("length");
    for (uint32_t i = 0; i != len; i++) {
      if (!parseMarshal(parser, QString("elements[%1]").arg(i))) {
        goto err;
      }
    }
    break;
  }
  case ')': {
    // tuple, 8-bit length
    // XXX: annotate as number
    uint32_t len = parser.getByte("length");
    for (uint32_t i = 0; i != len; i++) {
      if (!parseMarshal(parser, QString("elements[%1]").arg(i))) {
        goto err;
      }
    }
    break;
  }
  // '>': frozenset
  case 'r': {
    // ref
    // XXX: find that thing and annotate
    parser.getLe32("idx");
    break;
  }
  case 'c': {
    // code object
    // XXX: annotate all these as numbers
    parser.getLe32("argcount");
    parser.getLe32("kwonlyargcount");
    parser.getLe32("nlocals");
    parser.getLe32("stacksize");
    parser.getLe32("flags");
    if (!parseMarshal(parser, "code")) goto err;
    if (!parseMarshal(parser, "consts")) goto err;
    if (!parseMarshal(parser, "names")) goto err;
    if (!parseMarshal(parser, "varnames")) goto err;
    if (!parseMarshal(parser, "freevars")) goto err;
    if (!parseMarshal(parser, "cellvars")) goto err;
    if (!parseMarshal(parser, "filename")) goto err;
    if (!parseMarshal(parser, "name")) goto err;
    parser.getLe32("firstlineno");
    if (!parseMarshal(parser, "lnotab")) goto err;
    parseCode(parser.endChunk());
    return true;
  }
  default:
err:
    parser.endChunk();
    return false;
  }
  parser.endChunk();
  return true;
}

void parseLnotab(dbif::ObjectHandle code, dbif::ObjectHandle bytecodeBlob) {
  auto lnotabObj = findSubChunk(code, "lnotab");
  if (!lnotabObj)
    return;
  auto lnotabField = findField(lnotabObj, "bytes");
  if (!lnotabField)
    return;
  auto firstlinenoField = findField(code, "firstlineno");
  if (!firstlinenoField || firstlinenoField.raw_value.size() != 1 || firstlinenoField.raw_value.width() > 64)
    return;
  uint32_t firstlineno = firstlinenoField.raw_value.element64();
  auto lnotabBlob = makeSubBlob(code, "lnotab", lnotabField.raw_value);
  StreamParser parser(lnotabBlob, 0);
  uint64_t line = firstlineno;
  uint64_t addr = 0, prev_addr = 0;
  parser.startChunk("pyc_lnotab", "lnotab");
  for (uint64_t idx = 0; !parser.eof(); idx++) {
    uint8_t addr_inc = parser.getByte(QString("item[%1].addr_inc").arg(idx));
    uint8_t line_inc = parser.getByte(QString("item[%1].line_inc").arg(idx));
    addr += addr_inc;
    if (line_inc && addr != prev_addr) {
      // XXX set some sort of a prop with line no
      bytecodeBlob->syncRunMethod<dbif::ChunkCreateRequest>(
        QString("line_%1").arg(line), "pyc_line_tag", dbif::ObjectHandle(), prev_addr, addr);
      prev_addr = addr;
    }
    line += line_inc;
  }
  auto bytecodeDesc = bytecodeBlob->syncGetInfo<dbif::DescriptionRequest>();
  uint64_t bytecodeSize = bytecodeDesc.dynamicCast<dbif::BlobDescriptionReply>()->size;
  bytecodeBlob->syncRunMethod<dbif::ChunkCreateRequest>(
    QString("line_%1").arg(line), "pyc_line_tag", dbif::ObjectHandle(), prev_addr,
    bytecodeSize);
  parser.endChunk();
}

void parseCode(dbif::ObjectHandle code) {
  // XXX: needs ref support
  auto bytecodeObj = findSubChunk(code, "code");
  if (!bytecodeObj)
    return;
  auto bytecodeField = findField(bytecodeObj, "bytes");
  if (!bytecodeField)
    return;
  auto bytecodeBlob = makeSubBlob(code, "code", bytecodeField.raw_value);
  parseLnotab(code, bytecodeBlob);
}

void unpycFileBlob(dbif::ObjectHandle blob) {
  StreamParser parser(blob, 0);
  parser.startChunk("pycheader", "header");
  parser.getLe32("sig");
  parser.getLe32("time");
  parser.getLe32("size");
  parser.endChunk();
  parseMarshal(parser, "module");
}

}
}
