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
#include "parser/unpyc.h"
#include <typeinfo>
#include "data/field.h"
#include "dbif/error.h"
#include "dbif/info.h"
#include "dbif/method.h"
#include "dbif/promise.h"
#include "dbif/universe.h"
#include "parser/stream.h"
#include "parser/utils.h"

namespace veles {
namespace parser {

namespace {

enum {
  PYVER_FLAG_WORDCODE = 0x1,
  PYVER_FLAG_SIGNED_LINENO = 0x2,
  PYVER_FLAG_PY36 = 0x4,
  PYVER_FLAG_LT_PY36 = 0x8,
};

struct PycVersion {
  const char* name;
  uint32_t sig;
  uint32_t flags;
};

const uint32_t PYVER_FLAGS_PY33 = PYVER_FLAG_LT_PY36;
const uint32_t PYVER_FLAGS_PY34 = PYVER_FLAGS_PY33;
const uint32_t PYVER_FLAGS_PY35 = PYVER_FLAGS_PY34;
const uint32_t PYVER_FLAGS_PY36 = (PYVER_FLAGS_PY35 | PYVER_FLAG_WORDCODE |
                                   PYVER_FLAG_SIGNED_LINENO | PYVER_FLAG_PY36) &
                                  ~(PYVER_FLAG_LT_PY36);

const PycVersion pyc_versions[] = {
    {"cpython33", 0x0a0d0c9e, PYVER_FLAGS_PY33},
    {"cpython34", 0x0a0d0cee, PYVER_FLAGS_PY34},
    {"cpython35", 0x0a0d0d16, PYVER_FLAGS_PY35},
    {"cpython36", 0x0a0d0d33, PYVER_FLAGS_PY36},
};

enum {
  PYC_OP_SIMPLE,
  PYC_OP_NUM,
  PYC_OP_NAME,
  PYC_OP_JREL,
  PYC_OP_JABS,
  PYC_OP_EXT,
  PYC_OP_UEX,
  PYC_OP_CALL,
  PYC_OP_FUN,
  PYC_OP_CONST,
  PYC_OP_CMP,
};

struct PycOpcode {
  int opcode;
  const char* name;
  int type;
  uint32_t flags;
};

const PycOpcode pyc_ops[] = {
    {1, "POP_TOP", PYC_OP_SIMPLE},
    {2, "ROT_TWO", PYC_OP_SIMPLE},
    {3, "ROT_THREE", PYC_OP_SIMPLE},
    {4, "DUP_TOP", PYC_OP_SIMPLE},
    {5, "DUP_TWO", PYC_OP_SIMPLE},
    {9, "NOP", PYC_OP_SIMPLE},
    {10, "UNARY_POSITIVE", PYC_OP_SIMPLE},
    {11, "UNARY_NEGATIVE", PYC_OP_SIMPLE},
    {12, "UNARY_NOT", PYC_OP_SIMPLE},
    {15, "UNARY_INVERT", PYC_OP_SIMPLE},
    {16, "BINARY_MATRIX_MULTIPLY", PYC_OP_SIMPLE},
    {17, "INPLACE_MATRIX_MULTIPLY", PYC_OP_SIMPLE},
    {19, "BINARY_POWER", PYC_OP_SIMPLE},
    {20, "BINARY_MULTIPLY", PYC_OP_SIMPLE},
    {22, "BINARY_MODULO", PYC_OP_SIMPLE},
    {23, "BINARY_ADD", PYC_OP_SIMPLE},
    {24, "BINARY_SUBTRACT", PYC_OP_SIMPLE},
    {25, "BINARY_SUBSCR", PYC_OP_SIMPLE},
    {26, "BINARY_FLOOR_DIVIDE", PYC_OP_SIMPLE},
    {27, "BINARY_TRUE_DIVIDE", PYC_OP_SIMPLE},
    {28, "INPLACE_FLOOR_DIVIDE", PYC_OP_SIMPLE},
    {29, "INPLACE_TRUE_DIVIDE", PYC_OP_SIMPLE},
    {50, "GET_AITER", PYC_OP_SIMPLE},
    {51, "GET_ANEXT", PYC_OP_SIMPLE},
    {52, "BEFORE_ASYNC_WITH", PYC_OP_SIMPLE},
    {54, "STORE_MAP", PYC_OP_SIMPLE},
    {55, "INPLACE_ADD", PYC_OP_SIMPLE},
    {56, "INPLACE_SUBTRACT", PYC_OP_SIMPLE},
    {57, "INPLACE_MULTIPLY", PYC_OP_SIMPLE},
    {59, "INPLACE_MODULO", PYC_OP_SIMPLE},
    {60, "STORE_SUBSCR", PYC_OP_SIMPLE},
    {61, "DELETE_SUBSCR", PYC_OP_SIMPLE},
    {62, "BINARY_LSHIFT", PYC_OP_SIMPLE},
    {63, "BINARY_RSHIFT", PYC_OP_SIMPLE},
    {64, "BINARY_AND", PYC_OP_SIMPLE},
    {65, "BINARY_XOR", PYC_OP_SIMPLE},
    {66, "BINARY_OR", PYC_OP_SIMPLE},
    {67, "INPLACE_POWER", PYC_OP_SIMPLE},
    {68, "GET_ITER", PYC_OP_SIMPLE},
    {69, "STORE_LOCALS", PYC_OP_SIMPLE, PYVER_FLAG_LT_PY36},
    {69, "GET_YIELD_FROM_ITER", PYC_OP_SIMPLE, PYVER_FLAG_PY36},
    {70, "PRINT_EXPR", PYC_OP_SIMPLE},
    {71, "LOAD_BUILD_CLASS", PYC_OP_SIMPLE},
    {72, "YIELD_FROM", PYC_OP_SIMPLE},
    {73, "GET_AWAITABLE", PYC_OP_SIMPLE},
    {75, "INPLACE_LSHIFT", PYC_OP_SIMPLE},
    {76, "INPLACE_RSHIFT", PYC_OP_SIMPLE},
    {77, "INPLACE_AND", PYC_OP_SIMPLE},
    {78, "INPLACE_XOR", PYC_OP_SIMPLE},
    {79, "INPLACE_OR", PYC_OP_SIMPLE},
    {80, "BREAK_LOOP", PYC_OP_SIMPLE},
    {81, "WITH_CLEANUP", PYC_OP_SIMPLE, PYVER_FLAG_LT_PY36},
    {81, "WITH_CLEANUP_START", PYC_OP_SIMPLE, PYVER_FLAG_PY36},
    {82, "WITH_CLEANUP_FINISH", PYC_OP_SIMPLE},
    {83, "RETURN_VALUE", PYC_OP_SIMPLE},
    {84, "IMPORT_STAR", PYC_OP_SIMPLE},
    {85, "SETUP_ANNOTATIONS", PYC_OP_SIMPLE},
    {86, "YIELD_VALUE", PYC_OP_SIMPLE},
    {87, "POP_BLOCK", PYC_OP_SIMPLE},
    {88, "END_FINALLY", PYC_OP_SIMPLE},
    {89, "POP_EXCEPT", PYC_OP_SIMPLE},
    // opcodes have an argument from here on
    {90, "STORE_NAME", PYC_OP_NAME},
    {91, "DELETE_NAME", PYC_OP_NAME},
    {92, "UNPACK_SEQUENCE", PYC_OP_NUM},
    {93, "FOR_ITER", PYC_OP_JREL},
    {94, "UNPACK_EX", PYC_OP_UEX},
    {95, "STORE_ATTR", PYC_OP_NAME},
    {96, "DELETE_ATTR", PYC_OP_NAME},
    {97, "STORE_GLOBAL", PYC_OP_NAME},
    {98, "DELETE_GLOBAL", PYC_OP_NAME},
    {100, "LOAD_CONST", PYC_OP_CONST},
    {101, "LOAD_NAME", PYC_OP_NAME},
    {102, "BUILD_TUPLE", PYC_OP_NUM},
    {103, "BUILD_LIST", PYC_OP_NUM},
    {104, "BUILD_SET", PYC_OP_NUM},
    {105, "BUILD_MAP", PYC_OP_NUM},
    {106, "LOAD_ATTR", PYC_OP_NAME},
    {107, "COMPARE_OP", PYC_OP_CMP},
    {108, "IMPORT_NAME", PYC_OP_NAME},
    {109, "IMPORT_FROM", PYC_OP_NAME},
    {110, "JUMP_FORWARD", PYC_OP_JREL},
    {111, "JUMP_IF_FALSE_OR_POP", PYC_OP_JABS},
    {112, "JUMP_IF_TRUE_OR_POP", PYC_OP_JABS},
    {113, "JUMP_ABSOLUTE", PYC_OP_JABS},
    {114, "POP_JUMP_IF_FALSE", PYC_OP_JABS},
    {115, "POP_JUMP_IF_TRUE", PYC_OP_JABS},
    {116, "LOAD_GLOBAL", PYC_OP_NAME},
    {119, "CONTINUE_LOOP", PYC_OP_JABS},
    {120, "SETUP_LOOP", PYC_OP_JREL},
    {121, "SETUP_EXCEPT", PYC_OP_JREL},
    {122, "SETUP_FINALLY", PYC_OP_JREL},
    {124, "LOAD_FAST", PYC_OP_NUM},
    {125, "STORE_FAST", PYC_OP_NUM},
    {126, "DELETE_FAST", PYC_OP_NUM},
    {127, "STORE_ANNOTATION", PYC_OP_NUM},
    {130, "RAISE_VARARGS", PYC_OP_NUM},
    {131, "CALL_FUNCTION", PYC_OP_CALL},
    {132, "MAKE_FUNCTION", PYC_OP_FUN},
    {133, "BUILD_SLICE", PYC_OP_NUM},
    {134, "MAKE_CLOSURE", PYC_OP_FUN},
    {135, "LOAD_CLOSURE", PYC_OP_NUM},
    {136, "LOAD_DEREF", PYC_OP_NUM},
    {137, "STORE_DEREF", PYC_OP_NUM},
    {138, "DELETE_DEREF", PYC_OP_NUM},
    {140, "CALL_FUNCTION_VAR", PYC_OP_CALL},
    {141, "CALL_FUNCTION_KW", PYC_OP_CALL},
    {142, "CALL_FUNCTION_VAR_KW", PYC_OP_CALL, PYVER_FLAG_LT_PY36},
    {142, "CALL_FUNCTION_EX", PYC_OP_NUM, PYVER_FLAG_PY36},
    {143, "SETUP_WITH", PYC_OP_JREL},
    {144, "EXTENDED_ARG", PYC_OP_EXT},
    {145, "LIST_APPEND", PYC_OP_NUM},
    {146, "SET_ADD", PYC_OP_NUM},
    {147, "MAP_ADD", PYC_OP_NUM},
    {148, "LOAD_CLASS_DEREF", PYC_OP_NUM},
    {149, "BUILD_LIST_UNPACK", PYC_OP_NUM},
    {150, "BUILD_MAP_UNPACK", PYC_OP_NUM},
    {151, "BUILD_MAP_UNPACK_WITH_CALL", PYC_OP_NUM},
    {152, "BUILD_TUPLE_UNPACK", PYC_OP_NUM},
    {153, "BUILD_SET_UNPACK", PYC_OP_NUM},
    {154, "SETUP_ASYNC_WITH", PYC_OP_JREL},
    {155, "FORMAT_VALUE", PYC_OP_NUM},
    {156, "BUILD_CONST_KEY_MAP", PYC_OP_NUM},
    {157, "BUILD_STRING", PYC_OP_NUM},
    {158, "BUILD_TUPLE_UNPACK_WITH_CALL", PYC_OP_NUM},
};

void parseCode(const dbif::ObjectHandle& code, const PycVersion& version);

bool parseMarshal(StreamParser* parser, const QString& name,
                  const PycVersion& version) {
  parser->startChunk("marshal", name);
  uint8_t mtype = parser->getByte("type");
  // XXX: annotate type enum val & ref bit
  // XXX: store ref
  switch (mtype & 0x7f) {
    case '0':  // NULL
    case 'N':  // None
    case 'T':  // True
    case 'F':  // False
    case '.':  // ...
    case 'S':  // StopIteration (unused)
      break;
    case 'i': {
      // 32-bit signed int
      parser->getLe32("value", data::FieldHighType::SIGNED);
      break;
    }
    case 'l': {
      // big int
      // XXX: annotate with bignum value
      int32_t slen = parser->getLe32("len", data::FieldHighType::SIGNED);
      uint32_t len = slen < 0 ? -slen : slen;
      parser->getLe16("digits", len);
      break;
    }
    case 'g': {
      // XXX: get as 64-bit, annotate as double
      parser->getBytes("value", 8);
      break;
    }
    case 'y': {
      // XXX: get as 64-bit, annotate as double
      parser->getBytes("real", 8);
      parser->getBytes("imag", 8);
      break;
    }
    case 's': {
      // bytestring
      uint32_t len = parser->getLe32("length");
      // XXX: annotate as string, unless it's code / lnotab?
      parser->getBytes("bytes", len);
      break;
    }
    case 'u':
    case 't': {
      // UTF-8 unicode string, 32-bit length
      // XXX: annotate as number
      uint32_t len = parser->getLe32("length");
      parser->getBytes("chars", len);
      break;
    }
    case 'a':
    case 'A': {
      // 7-bit-only unicode string, 32-bit length
      // XXX: annotate as number
      uint32_t len = parser->getLe32("length");
      parser->getBytes("chars", len);
      break;
    }
    case 'z':
    case 'Z': {
      // 7-bit-only unicode string, 8-bit length
      // XXX: annotate as number
      uint32_t len = parser->getByte("length");
      parser->getBytes("chars", len);
      break;
    }
    case '(': {
      // tuple, 32-bit length
      // XXX: annotate as number
      uint32_t len = parser->getLe32("length");
      for (uint32_t i = 0; i != len; i++) {
        if (!parseMarshal(parser, QString("elements[%1]").arg(i), version)) {
          goto err;
        }
      }
      break;
    }
    case ')': {
      // tuple, 8-bit length
      // XXX: annotate as number
      uint32_t len = parser->getByte("length");
      for (uint32_t i = 0; i != len; i++) {
        if (!parseMarshal(parser, QString("elements[%1]").arg(i), version)) {
          goto err;
        }
      }
      break;
    }
    // '>': frozenset
    case 'r': {
      // ref
      // XXX: find that thing and annotate
      parser->getLe32("idx");
      break;
    }
    case 'c': {
      // code object
      // XXX: annotate all these as numbers
      parser->getLe32("argcount");
      parser->getLe32("kwonlyargcount");
      parser->getLe32("nlocals");
      parser->getLe32("stacksize");
      parser->getLe32("flags");
      if (!parseMarshal(parser, "code", version)) goto err;
      if (!parseMarshal(parser, "consts", version)) goto err;
      if (!parseMarshal(parser, "names", version)) goto err;
      if (!parseMarshal(parser, "varnames", version)) goto err;
      if (!parseMarshal(parser, "freevars", version)) goto err;
      if (!parseMarshal(parser, "cellvars", version)) goto err;
      if (!parseMarshal(parser, "filename", version)) goto err;
      if (!parseMarshal(parser, "name", version)) goto err;
      parser->getLe32("firstlineno");
      if (!parseMarshal(parser, "lnotab", version)) goto err;
      parseCode(parser->endChunk(), version);
      return true;
    }
    default:
    err:
      parser->endChunk();
      return false;
  }
  parser->endChunk();
  return true;
}

std::vector<std::tuple<uint64_t, uint64_t, uint64_t>> parseLnotab(
    const dbif::ObjectHandle& code, const dbif::ObjectHandle& bytecodeBlob) {
  std::vector<std::tuple<uint64_t, uint64_t, uint64_t>> res;
  auto lnotabObj = findSubChunk(code, "lnotab");
  if (!lnotabObj) {
    return res;
  }
  auto lnotabField = findField(lnotabObj, "bytes");
  if (!lnotabField) {
    return res;
  }
  auto firstlinenoField = findField(code, "firstlineno");
  if (!firstlinenoField || firstlinenoField.raw_value.size() != 1 ||
      firstlinenoField.raw_value.width() > 64) {
    return res;
  }
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
    if (line_inc != 0 && addr != prev_addr) {
      res.push_back(std::make_tuple(prev_addr, addr, line));
      prev_addr = addr;
    }
    line += line_inc;
  }
  auto bytecodeDesc = bytecodeBlob->syncGetInfo<dbif::DescriptionRequest>();
  uint64_t bytecodeSize =
      bytecodeDesc.dynamicCast<dbif::BlobDescriptionReply>()->size;
  res.push_back(std::make_tuple(prev_addr, bytecodeSize, line));
  parser.endChunk();
  return res;
}

void parseCode(const dbif::ObjectHandle& code, const PycVersion& version) {
  // XXX: needs ref support
  auto bytecodeObj = findSubChunk(code, "code");
  if (!bytecodeObj) {
    return;
  }
  auto bytecodeField = findField(bytecodeObj, "bytes");
  if (!bytecodeField) {
    return;
  }
  auto bytecodeBlob = makeSubBlob(code, "code", bytecodeField.raw_value);
  auto lines = parseLnotab(code, bytecodeBlob);
  StreamParser parser(bytecodeBlob, 0);
  while (!parser.eof()) {
    uint64_t arg = 0;
    int arg_width = 0;
    parser.startChunk("pyc_insn", QString("insn_%1").arg(parser.pos()));
  restart:
    uint8_t opcode = parser.getByte("opcode");
    bool found = false;
    for (auto& op : pyc_ops) {
      if (op.opcode == opcode && (op.flags & version.flags) == op.flags) {
        if ((version.flags & PYVER_FLAG_WORDCODE) != 0) {
          arg <<= 8;
          uint8_t cur_arg = parser.getByte("arg");
          arg |= cur_arg;
          arg_width += 8;
        } else {
          if (op.type != PYC_OP_SIMPLE) {
            arg <<= 16;
            auto cur_arg = parser.getLe16("arg", 1);
            if (!cur_arg.empty()) {
              arg |= cur_arg[0];
              arg_width += 16;
            }
          }
        }
        switch (op.type) {
          case PYC_OP_EXT:
            goto restart;
          case PYC_OP_SIMPLE:
            parser.setComment(op.name);
            break;
          case PYC_OP_JREL:
            arg += parser.pos();
          // fallthru
          case PYC_OP_NUM:
          default:
            parser.setComment(
                QString("%1 %2").arg(op.name, QString("%1").arg(arg)));
            break;
        }
        parser.endChunk();
        found = true;
        break;
      }
    }
    if (!found) {
      return;
    }
  }
#if 0
  for (auto &line : lines) {
    // XXX set some sort of a prop with line no
    bytecodeBlob->syncRunMethod<dbif::ChunkCreateRequest>(
      QString("line_%1").arg(std::get<2>(line)), "pyc_line_tag", dbif::ObjectHandle(), std::get<0>(line), std::get<1>(line));
  }
#endif
}

}  // namespace

void unpycFileBlob(const dbif::ObjectHandle& blob, uint64_t start,
                   const dbif::ObjectHandle& parent_chunk) {
  StreamParser parser(blob, start, parent_chunk);
  parser.startChunk("pycheader", "header");
  uint32_t sig = parser.getLe32("sig");
  parser.getLe32("time");
  parser.getLe32("size");
  parser.endChunk();
  for (auto& version : pyc_versions) {
    if (version.sig == sig) {
      parseMarshal(&parser, "module", version);
      break;
    }
  }
}

}  // namespace parser
}  // namespace veles
