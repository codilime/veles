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

#include "data/bindata.h"

#include <stdlib.h>

#include <QMap>
#include <QVector>

namespace veles {
namespace util {
namespace edit {

class EditEngine {
 public:
   EditEngine(size_t edit_stack_limit = 0) : edit_stack_limit_(edit_stack_limit) {}

   void changeBytes(size_t pos, const QVector<uint64_t> &bytes, bool add_to_history = true);

   bool hasUndo() const {return !edit_stack_.isEmpty();}
   void undo();

   void applyChanges(data::BinData &data, size_t offset = 0, int64_t max_bytes = -1) const;

   void clearChanges() {changes_.clear();}
   void clearEditStack() {edit_stack_.clear();}

   bool isChanged(size_t byte_pos) const;
   uint64_t byteValue(size_t byte_pos) const;

 private:
   size_t edit_stack_limit_;
   QVector<QMap<size_t, QVector<uint64_t>>> edit_stack_;
   QMap<size_t, QVector<uint64_t>> changes_;


   QMap<size_t, QVector<uint64_t>>::const_iterator itFromPos(size_t pos) const;
   QMap<size_t, QVector<uint64_t>> changesFromRange(size_t pos, size_t size) const;

};

}  // namespace edit
}  // namespace util
}  // namespace veles
