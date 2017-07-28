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
#include "data/repack.h"

#include "gtest/gtest.h"

namespace veles {
namespace data {

TEST(Repack, SimpleCopy) {
  BinData a(8, {1, 2, 3, 4});
  Repacker format{Endian::LITTLE, 8, 8};
  EXPECT_EQ(format.repackUnit(), 8u);
  EXPECT_EQ(format.repackSize(2), 2u);
  BinData b = format.repack(a, 1, 2);
  EXPECT_EQ(b.size(), 2u);
  EXPECT_EQ(b.width(), 8u);
  EXPECT_EQ(b.element64(0), 2u);
  EXPECT_EQ(b.element64(1), 3u);
}

TEST(Repack, Gather8To16Little) {
  BinData a(8, {1, 2, 3, 4, 5, 6});
  Repacker format{Endian::LITTLE, 8, 16};
  EXPECT_EQ(format.repackUnit(), 16u);
  EXPECT_EQ(format.repackSize(2), 4u);
  BinData b = format.repack(a, 1, 2);
  EXPECT_EQ(b.size(), 2u);
  EXPECT_EQ(b.width(), 16u);
  EXPECT_EQ(b.element64(0), 0x302u);
  EXPECT_EQ(b.element64(1), 0x504u);
}

TEST(Repack, Gather8To16Big) {
  BinData a(8, {1, 2, 3, 4, 5, 6});
  Repacker format{Endian::BIG, 8, 16};
  EXPECT_EQ(format.repackUnit(), 16u);
  EXPECT_EQ(format.repackSize(2), 4u);
  BinData b = format.repack(a, 1, 2);
  EXPECT_EQ(b.size(), 2u);
  EXPECT_EQ(b.width(), 16u);
  EXPECT_EQ(b.element64(0), 0x203u);
  EXPECT_EQ(b.element64(1), 0x405u);
}

TEST(Repack, Mash8To12Little) {
  BinData a(8, {0x12, 0x34, 0x56, 0x78, 0x9a});
  Repacker format{Endian::LITTLE, 8, 12};
  EXPECT_EQ(format.repackUnit(), 24u);
  EXPECT_EQ(format.repackSize(2), 3u);
  BinData b = format.repack(a, 1, 2);
  EXPECT_EQ(b.size(), 2u);
  EXPECT_EQ(b.width(), 12u);
  EXPECT_EQ(b.element64(0), 0x634u);
  EXPECT_EQ(b.element64(1), 0x785u);
}

TEST(Repack, Mash8To12Big) {
  BinData a(8, {0x12, 0x34, 0x56, 0x78, 0x9a});
  Repacker format{Endian::BIG, 8, 12};
  EXPECT_EQ(format.repackUnit(), 24u);
  EXPECT_EQ(format.repackSize(2), 3u);
  BinData b = format.repack(a, 1, 2);
  EXPECT_EQ(b.size(), 2u);
  EXPECT_EQ(b.width(), 12u);
  EXPECT_EQ(b.element64(0), 0x345u);
  EXPECT_EQ(b.element64(1), 0x678u);
}

TEST(Repack, Ugly8To12Little) {
  BinData a(8, {0x12, 0x34, 0x56});
  Repacker format{Endian::LITTLE, 8, 12};
  EXPECT_EQ(format.repackUnit(), 24u);
  EXPECT_EQ(format.repackSize(1), 2u);
  BinData b = format.repack(a, 1, 1);
  EXPECT_EQ(b.size(), 1u);
  EXPECT_EQ(b.width(), 12u);
  EXPECT_EQ(b.element64(), 0x634u);
}

TEST(Repack, Ugly8To12Big) {
  BinData a(8, {0x12, 0x34, 0x56});
  Repacker format{Endian::BIG, 8, 12};
  EXPECT_EQ(format.repackUnit(), 24u);
  EXPECT_EQ(format.repackSize(1), 2u);
  BinData b = format.repack(a, 1, 1);
  EXPECT_EQ(b.size(), 1u);
  EXPECT_EQ(b.width(), 12u);
  EXPECT_EQ(b.element64(), 0x345u);
}

TEST(Repack, Split8To1Little) {
  BinData a(8, {0x12, 0x34, 0x56});
  Repacker format{Endian::LITTLE, 8, 1};
  EXPECT_EQ(format.repackUnit(), 8u);
  EXPECT_EQ(format.repackSize(12), 2u);
  BinData b = format.repack(a, 1, 12);
  EXPECT_EQ(b.size(), 12u);
  EXPECT_EQ(b.width(), 1u);
  for (int i = 0; i < 12; i++) {
    EXPECT_EQ(b.element64(i), (0x634u >> i) & 1);
  }
}

TEST(Repack, Split8To1Big) {
  BinData a(8, {0x12, 0x34, 0x56});
  Repacker format{Endian::BIG, 8, 1};
  EXPECT_EQ(format.repackUnit(), 8u);
  EXPECT_EQ(format.repackSize(12), 2u);
  BinData b = format.repack(a, 1, 12);
  EXPECT_EQ(b.size(), 12u);
  EXPECT_EQ(b.width(), 1u);
  for (int i = 0; i < 12; i++) {
    EXPECT_EQ(b.element64(i), (0x345u >> (11 - i)) & 1);
  }
}

TEST(Repack, Split60To20Little) {
  BinData a(60, {0xfedcba987654321ull});
  Repacker format{Endian::LITTLE, 60, 20};
  EXPECT_EQ(format.repackUnit(), 60u);
  EXPECT_EQ(format.repackSize(3), 1u);
  BinData b = format.repack(a, 0, 3);
  EXPECT_EQ(b.size(), 3u);
  EXPECT_EQ(b.width(), 20u);
  EXPECT_EQ(b.element64(0), 0x54321u);
  EXPECT_EQ(b.element64(1), 0xa9876u);
  EXPECT_EQ(b.element64(2), 0xfedcbu);
}

TEST(Repack, Split60To20Big) {
  BinData a(60, {0xfedcba987654321ull});
  Repacker format{Endian::BIG, 60, 20};
  EXPECT_EQ(format.repackUnit(), 60u);
  EXPECT_EQ(format.repackSize(3), 1u);
  BinData b = format.repack(a, 0, 3);
  EXPECT_EQ(b.size(), 3u);
  EXPECT_EQ(b.width(), 20u);
  EXPECT_EQ(b.element64(0), 0xfedcbu);
  EXPECT_EQ(b.element64(1), 0xa9876u);
  EXPECT_EQ(b.element64(2), 0x54321u);
}

TEST(Repack, Split16To8Little) {
  BinData a(16, {0x1234, 0x5678, 0x9abc});
  Repacker format{Endian::LITTLE, 16, 8};
  EXPECT_EQ(format.repackUnit(), 16u);
  EXPECT_EQ(format.repackSize(3), 2u);
  BinData b = format.repack(a, 1, 3);
  EXPECT_EQ(b.size(), 3u);
  EXPECT_EQ(b.width(), 8u);
  EXPECT_EQ(b.element64(0), 0x78u);
  EXPECT_EQ(b.element64(1), 0x56u);
  EXPECT_EQ(b.element64(2), 0xbcu);
}

TEST(Repack, Split16To8Big) {
  BinData a(16, {0x1234, 0x5678, 0x9abc});
  Repacker format{Endian::BIG, 16, 8};
  EXPECT_EQ(format.repackUnit(), 16u);
  EXPECT_EQ(format.repackSize(3), 2u);
  BinData b = format.repack(a, 1, 3);
  EXPECT_EQ(b.size(), 3u);
  EXPECT_EQ(b.width(), 8u);
  EXPECT_EQ(b.element64(0), 0x56u);
  EXPECT_EQ(b.element64(1), 0x78u);
  EXPECT_EQ(b.element64(2), 0x9au);
}

TEST(Repack, Padded8to23LeftLittle) {
  BinData a(8, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa});
  Repacker format{Endian::LITTLE, 8, 23, 9, 0};
  EXPECT_EQ(format.repackUnit(), 32u);
  EXPECT_EQ(format.repackSize(2), 8u);
  BinData b = format.repack(a, 1, 2);
  EXPECT_EQ(b.size(), 2u);
  EXPECT_EQ(b.width(), 23u);
  EXPECT_EQ(b.element64(0), 0x443322u);
  EXPECT_EQ(b.element64(1), 0x087766u);
}

TEST(Repack, Padded8to23RightLittle) {
  BinData a(8, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa});
  Repacker format{Endian::LITTLE, 8, 23, 0, 9};
  EXPECT_EQ(format.repackUnit(), 32u);
  EXPECT_EQ(format.repackSize(2), 8u);
  BinData b = format.repack(a, 1, 2);
  EXPECT_EQ(b.size(), 2u);
  EXPECT_EQ(b.width(), 23u);
  EXPECT_EQ(b.element64(0), 0x2aa219u);
  EXPECT_EQ(b.element64(1), 0x4cc43bu);
}

TEST(Repack, Padded8to23MixedLittle) {
  BinData a(8, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa});
  Repacker format{Endian::LITTLE, 8, 23, 1, 8};
  EXPECT_EQ(format.repackUnit(), 32u);
  EXPECT_EQ(format.repackSize(2), 8u);
  BinData b = format.repack(a, 1, 2);
  EXPECT_EQ(b.size(), 2u);
  EXPECT_EQ(b.width(), 23u);
  EXPECT_EQ(b.element64(0), 0x554433u);
  EXPECT_EQ(b.element64(1), 0x198877u);
}

TEST(Repack, Padded8to23LeftBig) {
  BinData a(8, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa});
  Repacker format{Endian::BIG, 8, 23, 9, 0};
  EXPECT_EQ(format.repackUnit(), 32u);
  EXPECT_EQ(format.repackSize(2), 8u);
  BinData b = format.repack(a, 1, 2);
  EXPECT_EQ(b.size(), 2u);
  EXPECT_EQ(b.width(), 23u);
  EXPECT_EQ(b.element64(0), 0x334455u);
  EXPECT_EQ(b.element64(1), 0x778899u);
}

TEST(Repack, Padded8to23RightBig) {
  BinData a(8, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa});
  Repacker format{Endian::BIG, 8, 23, 0, 9};
  EXPECT_EQ(format.repackUnit(), 32u);
  EXPECT_EQ(format.repackSize(2), 8u);
  BinData b = format.repack(a, 1, 2);
  EXPECT_EQ(b.size(), 2u);
  EXPECT_EQ(b.width(), 23u);
  EXPECT_EQ(b.element64(0), 0x1119a2u);
  EXPECT_EQ(b.element64(1), 0x333bc4u);
}

TEST(Repack, Padded8to23MixedBig) {
  BinData a(8, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa});
  Repacker format{Endian::BIG, 8, 23, 1, 8};
  EXPECT_EQ(format.repackUnit(), 32u);
  EXPECT_EQ(format.repackSize(2), 8u);
  BinData b = format.repack(a, 1, 2);
  EXPECT_EQ(b.size(), 2u);
  EXPECT_EQ(b.width(), 23u);
  EXPECT_EQ(b.element64(0), 0x223344u);
  EXPECT_EQ(b.element64(1), 0x667788u);
}

TEST(Repack, MsgpackConversion) {
  auto format = std::make_shared<Repacker>(Endian::BIG, 8, 23, 1, 8);
  auto obj = messages::toMsgpackObject(format);
  EXPECT_EQ(*(*obj->getMap())["endian"]->getString(), "BIG");
  EXPECT_EQ((*obj->getMap())["from_width"]->getUnsignedInt(), 8u);
  EXPECT_EQ((*obj->getMap())["to_width"]->getUnsignedInt(), 23u);
  EXPECT_EQ((*obj->getMap())["high_pad"]->getUnsignedInt(), 1u);
  EXPECT_EQ((*obj->getMap())["low_pad"]->getUnsignedInt(), 8u);
  std::shared_ptr<Repacker> ptr;
  messages::fromMsgpackObject(obj, &ptr);
  EXPECT_EQ(ptr->endian, Endian::BIG);
  EXPECT_EQ(ptr->from_width, 8u);
  EXPECT_EQ(ptr->to_width, 23u);
  EXPECT_EQ(ptr->high_pad, 1u);
  EXPECT_EQ(ptr->low_pad, 8u);
}

}  // namespace data
}  // namespace veles
