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

#include "gtest/gtest.h"
#include "util/misc.h"

namespace veles {
namespace util {

TEST(TestMisc, TestBytesToIntLe) {
  uint8_t data[] = { 0x00 };
  EXPECT_EQ(bytesToIntLe<uint8_t>(data, 1), 0);
  uint8_t data2[] = { 0x12, 0x34 };
  EXPECT_EQ(bytesToIntLe<uint16_t>(data2, 2), 0x3412);
  uint8_t data3[] = { 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0 };
  EXPECT_EQ(bytesToIntLe<uint64_t>(data3, 8), 0xf0debc9a78563412);
}

TEST(TestMisc, TestIntToBytesLe) {
  auto res = intToBytesLe(0, 4);
  uint8_t exp[] = { 0x00, 0x00, 0x00, 0x00 };
  EXPECT_EQ(memcmp(res, exp, 4), 0);
  delete[] res;
  auto res2 = intToBytesLe(0x3412, 2);
  uint8_t exp2[] = { 0x12, 0x34 };
  EXPECT_EQ(memcmp(res2, exp2, 2), 0);
  delete[] res2;
  auto res3 = intToBytesLe(0xf0debc9a78563412, 8);
  uint8_t exp3[] = { 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0 };
  EXPECT_EQ(memcmp(res3, exp3, 8), 0);
  delete[] res3;
}

}  // namespace util
}  // namespace veles
