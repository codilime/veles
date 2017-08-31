/*
 * Copyright 2017 CodiLime
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

#include <cstring>

#include "kaitai/kaitaistream.h"

namespace veles {
namespace kaitai {

kaitai::kstream::kstream(const veles::dbif::ObjectHandle& blob, uint64_t start,
                         const veles::dbif::ObjectHandle& parent_chunk,
                         uint64_t max_size, bool error)
    : obj_(blob), current_name_(nullptr), error_(error), max_size_(max_size) {
  parser_ = new veles::parser::StreamParser(blob, start, parent_chunk);
}

kaitai::kstream::~kstream() { delete parser_; }

veles::dbif::ObjectHandle kaitai::kstream::startChunk(const char* name) {
  return parser_->startChunk(name, name);
}

veles::dbif::ObjectHandle kaitai::kstream::endChunk() {
  return parser_->endChunk();
}

void kaitai::kstream::close() {}

bool kaitai::kstream::is_eof() {
  if (max_size_ != 0 && pos() >= max_size_) {
    return true;
  }
  return error_ || parser_->eof();
}

uint64_t kaitai::kstream::pos() {
  if (error_) {
    return 0;
  }
  return parser_->pos();
}

uint64_t kaitai::kstream::size() {
  if (error_) {
    return 0;
  }
  return parser_->pos() + parser_->bytesLeft();
}

int8_t kaitai::kstream::read_s1() {
  if (error_) {
    return 0;
  }
  return parser_->getByte(current_name_);
}

int16_t kaitai::kstream::read_s2be() {
  if (error_) {
    return 0;
  }
  return parser_->getBe16(current_name_, 1)[0];
}

int32_t kaitai::kstream::read_s4be() {
  if (error_) {
    return 0;
  }
  return parser_->getBe32(current_name_);
}

int64_t kaitai::kstream::read_s8be() {
  if (error_) {
    return 0;
  }
  return parser_->getBe64(current_name_);
}

int16_t kaitai::kstream::read_s2le() {
  if (error_) {
    return 0;
  }
  return parser_->getLe16(current_name_, 1)[0];
}

int32_t kaitai::kstream::read_s4le() {
  if (error_) {
    return 0;
  }
  return parser_->getLe32(current_name_);
}

int64_t kaitai::kstream::read_s8le() {
  if (error_) {
    return 0;
  }
  return parser_->getLe64(current_name_);
}

uint8_t kaitai::kstream::read_u1() {
  if (error_) {
    return 0;
  }
  return parser_->getByte(current_name_);
}

uint16_t kaitai::kstream::read_u2be() {
  if (error_) {
    return 0;
  }
  return parser_->getBe16(current_name_, 1)[0];
}

uint32_t kaitai::kstream::read_u4be() {
  if (error_) {
    return 0;
  }
  return parser_->getBe32(current_name_);
}

uint64_t kaitai::kstream::read_u8be() {
  if (error_) {
    return 0;
  }
  return parser_->getBe64(current_name_);
}

uint16_t kaitai::kstream::read_u2le() {
  if (error_) {
    return 0;
  }
  return parser_->getLe16(current_name_, 1)[0];
}

uint32_t kaitai::kstream::read_u4le() {
  if (error_) {
    return 0;
  }
  return parser_->getLe32(current_name_);
}

uint64_t kaitai::kstream::read_u8le() {
  if (error_) {
    return 0;
  }
  return parser_->getLe64(current_name_);
}

float kaitai::kstream::read_f4be() {
  if (error_) {
    return 0.0;
  }
  return parser_->getFloat32Be(current_name_);
}

double kaitai::kstream::read_f8be() {
  if (error_) {
    return 0.0;
  }
  return parser_->getFloat64Be(current_name_);
}

float kaitai::kstream::read_f4le() {
  if (error_) {
    return 0.0;
  }
  return parser_->getFloat32Le(current_name_);
}

double kaitai::kstream::read_f8le() {
  if (error_) {
    return 0.0;
  }
  return parser_->getFloat64Le(current_name_);
}

std::string kaitai::kstream::read_str_eos(const char* enc) {
  if (error_) {
    return "";
  }
  return bytes_to_string(parser_->getBytes(current_name_, parser_->bytesLeft()),
                         enc);
}

std::string kaitai::kstream::read_str_byte_limit(size_t len, const char* enc) {
  if (error_) {
    return "";
  }
  return bytes_to_string(parser_->getBytes(current_name_, len), enc);
}

std::string kaitai::kstream::read_strz(const char* enc, char term, bool include,
                                       bool consume, bool /*eos_error*/) {
  if (error_) {
    return "";
  }
  auto data = parser_->getBytesUntil(current_name_, term, include && consume);

  if (consume && !include) {
    parser_->skip(1);
  }

  return bytes_to_string(data, enc);
}

std::vector<uint8_t> kaitai::kstream::read_bytes(size_t len) {
  if (error_) {
    return {};
  }

  // Hack to avoid double chunks when allocating new io from fixed length data
  if (current_name_ != nullptr && strncmp(current_name_, "_skip_me_", 9) == 0) {
    parser_->skip(len);
    return std::vector<uint8_t>(len);
  }
  auto result = parser_->getBytes(current_name_, len);
  return result;
}

std::vector<uint8_t> kaitai::kstream::read_bytes_full() {
  if (error_) {
    return {};
  }

  return read_bytes(parser_->bytesLeft());
}

std::vector<uint8_t> kaitai::kstream::ensure_fixed_contents(
    const std::string& expected) {
  if (error_) {
    return {};
  }
  auto result = read_bytes(expected.length());
  std::string actual = std::string(result.begin(), result.end());
  if (actual != expected) {
    error_ = true;
  }

  return result;
}

std::string kaitai::kstream::bytes_to_string(const std::vector<uint8_t>& bytes,
                                             const char* /*src_enc*/) {
  std::string res;
  for (auto byte : bytes) {
    res += byte;
  }
  return res;
}

void kaitai::kstream::pushName(const char* name) {
  names_stack_.emplace_back(name);
  current_name_ = names_stack_.back().c_str();
}

void kaitai::kstream::popName() {
  if (!names_stack_.empty()) {
    names_stack_.pop_back();
  }
  if (!names_stack_.empty()) {
    current_name_ = names_stack_.back().c_str();
  } else {
    current_name_ = nullptr;
  }
}

}  // namespace kaitai
}  // namespace veles
