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

void EditEngine::changeBytes(size_t pos, const QVector<uint64_t>& bytes, const QVector<uint64_t>& old_bytes, bool add_to_history) {
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
  QVector<uint64_t> new_bytes;
  if (!overlapping.isEmpty()) {
    size_t first_pos = overlapping.first();
    size_t last_pos = overlapping.last();
    auto first_chunk = changes_[first_pos];
    auto last_chunk = changes_[last_pos];
    size_t last_size = last_chunk.size();

    if (new_pos > first_pos) {
      new_pos = first_pos;
    }

    for (size_t i = first_pos; i < pos; ++i) {
      new_bytes.append(first_chunk[static_cast<int>(i - first_pos)]);
    }

    for (int i=0; i < bytes.size(); ++i) {
      new_bytes.append(bytes[i]);
    }

    for (size_t i = pos + bytes.size(); i < last_pos + last_size; ++i) {
      new_bytes.append(last_chunk[static_cast<int>(i - last_pos)]);
    }

  } else {
    new_bytes = bytes;
  }

  for (auto pos : overlapping) {
    changes_.remove(pos);
  }

  changes_[new_pos] = new_bytes;

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

void EditEngine::applyChanges(data::BinData &bindata, size_t offset, int64_t max_bytes) const {

  if (max_bytes == -1) {
    max_bytes = bindata.size();
  }

  auto changes = changesFromRange(offset, max_bytes);
  for (auto it = changes.begin(); it != changes.constEnd(); ++it) {
    size_t pos = it.key();
    auto data = it.value();
    for (int i = 0; i < data.size(); ++i) {
      bindata.setElement64(pos - offset + i, data[i]);
    }
  }
}

QPair<size_t, data::BinData> EditEngine::popFirstChange(uint32_t bindata_width) {
  data::BinData bindata(bindata_width, 0);
  size_t pos = 0;

  if (!changes_.isEmpty()) {
    pos = changes_.firstKey();
    auto first_chunk = changes_[pos];
    bindata = data::BinData(bindata_width, first_chunk.size());
    for (int i=0; i < first_chunk.size(); i++) {
      bindata.setElement64(i, first_chunk[i]);
    }
    changes_.remove(pos);
  }

  return QPair<size_t, data::BinData>(pos, bindata);
}

QMap<size_t, QVector<uint64_t>>::const_iterator EditEngine::itFromPos(size_t byte_pos) const {

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

bool EditEngine::isChanged(size_t byte_pos) const {
  return itFromPos(byte_pos) != changes_.constEnd();
}

uint64_t EditEngine::byteValue(size_t byte_pos) const {
  auto it = itFromPos(byte_pos);

  if (it == changes_.constEnd()) {
    return 0;
  }

  size_t pos = it.key();
  auto data = it.value();

  if (byte_pos < pos || pos + data.size() <= byte_pos) {
    return 0;
  }

  return data[static_cast<int>(byte_pos - pos)];
}

bool EditEngine::hasChanges() const {
  return !changes_.isEmpty();
}

QMap<size_t, QVector<uint64_t>> EditEngine::changesFromRange(size_t byte_pos, size_t size) const {
  QMap<size_t, QVector<uint64_t>> res;

  auto it = itFromPos(byte_pos);
  if (it == changes_.constEnd()) {
    it = changes_.upperBound(byte_pos);
  }

  while (it != changes_.constEnd() && it.key() < byte_pos + size) {

    size_t pos = it.key();
    auto data = it.value();

    if (pos < byte_pos) {
      QVector<uint64_t> new_data;

      for (int i = static_cast<int>(byte_pos - pos); i < data.size(); ++i) {
        new_data.append(data[i]);
      }

      data = new_data;
      pos = byte_pos;

    }

    if (pos + data.size() > byte_pos + size) {
      QVector<uint64_t> new_data;
      size_t last_chunk_bytes_count = byte_pos + size - pos;
      for (size_t i = 0; i < last_chunk_bytes_count; ++i) {
        new_data.append(data[static_cast<int>(i)]);
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
