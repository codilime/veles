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
#pragma once

#include <cstdlib>
#include <memory>
#include <vector>

#include <QMap>

#include "data/bindata.h"
#include "ui/fileblobmodel.h"

namespace veles {
namespace util {

/**
 * EditEngine is an abstraction layer for FileBlobModel, which keeps current
 * (local) changes (before uploading them to the backend) and editing history.
 * It works in quite efficient way even on huge files.
 */

class EditEngine {
 public:
  explicit EditEngine(ui::FileBlobModel* original_data,
                      int edit_stack_limit = 100)
      : original_data_(original_data), edit_stack_limit_(edit_stack_limit) {
    initAddressMapping();
  }

  /**
   * Substitutes `bytes.size()` bytes starting from position `pos`
   * for values from `bytes`.
   */
  void modifyBytes(size_t pos, const data::BinData& bytes,
                   bool add_to_history = true);
  /**
   * Inserts data from `bytes` in position `pos`.
   * Works in pessimistic time O(numberOfLocalEdits + bytes.size()).
   */
  void insertBytes(size_t pos, const data::BinData& bytes,
                   bool add_to_history = true);
  /**
   * Removes `size` bytes starting from position `pos`.
   * Works in pessimistic time O(numberOfLocalEdits).
   */
  void removeBytes(size_t pos, size_t size, bool add_to_history = true);
  /**
   * Updates the EditEngine after remapping request from server.
   * It can occur e.g. when someone from the other client do an insert
   * or a removal.
   * Works in pessimistic time O(numberOfLocalEdits).
   */
  void remapOrigin(size_t origin_pos, size_t old_size, size_t new_size);

  /** Return whether there is any change to undo. */
  bool hasUndo() const { return !edit_stack_.isEmpty(); }
  /** Undo last change and returns first byte changed by this operation. */
  // TODO(catsuryuu): `undo` works only with `modifyBytes`
  size_t undo();

  bool hasChanges() const { return has_changes_; }
  /**
   * Uploads all local changes to FileBlobModel.
   * After applying there is no local changes left.
   */
  void applyChanges();
  /** Removes all local changes. */
  void clear();

  /** Returns size of local data state. */
  size_t dataSize() const {
    return original_data_->binData().size() + data_size_difference_;
  }
  /** Returns value of byte from position `pos`. */
  uint64_t byteValue(size_t pos) const;
  /** Returns `size` bytes starting from position `pos`. */
  data::BinData bytesValues(size_t pos, size_t size) const;
  /**
   * Returns `size` boolean values that correspond to bytes starting from
   * position `pos` and indicates whether specific bytes have been changed
   * or not.
   */
  std::vector<bool> modifiedPositions(size_t pos, size_t size) const;

 private:
  struct EditNode {
    std::shared_ptr<data::BinData> fragment_;
    size_t offset_;

    EditNode(const std::shared_ptr<data::BinData>& fragment, size_t offset)
        : fragment_(fragment), offset_(offset) {}
    explicit EditNode(const data::BinData& bindata)
        : fragment_(std::make_shared<data::BinData>(bindata)), offset_(0) {}
  };

  ui::FileBlobModel* original_data_;
  // TODO(catsuryuu): change to std::map after switching to C++17 (and use
  // `insert_or_assign`)
  // Reason: QMap uses int for container size
  QMap<size_t, EditNode> address_mapping_;
  size_t data_size_difference_;
  bool has_changes_;

  int edit_stack_limit_;
  QList<data::BinData> edit_stack_data_;
  QList<QPair<size_t, size_t>> edit_stack_;

  data::BinData getDataFromEditNode(const EditNode& edit_node, size_t offset,
                                    size_t size) const;
  void remap(size_t pos, size_t old_size, size_t new_size);
  void trySquashWithPrev(const QMap<size_t, EditNode>::iterator& it);
  void trySquash(const QMap<size_t, EditNode>::iterator& it);

  void initAddressMapping();
};

}  // namespace util
}  // namespace veles
