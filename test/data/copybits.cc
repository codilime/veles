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
#include "data/bindata.h"
#include "gtest/gtest.h"

namespace veles {
namespace data {

TEST(CopyTest, SimpleCopy) {
  uint8_t dst[3] = {0};
  const uint8_t src[3] = {0x12, 0x34, 0x56};
  BinData::copyBits(dst, 0, src, 0, 18);
  EXPECT_EQ(dst[0], 0x12);
  EXPECT_EQ(dst[1], 0x34);
  EXPECT_EQ(dst[2], 0x02);
}

TEST(CopyTest, SimpleCopyOffset) {
  uint8_t dst[4] = {0};
  const uint8_t src[4] = {0x12, 0x34, 0x56, 0x78};
  BinData::copyBits(dst, 2, src, 2, 26);
  EXPECT_EQ(dst[0], 0x10);
  EXPECT_EQ(dst[1], 0x34);
  EXPECT_EQ(dst[2], 0x56);
  EXPECT_EQ(dst[3], 0x08);
}

TEST(CopyTest, SimpleMask) {
  uint8_t dst[4] = {0xff, 0xff, 0xff, 0xff};
  const uint8_t src[4] = {0, 0, 0, 0};
  BinData::copyBits(dst, 2, src, 2, 26);
  EXPECT_EQ(dst[0], 0x03);
  EXPECT_EQ(dst[1], 0x00);
  EXPECT_EQ(dst[2], 0x00);
  EXPECT_EQ(dst[3], 0xf0);
}

TEST(CopyTest, MicroInsert) {
  uint8_t dst[1] = {0xff};
  const uint8_t src[1] = {6};
  BinData::copyBits(dst, 2, src, 0, 4);
  EXPECT_EQ(dst[0], 0xdb);
}

TEST(CopyTest, MicroExtract) {
  uint8_t dst[1] = {0xff};
  const uint8_t src[1] = {0xdb};
  BinData::copyBits(dst, 0, src, 2, 4);
  EXPECT_EQ(dst[0], 0xf6);
}

TEST(CopyTest, Shift) {
  uint8_t dst[5] = {0xaa, 0xaa, 0xaa, 0xaa, 0xaa};
  const uint8_t src[4] = {0x10, 0x32, 0x54, 0x76};
  BinData::copyBits(dst, 4, src, 0, 32);
  EXPECT_EQ(dst[0], 0x0a);
  EXPECT_EQ(dst[1], 0x21);
  EXPECT_EQ(dst[2], 0x43);
  EXPECT_EQ(dst[3], 0x65);
  EXPECT_EQ(dst[4], 0xa7);
}

TEST(CopyTest, ShiftOffset) {
  uint8_t dst[8] = {0x55, 0x55, 0x55, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa};
  const uint8_t src[6] = {0x33, 0x33, 0x10, 0x32, 0x54, 0x76};
  BinData::copyBits(dst, 28, src, 16, 32);
  EXPECT_EQ(dst[0], 0x55);
  EXPECT_EQ(dst[1], 0x55);
  EXPECT_EQ(dst[2], 0x55);
  EXPECT_EQ(dst[3], 0x0a);
  EXPECT_EQ(dst[4], 0x21);
  EXPECT_EQ(dst[5], 0x43);
  EXPECT_EQ(dst[6], 0x65);
  EXPECT_EQ(dst[7], 0xa7);
}

}  // namespace data
}  // namespace veles
