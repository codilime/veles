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

#include "util/edit.h"

namespace veles {
namespace util {

void EditEngine::changeBytes(size_t pos, const data::BinData& bytes,
                             const data::BinData& old_bytes,
                             bool add_to_history) {
  assert(bytes.width() == bindata_width_ &&
         old_bytes.width() == bindata_width_);
  if (add_to_history) {
    edit_stack_data_.push_back(old_bytes);
    edit_stack_.push_back(QPair<size_t, size_t>(pos, bytes.size()));
    while (edit_stack_.size() > edit_stack_limit_) {
      edit_stack_data_.pop_front();
      edit_stack_.pop_front();
    }
  }

  QVector<size_t> overlapping;
  auto it = changes_.lowerBound(pos);
  if (!changes_.empty() && it != changes_.begin()) {
    --it;
  }
  while (it != changes_.constEnd() && it.key() <= pos + bytes.size()) {
    if (it.key() + it.value().size() < pos) {
      ++it;
      continue;
    }

    overlapping.append(it.key());
    ++it;
  }

  size_t new_pos = pos;
  if (!overlapping.isEmpty()) {
    size_t first_pos = overlapping.first();
    size_t last_pos = overlapping.last();
    auto first_chunk = changes_[first_pos];
    auto last_chunk = changes_[last_pos];
    size_t last_size = last_chunk.size();
    size_t size = bytes.size();
    size += pos > first_pos ? pos - first_pos : 0;
    size += last_pos + last_size > pos + bytes.size()
                ? last_pos + last_size - (pos + bytes.size())
                : 0;
    data::BinData new_bytes(bindata_width_, size);
    size_t count = 0;

    if (new_pos > first_pos) {
      new_pos = first_pos;
    }

    for (size_t i = first_pos; i < pos; ++i, ++count) {
      new_bytes.setElement64(
          count, first_chunk.element64(static_cast<int>(i - first_pos)));
    }

    for (size_t i = 0; i < bytes.size(); ++i, ++count) {
      new_bytes.setElement64(count, bytes.element64(i));
    }

    for (size_t i = pos + bytes.size(); i < last_pos + last_size;
         ++i, ++count) {
      new_bytes.setElement64(
          count, last_chunk.element64(static_cast<int>(i - last_pos)));
    }

    for (auto pos : overlapping) {
      changes_.remove(pos);
    }

    changes_[new_pos] = new_bytes;
  } else {
    changes_[new_pos] = bytes;
  }
}

size_t EditEngine::undo() {
  if (!hasUndo()) {
    return 0;
  }

  auto range = edit_stack_.back();
  auto last_change = edit_stack_data_.back();

  changeBytes(range.first, last_change, {}, false);

  edit_stack_data_.pop_back();
  edit_stack_.pop_back();

  return range.first;
}

void EditEngine::applyChanges(data::BinData* bindata, size_t offset,
                              int64_t max_bytes) const {
  assert(bindata->width() == bindata_width_);
  if (max_bytes == -1) {
    max_bytes = bindata->size();
  }

  auto changes = changesFromRange(offset, max_bytes);
  for (auto it = changes.begin(); it != changes.constEnd(); ++it) {
    size_t pos = it.key();
    auto data = it.value();
    for (size_t i = 0; i < data.size(); ++i) {
      bindata->setElement64(pos - offset + i, data.element64(i));
    }
  }
}

QPair<size_t, data::BinData> EditEngine::popFirstChange() {
  data::BinData bindata(bindata_width_, 0);
  size_t pos = 0;

  if (!changes_.isEmpty()) {
    pos = changes_.firstKey();
    auto first_chunk = changes_[pos];
    bindata = data::BinData(bindata_width_, first_chunk.size());
    for (size_t i = 0; i < first_chunk.size(); i++) {
      bindata.setElement64(i, first_chunk.element64(i));
    }
    changes_.remove(pos);
  }

  return QPair<size_t, data::BinData>(pos, bindata);
}

QMap<size_t, data::BinData>::const_iterator EditEngine::itFromPos(
    size_t byte_pos) const {
  if (changes_.isEmpty()) {
    return changes_.constEnd();
  }
  auto it = changes_.upperBound(byte_pos);

  if (it != changes_.begin()) {
    --it;
  }

  size_t pos = it.key();
  auto data = it.value();

  if (byte_pos < pos || (byte_pos >= pos + data.size())) {
    return changes_.constEnd();
  }

  return it;
}

uint64_t EditEngine::byteValue(size_t byte_pos) const {
  const auto& it = itFromPos(byte_pos);

  if (it == changes_.constEnd()) {
    return original_data_->binData()[byte_pos].element64();
  }

  size_t pos = it.key();
  const auto& data = it.value();

  if (byte_pos < pos || pos + data.size() <= byte_pos) {
    return 0;
  }

  return data.element64(static_cast<int>(byte_pos - pos));
}

uint64_t EditEngine::originalByteValue(size_t byte_pos) const {
  return original_data_->binData()[byte_pos].element64();
}

bool EditEngine::hasChanges() const { return !changes_.isEmpty(); }

QMap<size_t, data::BinData> EditEngine::changesFromRange(size_t byte_pos,
                                                         size_t size) const {
  QMap<size_t, data::BinData> res;

  auto it = itFromPos(byte_pos);
  if (it == changes_.constEnd()) {
    it = changes_.upperBound(byte_pos);
  }

  while (it != changes_.constEnd() && it.key() < byte_pos + size) {
    size_t pos = it.key();
    auto data = it.value();

    if (pos < byte_pos) {
      auto len = data.size() - static_cast<int>(byte_pos - pos);
      len = len > 0 ? len : 0;
      data::BinData new_data(bindata_width_, len);

      int counter = 0;
      for (size_t i = byte_pos - pos; i < data.size(); ++i, ++counter) {
        new_data.setElement64(counter, data.element64(i));
      }

      data = new_data;
      pos = byte_pos;
    }

    if (pos + data.size() > byte_pos + size) {
      data::BinData new_data(bindata_width_, byte_pos + size - pos > 0
                                                 ? byte_pos + size - pos
                                                 : 0);
      size_t last_chunk_bytes_count = byte_pos + size - pos;
      for (size_t i = 0; i < last_chunk_bytes_count; ++i) {
        new_data.setElement64(i, data.element64(static_cast<int>(i)));
      }

      data = new_data;
    }

    res[pos] = data;
    ++it;
  }

  return res;
}

void EditEngine::clear() {
  changes_.clear();
  edit_stack_data_.clear();
  edit_stack_.clear();
}

}  // namespace util
}  // namespace veles
