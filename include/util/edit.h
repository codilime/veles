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

#include <QMap>
#include <QVector>

#include "data/bindata.h"
#include "ui/fileblobmodel.h"

namespace veles {
namespace util {

class EditEngine {
 public:
  explicit EditEngine(ui::FileBlobModel* original_data,
                      int edit_stack_limit = 100)
      : original_data_(original_data), edit_stack_limit_(edit_stack_limit) {
    initAddressMapping();
  }

  void changeBytes(size_t pos, const data::BinData& bytes,
                   bool add_to_history = true);

  bool hasUndo() const { return !edit_stack_.isEmpty(); }
  /** Undo last changeBytes and returns first byte changed by this operation */
  size_t undo();

  bool hasChanges() const { return has_changes_; }
  void applyChanges();
  void clear();

  uint64_t byteValue(size_t pos) const;
  uint64_t originalByteValue(size_t pos) const;
  data::BinData bytesValues(size_t pos, size_t size) const;
  data::BinData originalBytesValues(size_t pos, size_t size) const;

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
  QMap<size_t, EditNode> address_mapping_;
  bool has_changes_;

  int edit_stack_limit_;
  QList<data::BinData> edit_stack_data_;
  QList<QPair<size_t, size_t>> edit_stack_;

  data::BinData getDataFromEditNode(const EditNode& edit_node, size_t offset,
                                    size_t size) const;
  void trySquashWithPrev(const QMap<size_t, EditNode>::iterator& it);
  void trySquash(const QMap<size_t, EditNode>::iterator& it);

  void initAddressMapping();
};

}  // namespace util
}  // namespace veles
