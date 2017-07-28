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
#include "util/encoders/base64_encoder.h"
#include "gtest/gtest.h"

#include <QByteArray>
#include <QString>

namespace veles {
namespace util {
namespace encoders {

TEST(Base64Encoder, encode) {
  auto encoder = Base64Encoder();
  EXPECT_EQ(encoder.encode(QByteArray::fromHex("")), "");
  EXPECT_EQ(encoder.encode(QByteArray::fromHex("01")), "AQ==");
  EXPECT_EQ(encoder.encode(QByteArray::fromHex("0102")), "AQI=");
  EXPECT_EQ(encoder.encode(QByteArray::fromHex("ffffff")), "////");
  EXPECT_EQ(encoder.encode(QByteArray::fromHex("aabbccdd")), "qrvM3Q==");
  EXPECT_EQ(encoder.encode(nullptr, 0), "");
  uint8_t test[] = {0xAA, 0xBB, 0xCC, 0xDD};
  EXPECT_EQ(encoder.encode(test, 4), "qrvM3Q==");
}

TEST(Base64Encoder, decode) {
  auto encoder = Base64Encoder();
  EXPECT_EQ(encoder.decode("AQ=="), QByteArray::fromHex("01"));
  EXPECT_EQ(encoder.decode("AP\n8="), QByteArray::fromHex("00ff"));
  EXPECT_EQ(encoder.decode("qrv$$$M3Q=="), QByteArray::fromHex("aabbccdd"));
  EXPECT_EQ(encoder.decode("qrv$$$M3Q"), QByteArray::fromHex("aabbccdd"));
  EXPECT_EQ(encoder.decode(""), QByteArray::fromHex(""));
  EXPECT_EQ(encoder.decode("a"), QByteArray::fromHex(""));
  EXPECT_EQ(encoder.decode("aa"), QByteArray::fromHex("69"));
}

TEST(Base64Encoder, validate) {
  auto encoder = Base64Encoder();
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
