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

#include "data/bindata.h"
#include "data/repack.h"
#include "data/types.h"

namespace veles {
namespace data {

struct FieldHighType {
  enum FieldHighMode {
    // No type (use this one for bitfield containers).
    NONE,
    // Fixed-point numbers (incl. integers).
    FIXED,
    // Floating-point numbers.
    FLOAT,
    // Strings in various encodings.
    STRING,
    POINTER,
    ENUM,
  } mode;
  // For FIXED and POINTER, positive means the raw value is scaled up by
  // a PoT, negative is for FIXED only and means a fractional value.
  int shift;
  // For FIXED.
  enum FieldSignMode {
    UNSIGNED,
    SIGNED,
  } sign_mode;
  // For FLOAT.
  enum FieldFloatMode {
    IEEE754_SINGLE,
    IEEE754_DOUBLE,
  } float_mode;
  // For FLOAT.  If true, this field should be made of an even number of
  // elements - even ones are real parts, odd ones are imaginary.
  bool float_complex;
  // For STRING.
  enum FieldStringMode {
    // Size is set elsewhere, characters are passed unchanged.
    STRING_RAW,
    // Size is set elsewhere, zeros at the end of the string are trimmed.
    STRING_ZERO_PADDED,
    // Size is determined by the position of the first zero byte, which is
    // trimmed.
    STRING_ZERO_TERMINATED,
  } string_mode;
  enum FieldStringEncoding {
    // Use directly as Unicode codepoint (ie. ISO-8859-1, UCS-2, UCS-4).
    ENC_RAW,
    ENC_UTF8,
    ENC_UTF16,
  } string_encoding;
  // For ENUM: names the enum type; for POINTER: names the pointed-to type.
  QString type_name;

  static FieldHighType fixed(FieldSignMode sign_mode, int shift = 0) {
    FieldHighType res;
    res.mode = FIXED;
    res.sign_mode = sign_mode;
    res.shift = shift;
    return res;
  }

  static FieldHighType floating(FieldFloatMode float_mode,
                                bool float_complex = false) {
    FieldHighType res;
    res.mode = FLOAT;
    res.float_mode = float_mode;
    res.float_complex = float_complex;
    return res;
  }

  static FieldHighType string(FieldStringMode string_mode,
                              FieldStringEncoding string_encoding = ENC_RAW) {
    FieldHighType res;
    res.mode = STRING;
    res.string_mode = string_mode;
    res.string_encoding = string_encoding;
    return res;
  }
};

struct ChunkDataItem {
  enum ChunkDataItemType {
    NONE,
    SUBCHUNK,
    SUBBLOB,
    FIELD,
    // BITFIELDs are associated with the preceding FIELD or COMPUTED.
    BITFIELD,
    COMPUTED,
    PAD,
  } type = NONE;
  // Not for COMPUTED; for BITFIELD these are bit indices.
  uint64_t start;
  uint64_t end;
  // Not for PAD, redundant for SUBCHUNK.
  QString name;
  // Only for FIELD.
  Repacker repack;
  uint64_t num_elements;
  // For COMPUTED, FIELD, BITFIELD.
  FieldHighType high_type;
  BinData raw_value;
  // For COMPUTED, FIELD, BITFIELD, SUBCHUNK.
  std::vector<ObjectHandle> ref;

  explicit operator bool() const { return type != NONE; }

  ChunkDataItem() : type() {}

  static ChunkDataItem subchunk(uint64_t start, uint64_t end,
                                const QString& name, ObjectHandle chunk) {
    ChunkDataItem res;
    res.type = SUBCHUNK;
    res.start = start;
    res.end = end;
    res.name = name;
    res.ref = {chunk};
    return res;
  }

  static ChunkDataItem subblob(const QString& name, ObjectHandle blob) {
    ChunkDataItem res;
    res.type = SUBBLOB;
    res.name = name;
    res.ref = {blob};
    return res;
  }

  static ChunkDataItem field(uint64_t start, uint64_t end, const QString& name,
                             const Repacker& repack, uint64_t num_elements,
                             const FieldHighType& high_type,
                             const BinData& raw_value,
                             const std::vector<ObjectHandle>& ref = {}) {
    ChunkDataItem res;
    res.type = FIELD;
    res.start = start;
    res.end = end;
    res.name = name;
    res.repack = repack;
    res.num_elements = num_elements;
    res.high_type = high_type;
    res.raw_value = raw_value;
    res.ref = ref;
    return res;
  }
};

}  // namespace data
}  // namespace veles
