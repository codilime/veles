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
                             bool add_to_history) {
  if (bytes.size() == 0) {
    return;
  }
  assert(pos + bytes.size() <= dataSize());

  if (add_to_history) {
    edit_stack_data_.push_back(bytesValues(pos, bytes.size()));
    edit_stack_.push_back(QPair<size_t, size_t>(pos, bytes.size()));
    while (edit_stack_.size() > edit_stack_limit_) {
      edit_stack_data_.pop_front();
      edit_stack_.pop_front();
    }
  }

  has_changes_ = true;

  const size_t end_pos = pos + bytes.size();
  auto next_it = address_mapping_.upperBound(pos);
  assert(next_it != address_mapping_.cbegin());
  auto it = next_it;
  --it;

  if (next_it == address_mapping_.cend() || end_pos <= next_it.key()) {
    if (it->fragment_ == nullptr) {
      if (address_mapping_.find(end_pos) == address_mapping_.cend()) {
        address_mapping_.insert(
            end_pos, EditNode(nullptr, it->offset_ + (end_pos - it.key())));
      }
      // This node can overwrite previous `*it` and that's OK.
      it = address_mapping_.insert(pos, EditNode(bytes));

      trySquash(it);
    } else {
      it->fragment_->setData(it->offset_ + (pos - it.key()), bytes.size(),
                             bytes);
    }
  } else {
    ++it;
    ++next_it;
    while (next_it != address_mapping_.cend() && next_it.key() < end_pos) {
      it = address_mapping_.erase(it);
      ++next_it;
    }

    EditNode last_modified_node = it.value();
    last_modified_node.offset_ += end_pos - it.key();
    address_mapping_.erase(it);

    if (address_mapping_.find(end_pos) == address_mapping_.cend()) {
      address_mapping_.insert(end_pos, last_modified_node);
    }
    // This can overwrite the first overlapping node and that's OK.
    it = address_mapping_.insert(pos, EditNode(bytes));

    trySquash(it);
  }
}

void EditEngine::insertBytes(size_t pos, const data::BinData& bytes,
                             bool add_to_history) {
  if (bytes.size() == 0) {
    return;
  }

  if (add_to_history) {
    // TODO(catsuryuu)
  }

  has_changes_ = true;

  remap(pos, 0, bytes.size());

  // This can overwrite the node with `pos` key and that's OK.
  auto it = address_mapping_.insert(pos, EditNode(bytes));

  trySquash(it);
}

void EditEngine::removeBytes(size_t pos, size_t size, bool add_to_history) {
  if (size == 0) {
    return;
  }

  if (add_to_history) {
    // TODO(catsuryuu)
  }

  has_changes_ = true;

  remap(pos, size, 0);

  trySquash(address_mapping_.lowerBound(pos));
}

void EditEngine::remapOrigin(size_t origin_pos, size_t old_size,
                             size_t new_size) {
  // This variable can be overflowed when `old_size` > `new_size`,
  // but it will be OK when we add this difference to some address.
  const size_t offset = new_size - old_size;

  auto search_it = address_mapping_.cbegin();
  auto find_local_pos = [this, &search_it](size_t origin_pos) {
    size_t local_pos = origin_pos;
    while (search_it != address_mapping_.cend()) {
      if (search_it->fragment_ == nullptr) {
        if (origin_pos < search_it->offset_) {
          local_pos = std::min(local_pos, search_it.key());
          break;
        }
        local_pos = search_it.key() + (origin_pos - search_it->offset_);
      }
      ++search_it;
    }
    return local_pos;
  };

  const size_t local_pos = find_local_pos(origin_pos);

  const size_t origin_end_pos = origin_pos + old_size;
  const size_t local_end_pos = find_local_pos(origin_end_pos);

  // TODO(catsuryuu): Rewrite origin remapping (without using `remap`)
  // for not losing local changes in deleted range.
  remap(local_pos, local_end_pos - local_pos, new_size);

  auto local_pos_it = address_mapping_.lowerBound(local_pos);
  for (auto mutable_it = local_pos_it; mutable_it != address_mapping_.cend();
       ++mutable_it) {
    if (mutable_it->fragment_ == nullptr) {
      mutable_it->offset_ += offset;
    }
  }

  if (new_size > 0) {
    --local_pos_it;
    if (local_pos_it->fragment_ != nullptr) {
      address_mapping_.insert(local_pos, EditNode(nullptr, origin_pos));
    }
  }
}

size_t EditEngine::undo() {
  if (!hasUndo()) {
    return 0;
  }

  auto range = edit_stack_.back();
  auto last_change = edit_stack_data_.back();

  changeBytes(range.first, last_change, false);

  edit_stack_data_.pop_back();
  edit_stack_.pop_back();

  return range.first;
}

void EditEngine::applyChanges() {
  for (auto it = address_mapping_.cbegin(); it != address_mapping_.cend();
       ++it) {
    if (it->fragment_ != nullptr) {
      auto next_it = it;
      ++next_it;
      assert(next_it != address_mapping_.cend());

      size_t size = next_it.key() - it.key();
      assert(it->offset_ + size <= it->fragment_->size());

      original_data_->uploadNewData(it->fragment_->data(it->offset_, size),
                                    it.key());
    }
  }
  initAddressMapping();
}

void EditEngine::clear() {
  initAddressMapping();
  edit_stack_data_.clear();
  edit_stack_.clear();
}

uint64_t EditEngine::byteValue(size_t pos) const {
  auto next_it = address_mapping_.upperBound(pos);
  assert(next_it != address_mapping_.cbegin());
  auto it = next_it;
  --it;

  size_t offset_in_fragment = it->offset_ + (pos - it.key());
  if (it->fragment_ == nullptr) {
    return original_data_->binData().element64(offset_in_fragment);
  }
  return it->fragment_->element64(offset_in_fragment);
}

data::BinData EditEngine::bytesValues(size_t pos, size_t size) const {
  data::BinData result = data::BinData(original_data_->binData().width(), size);

  const size_t end_pos = pos + size;
  auto next_it = address_mapping_.upperBound(pos);
  assert(next_it != address_mapping_.cbegin());
  auto it = next_it;
  --it;

  if (next_it == address_mapping_.cend() || end_pos <= next_it.key()) {
    result.setData(0, size,
                   getDataFromEditNode(it.value(), pos - it.key(), size));
  } else {
    size_t size_to_write = next_it.key() - pos;
    result.setData(
        0, size_to_write,
        getDataFromEditNode(it.value(), pos - it.key(), size_to_write));
    size_t bytes_written = size_to_write;

    ++it;
    ++next_it;
    while (next_it != address_mapping_.cend() && next_it.key() < end_pos) {
      size_to_write = next_it.key() - it.key();
      result.setData(bytes_written, size_to_write,
                     getDataFromEditNode(it.value(), 0, size_to_write));
      bytes_written += size_to_write;
      ++it;
      ++next_it;
    }

    size_to_write = size - bytes_written;
    result.setData(bytes_written, size_to_write,
                   getDataFromEditNode(it.value(), 0, size_to_write));
  }

  return result;
}

std::vector<bool> EditEngine::modifiedPositions(size_t pos, size_t size) const {
  std::vector<bool> result(size);

  const size_t end_pos = pos + size;
  auto next_it = address_mapping_.upperBound(pos);
  assert(next_it != address_mapping_.cbegin());
  auto it = next_it;
  --it;

  auto set_result_true = [&it, &result](size_t start, size_t end) {
    if (it->fragment_ != nullptr) {
      for (size_t i = start; i < end; ++i) {
        result[i] = true;
      }
    }
  };

  if (next_it == address_mapping_.cend() || end_pos <= next_it.key()) {
    set_result_true(0, size);
  } else {
    set_result_true(0, next_it.key() - pos);
    ++it;
    ++next_it;
    while (next_it != address_mapping_.cend() && next_it.key() < end_pos) {
      set_result_true(it.key() - pos, next_it.key() - pos);
      ++it;
      ++next_it;
    }
    set_result_true(it.key() - pos, size);
  }

  return result;
}

data::BinData EditEngine::getDataFromEditNode(const EditNode& edit_node,
                                              size_t offset,
                                              size_t size) const {
  if (edit_node.fragment_ == nullptr) {
    return original_data_->binData().data(edit_node.offset_ + offset, size);
  }
  return edit_node.fragment_->data(edit_node.offset_ + offset, size);
}

void EditEngine::remap(size_t pos, size_t old_size, size_t new_size) {
  // This variable can be overflowed when `old_size` > `new_size`,
  // but it will be OK when we add this difference to some address.
  const size_t offset = new_size - old_size;

  // iterator with key greater or equal `pos`
  auto it_ge_pos = address_mapping_.lowerBound(pos);

  QMap<size_t, EditNode> new_address_mapping;
  for (auto it = address_mapping_.cbegin(); it != it_ge_pos; ++it) {
    new_address_mapping.insert(it.key(), it.value());
  }

  // end position of the removed data - here we have to split node and save only
  // the suffix.
  const size_t end_pos = pos + old_size;
  auto split_node_next_it = address_mapping_.upperBound(end_pos);
  assert(split_node_next_it != address_mapping_.cbegin());
  auto split_node_it = split_node_next_it;
  --split_node_it;

  new_address_mapping.insert(
      end_pos + offset,
      EditNode(split_node_it->fragment_,
               split_node_it->offset_ + (end_pos - split_node_it.key())));

  for (auto it = split_node_next_it; it != address_mapping_.cend(); ++it) {
    // This can overwrite the `*pos_it` node and that's OK.
    new_address_mapping.insert(it.key() + offset, it.value());
  }

  address_mapping_ = std::move(new_address_mapping);
  data_size_difference_ += offset;
}

/*
 * Warning: it can invalidate iterator `it`
 */
void EditEngine::trySquashWithPrev(const QMap<size_t, EditNode>::iterator& it) {
  if (it == address_mapping_.cbegin()) {
    return;
  }
  auto prev = it;
  --prev;
  if (it->fragment_ == nullptr) {
    // TODO(catsuryuu): implement squashing original fragments later (with
    // deleting feature)
  } else {
    if (prev->fragment_ != nullptr) {
      auto next = it;
      ++next;  // it exists because last EditNode should be from original
      auto prev_size = it.key() - prev.key();
      auto it_size = next.key() - it.key();
      address_mapping_.insert(
          prev.key(), EditNode(prev->fragment_->data(prev->offset_, prev_size) +
                               it->fragment_->data(it->offset_, it_size)));
      address_mapping_.erase(it);
    }
  }
}

void EditEngine::trySquash(const QMap<size_t, EditNode>::iterator& it) {
  auto next = it;
  ++next;
  trySquashWithPrev(it);
  if (next != address_mapping_.cend()) {
    trySquashWithPrev(next);
  }
}

void EditEngine::initAddressMapping() {
  address_mapping_.clear();
  address_mapping_.insert(0, EditNode(nullptr, 0));
  data_size_difference_ = 0;
  has_changes_ = false;
}

}  // namespace util
}  // namespace veles
