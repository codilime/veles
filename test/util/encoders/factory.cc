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
#include "util/encoders/factory.h"
#include "gtest/gtest.h"
#include "util/encoders/base64_encoder.h"
#include "util/encoders/hex_encoder.h"

namespace veles {
namespace util {
namespace encoders {

TEST(Factory, keys) {
  auto keys = EncodersFactory::keys();
  ASSERT_TRUE(keys.contains("hex"));
  ASSERT_TRUE(keys.contains("base64"));
}

TEST(Factory, createHex) {
  auto encoder = EncodersFactory::createEncoder("hex");
  EXPECT_NE(encoder, nullptr);
  EXPECT_NE(dynamic_cast<HexEncoder*>(encoder), nullptr);
  delete encoder;
}

TEST(Factory, createBase64) {
  auto encoder = EncodersFactory::createEncoder("base64");
  EXPECT_NE(encoder, nullptr);
  EXPECT_NE(dynamic_cast<Base64Encoder*>(encoder), nullptr);
  delete encoder;
}

TEST(Factory, createNotExisting) {
  auto encoder = EncodersFactory::createEncoder("notExisting");
  EXPECT_EQ(encoder, nullptr);
}

}  // namespace encoders
}  // namespace util
}  // namespace veles
