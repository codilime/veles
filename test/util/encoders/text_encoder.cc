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
#include "util/encoders/text_encoder.h"

#include "gtest/gtest.h"

#include <QByteArray>
#include <QString>

namespace veles {
namespace util {
namespace encoders {

TEST(TextEncoder, encode) {
  auto encoder = TextEncoder();
  EXPECT_EQ(encoder.encode(QByteArray::fromHex("")), "");
  EXPECT_EQ(encoder.encode(QByteArray::fromHex("0041000041")), "AA");
  EXPECT_EQ(encoder.encode(QByteArray::fromHex("0708090a0b0c0d")),
            "\a\b\t\n\v\f\r");
  EXPECT_EQ(encoder.encode(nullptr, 0), "");
  uint8_t test[] = {0x00, 0x41, 0x00, 0x00, 0x42};
  EXPECT_EQ(encoder.encode(test, 5), "AB");
}

TEST(TextEncoder, decode) {
  auto encoder = TextEncoder();
  EXPECT_EQ(encoder.decode(""), QByteArray::fromHex(""));
  EXPECT_EQ(encoder.decode(QString::fromLatin1("\0a\0b\0", 5)),
            QByteArray::fromHex("0061006200"));
  EXPECT_EQ(encoder.decode("\a\b\t\n\v\f\r"),
            QByteArray::fromHex("0708090a0b0c0d"));
}

TEST(TextEncoder, validate) {
  auto encoder = TextEncoder();
  auto test = [&encoder](const QByteArray& bytes) {
    EXPECT_EQ(encoder.decode(encoder.encode(bytes)), bytes);
  };
  auto test_str = [&test](const QString& str) {
    QByteArray bytes = str.toLatin1();
    test(bytes);
  };

  test_str("aaaa");
  test_str("test_string_asdf\n\r\r\n\0asdf");
  test_str("");
  test_str(QString(1024, 'x'));
  QByteArray all_bytes;  // excluding null
  for (int i = 1; i < 256; i++) {
    all_bytes.append(static_cast<char>(i));
  }
  test(all_bytes);
}

}  // namespace encoders
}  // namespace util
}  // namespace veles
