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
#include "util/encoders/hex_encoder.h"
#include "gtest/gtest.h"

#include <QByteArray>

namespace veles {
namespace util {
namespace encoders {

TEST(HexEncoder, encode) {
  auto encoder = HexEncoder();

  EXPECT_EQ(encoder.encode(QByteArray::fromHex("")), "");
  EXPECT_EQ(encoder.encode(QByteArray::fromHex("01")), "01");
  EXPECT_EQ(encoder.encode(QByteArray::fromHex("0102")), "0102");
  EXPECT_EQ(encoder.encode(QByteArray::fromHex("ffff")), "ffff");
  EXPECT_EQ(encoder.encode(nullptr, 0), "");
  uint8_t test[] = {0x01, 0x02, 0xFF};
  EXPECT_EQ(encoder.encode(test, 3), "0102ff");
}

TEST(HexEncoder, decode) {
  auto encoder = HexEncoder();
  EXPECT_EQ(encoder.decode(""), QByteArray::fromHex(""));
  EXPECT_EQ(encoder.decode("01"), QByteArray::fromHex("01"));
  EXPECT_EQ(encoder.decode("01 02"), QByteArray::fromHex("0102"));
  EXPECT_EQ(encoder.decode("00\nff"), QByteArray::fromHex("00ff"));
  EXPECT_EQ(encoder.decode("00,11\nfFz"), QByteArray::fromHex("0011ff"));
}

TEST(HexEncoder, validate) {
  auto encoder = HexEncoder();
  auto test = [&encoder](const QByteArray& bytes) {
    EXPECT_EQ(encoder.decode(encoder.encode(bytes)), bytes);
  };
  auto test_str = [&encoder, &test](const QString& str) {
    QByteArray bytes = str.toLatin1();
    test(bytes);
  };

  test_str("aaaa");
  test_str("test_string_asdf\n\r\r\n\0asdf");
  test_str("");
  test_str(QString(1024, 'x'));
  QByteArray allBytes;
  for (int i = 0; i < 256; i++) {
    allBytes.append(static_cast<char>(i));
  }
  test(allBytes);
}

}  // namespace encoders
}  // namespace util
}  // namespace veles
