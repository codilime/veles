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
#include "util/encoders/hex_encoder.h"


namespace veles {
namespace util {
namespace encoders {

TEST(HexEncoder, encode) {
  auto encoder = HexEncoder();

  EXPECT_EQ(encoder.encode(QByteArray::fromHex("01")), "01");
  EXPECT_EQ(encoder.encode(QByteArray::fromHex("0102")), "0102");
  EXPECT_EQ(encoder.encode(QByteArray::fromHex("ffff")), "ffff");
}

TEST(HexEncoder, decode) {
  auto encoder = HexEncoder();
  EXPECT_EQ(encoder.decode("01"), QByteArray::fromHex("01"));
  EXPECT_EQ(encoder.decode("01 02"), QByteArray::fromHex("0102"));
  EXPECT_EQ(encoder.decode("00\nff"), QByteArray::fromHex("00ff"));
  EXPECT_EQ(encoder.decode("00\nFFz"), QByteArray::fromHex("00ff"));
}

TEST(HexEncoder, validate) {
  auto encoder = HexEncoder();

  ASSERT_TRUE(encoder.validateEncoded("01"));
  ASSERT_TRUE(encoder.validateEncoded("01aa"));
  ASSERT_TRUE(encoder.validateEncoded("01 aa"));
  ASSERT_TRUE(encoder.validateEncoded("01 Aa\nbB"));

  ASSERT_FALSE(encoder.validateEncoded("zz"));
  ASSERT_FALSE(encoder.validateEncoded("AAA"));
}



}  // namespace encoders
}  // namespace util
}  // namespace veles
