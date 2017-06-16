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

#include "gtest/gtest.h"

using namespace testing;

namespace veles {
namespace util {

TEST(TestEditEngine, TestChangeBytes) {
   EditEngine edit_engine(8);

   ASSERT_FALSE(edit_engine.hasChanges());
   ASSERT_FALSE(edit_engine.isChanged(0));
   edit_engine.changeBytes(0, data::BinData(8, {1}), data::BinData(8, {2}));
   ASSERT_TRUE(edit_engine.hasChanges());
   ASSERT_TRUE(edit_engine.isChanged(0));
   ASSERT_FALSE(edit_engine.isChanged(1));
   ASSERT_EQ(edit_engine.byteValue(0), 1);
   edit_engine.changeBytes(2, data::BinData(8, {2}), data::BinData(8, {2}));
   edit_engine.changeBytes(1, data::BinData(8, {3}), data::BinData(8, {2}));
   auto change = edit_engine.popFirstChange();
   ASSERT_FALSE(edit_engine.hasChanges());
   ASSERT_EQ(change.first, 0);
   ASSERT_EQ(change.second, data::BinData(8, {1,3,2}));
}

TEST(TestEditEngine, TestApplyChanges) {
   EditEngine edit_engine(8);

   edit_engine.changeBytes(10, data::BinData(8, {1}), data::BinData(8, {2}));
   edit_engine.changeBytes(11, data::BinData(8, {2}), data::BinData(8, {2}));

   data::BinData bindata(8, {0, 0, 0, 0, 0});
   edit_engine.applyChanges(bindata, 8, 3);
   ASSERT_EQ(bindata, data::BinData(8, {0, 0, 1, 0, 0}));
   edit_engine.applyChanges(bindata, 8, 4);
   ASSERT_EQ(bindata, data::BinData(8, {0, 0, 1, 2, 0}));
   edit_engine.applyChanges(bindata, 9, 5);
   ASSERT_EQ(bindata, data::BinData(8, {0, 1, 2, 2, 0}));
}

TEST(TestEditEngine, TestUndo) {
   EditEngine edit_engine(8, 3);

   ASSERT_FALSE(edit_engine.hasUndo());
   edit_engine.changeBytes(0, data::BinData(8, {2}), data::BinData(8, {1}));
   ASSERT_TRUE(edit_engine.hasUndo());
   edit_engine.changeBytes(0, data::BinData(8, {3}), data::BinData(8, {2}));
   edit_engine.changeBytes(0, data::BinData(8, {4}), data::BinData(8, {3}));
   edit_engine.changeBytes(0, data::BinData(8, {5}), data::BinData(8, {4}));
   edit_engine.undo();
   ASSERT_EQ(edit_engine.byteValue(0), 4);
   ASSERT_TRUE(edit_engine.hasUndo());
   edit_engine.undo();
   ASSERT_EQ(edit_engine.byteValue(0), 3);
   ASSERT_TRUE(edit_engine.hasUndo());
   edit_engine.undo();
   ASSERT_EQ(edit_engine.byteValue(0), 2);
   ASSERT_FALSE(edit_engine.hasUndo());
   edit_engine.changeBytes(0, data::BinData(8, {3}), data::BinData(8, {2}), false);
   ASSERT_FALSE(edit_engine.hasUndo());
}

}  // namespace util
}  // namespace veles
