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
namespace edit {

void EditEngine::changeBytes(size_t pos, const QVector<uint64_t> &bytes, bool add_to_history) {
  if (add_to_history) {
    edit_stack_.push_back(changesFromRange(pos, bytes.size()));
    if (edit_stack_.size() >= edit_stack_limit_) {
      edit_stack_.pop_front();
    }
  }

  //TODO: merge changes

  changes_[pos] = bytes;
}

void EditEngine::undo() {
  if (!hasUndo()) {
    return;
  }

  auto last_change = edit_stack_.back();

  for (auto it = last_change.begin(); it != last_change.end(); it++) {
     changeBytes(it.key(), it.value(), false);
  }

  edit_stack_.pop_back();
}

void EditEngine::applyChanges(data::BinData &bindata, size_t offset, int64_t max_bytes) const {
   auto changes = changesFromRange(offset, max_bytes);

   for (auto it = changes.begin(); it != changes.end(); it++) {
     size_t pos = it.key();
     auto data = it.value();
     for (uint64_t i = 0; i < data.size(); it++) {
       bindata.setElement64(pos - offset + i, data[i]);
     }
   }
}

QMap<size_t, QVector<uint64_t>>::const_iterator EditEngine::itFromPos(size_t byte_pos) const {
  auto it = changes_.upperBound(byte_pos);
  it--;
  if (it == changes_.end()) {
    return it;
  }

  size_t pos = it.key();
  auto data = it.value();

  if (byte_pos < pos || (byte_pos > pos + data.size())) {
    return changes_.end();
  }

  return it;
}

bool EditEngine::isChanged(size_t byte_pos) const {
  return itFromPos(byte_pos) != changes_.end();
}

uint64_t EditEngine::byteValue(size_t byte_pos) const {

  auto it = itFromPos(byte_pos);

  if (it == changes_.end()) {
    return 0;
  }

  size_t pos = it.key();
  auto data = it.value();

  if (byte_pos < pos || pos + data.size() <= byte_pos) {
    return 0;
  }

  return data[static_cast<int>(byte_pos - pos)];
}

QMap<size_t, QVector<uint64_t>> EditEngine::changesFromRange(size_t byte_pos, size_t size) const {
  QMap<size_t, QVector<uint64_t>> res;

  auto it = itFromPos(byte_pos);

  if (it == changes_.end()) {
    return res;
  }

  while (it.key() < byte_pos + size) {

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

    if (byte_pos + size > pos + data.size()) {
      QVector<uint64_t> new_data;
      for (auto i = 0; i < 2*data.size() - byte_pos - size + pos; ++i) {
        new_data.append(data[i]);
      }

      data = new_data;
    }

    res[pos] = data;
  }

  return res;
}

}  // namespace edit
}  // namespace util
}  // namespace veles
