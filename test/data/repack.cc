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
#include "data/repack.h"

namespace veles {
namespace data {

TEST(Repack, SimpleCopy) {
  BinData a(8, {1, 2, 3, 4});
  RepackFormat format{RepackEndian::LITTLE, 8};
  EXPECT_EQ(repackUnit(a.width(), format), 8);
  EXPECT_EQ(repackSize(a.width(), format, 2), 2);
  BinData b = repack(a, format, 1, 2);
  EXPECT_EQ(b.size(), 2);
  EXPECT_EQ(b.width(), 8);
  EXPECT_EQ(b.element64(0), 2);
  EXPECT_EQ(b.element64(1), 3);
}

TEST(Repack, Gather8To16Little) {
  BinData a(8, {1, 2, 3, 4, 5, 6});
  RepackFormat format{RepackEndian::LITTLE, 16};
  EXPECT_EQ(repackUnit(a.width(), format), 16);
  EXPECT_EQ(repackSize(a.width(), format, 2), 4);
  BinData b = repack(a, format, 1, 2);
  EXPECT_EQ(b.size(), 2);
  EXPECT_EQ(b.width(), 16);
  EXPECT_EQ(b.element64(0), 0x302);
  EXPECT_EQ(b.element64(1), 0x504);
}

TEST(Repack, Gather8To16Big) {
  BinData a(8, {1, 2, 3, 4, 5, 6});
  RepackFormat format{RepackEndian::BIG, 16};
  EXPECT_EQ(repackUnit(a.width(), format), 16);
  EXPECT_EQ(repackSize(a.width(), format, 2), 4);
  BinData b = repack(a, format, 1, 2);
  EXPECT_EQ(b.size(), 2);
  EXPECT_EQ(b.width(), 16);
  EXPECT_EQ(b.element64(0), 0x203);
  EXPECT_EQ(b.element64(1), 0x405);
}

TEST(Repack, Mash8To12Little) {
  BinData a(8, {0x12, 0x34, 0x56, 0x78, 0x9a});
  RepackFormat format{RepackEndian::LITTLE, 12};
  EXPECT_EQ(repackUnit(a.width(), format), 24);
  EXPECT_EQ(repackSize(a.width(), format, 2), 3);
  BinData b = repack(a, format, 1, 2);
  EXPECT_EQ(b.size(), 2);
  EXPECT_EQ(b.width(), 12);
  EXPECT_EQ(b.element64(0), 0x634);
  EXPECT_EQ(b.element64(1), 0x785);
}

TEST(Repack, Mash8To12Big) {
  BinData a(8, {0x12, 0x34, 0x56, 0x78, 0x9a});
  RepackFormat format{RepackEndian::BIG, 12};
  EXPECT_EQ(repackUnit(a.width(), format), 24);
  EXPECT_EQ(repackSize(a.width(), format, 2), 3);
  BinData b = repack(a, format, 1, 2);
  EXPECT_EQ(b.size(), 2);
  EXPECT_EQ(b.width(), 12);
  EXPECT_EQ(b.element64(0), 0x345);
  EXPECT_EQ(b.element64(1), 0x678);
}

TEST(Repack, Ugly8To12Little) {
  BinData a(8, {0x12, 0x34, 0x56});
  RepackFormat format{RepackEndian::LITTLE, 12};
  EXPECT_EQ(repackUnit(a.width(), format), 24);
  EXPECT_EQ(repackSize(a.width(), format, 1), 2);
  BinData b = repack(a, format, 1, 1);
  EXPECT_EQ(b.size(), 1);
  EXPECT_EQ(b.width(), 12);
  EXPECT_EQ(b.element64(), 0x634);
}

TEST(Repack, Ugly8To12Big) {
  BinData a(8, {0x12, 0x34, 0x56});
  RepackFormat format{RepackEndian::BIG, 12};
  EXPECT_EQ(repackUnit(a.width(), format), 24);
  EXPECT_EQ(repackSize(a.width(), format, 1), 2);
  BinData b = repack(a, format, 1, 1);
  EXPECT_EQ(b.size(), 1);
  EXPECT_EQ(b.width(), 12);
  EXPECT_EQ(b.element64(), 0x345);
}

TEST(Repack, Split8To1Little) {
  BinData a(8, {0x12, 0x34, 0x56});
  RepackFormat format{RepackEndian::LITTLE, 1};
  EXPECT_EQ(repackUnit(a.width(), format), 8);
  EXPECT_EQ(repackSize(a.width(), format, 12), 2);
  BinData b = repack(a, format, 1, 12);
  EXPECT_EQ(b.size(), 12);
  EXPECT_EQ(b.width(), 1);
  for (int i = 0; i < 12; i++)
    EXPECT_EQ(b.element64(i), (0x634 >> i) & 1);
}

TEST(Repack, Split8To1Big) {
  BinData a(8, {0x12, 0x34, 0x56});
  RepackFormat format{RepackEndian::BIG, 1};
  EXPECT_EQ(repackUnit(a.width(), format), 8);
  EXPECT_EQ(repackSize(a.width(), format, 12), 2);
  BinData b = repack(a, format, 1, 12);
  EXPECT_EQ(b.size(), 12);
  EXPECT_EQ(b.width(), 1);
  for (int i = 0; i < 12; i++)
    EXPECT_EQ(b.element64(i), (0x345 >> (11 - i)) & 1);
}

TEST(Repack, Split60To20Little) {
  BinData a(60, {0xfedcba987654321ull});
  RepackFormat format{RepackEndian::LITTLE, 20};
  EXPECT_EQ(repackUnit(a.width(), format), 60);
  EXPECT_EQ(repackSize(a.width(), format, 3), 1);
  BinData b = repack(a, format, 0, 3);
  EXPECT_EQ(b.size(), 3);
  EXPECT_EQ(b.width(), 20);
  EXPECT_EQ(b.element64(0), 0x54321);
  EXPECT_EQ(b.element64(1), 0xa9876);
  EXPECT_EQ(b.element64(2), 0xfedcb);
}

TEST(Repack, Split60To20Big) {
  BinData a(60, {0xfedcba987654321ull});
  RepackFormat format{RepackEndian::BIG, 20};
  EXPECT_EQ(repackUnit(a.width(), format), 60);
  EXPECT_EQ(repackSize(a.width(), format, 3), 1);
  BinData b = repack(a, format, 0, 3);
  EXPECT_EQ(b.size(), 3);
  EXPECT_EQ(b.width(), 20);
  EXPECT_EQ(b.element64(0), 0xfedcb);
  EXPECT_EQ(b.element64(1), 0xa9876);
  EXPECT_EQ(b.element64(2), 0x54321);
}

TEST(Repack, Split16To8Little) {
  BinData a(16, {0x1234, 0x5678, 0x9abc});
  RepackFormat format{RepackEndian::LITTLE, 8};
  EXPECT_EQ(repackUnit(a.width(), format), 16);
  EXPECT_EQ(repackSize(a.width(), format, 3), 2);
  BinData b = repack(a, format, 1, 3);
  EXPECT_EQ(b.size(), 3);
  EXPECT_EQ(b.width(), 8);
  EXPECT_EQ(b.element64(0), 0x78);
  EXPECT_EQ(b.element64(1), 0x56);
  EXPECT_EQ(b.element64(2), 0xbc);
}

TEST(Repack, Split16To8Big) {
  BinData a(16, {0x1234, 0x5678, 0x9abc});
  RepackFormat format{RepackEndian::BIG, 8};
  EXPECT_EQ(repackUnit(a.width(), format), 16);
  EXPECT_EQ(repackSize(a.width(), format, 3), 2);
  BinData b = repack(a, format, 1, 3);
  EXPECT_EQ(b.size(), 3);
  EXPECT_EQ(b.width(), 8);
  EXPECT_EQ(b.element64(0), 0x56);
  EXPECT_EQ(b.element64(1), 0x78);
  EXPECT_EQ(b.element64(2), 0x9a);
}

TEST(Repack, Padded8to23LeftLittle) {
  BinData a(8, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa});
  RepackFormat format{RepackEndian::LITTLE, 23, 9, 0};
  EXPECT_EQ(repackUnit(a.width(), format), 32);
  EXPECT_EQ(repackSize(a.width(), format, 2), 8);
  BinData b = repack(a, format, 1, 2);
  EXPECT_EQ(b.size(), 2);
  EXPECT_EQ(b.width(), 23);
  EXPECT_EQ(b.element64(0), 0x443322);
  EXPECT_EQ(b.element64(1), 0x087766);
}

TEST(Repack, Padded8to23RightLittle) {
  BinData a(8, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa});
  RepackFormat format{RepackEndian::LITTLE, 23, 0, 9};
  EXPECT_EQ(repackUnit(a.width(), format), 32);
  EXPECT_EQ(repackSize(a.width(), format, 2), 8);
  BinData b = repack(a, format, 1, 2);
  EXPECT_EQ(b.size(), 2);
  EXPECT_EQ(b.width(), 23);
  EXPECT_EQ(b.element64(0), 0x2aa219);
  EXPECT_EQ(b.element64(1), 0x4cc43b);
}

TEST(Repack, Padded8to23MixedLittle) {
  BinData a(8, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa});
  RepackFormat format{RepackEndian::LITTLE, 23, 1, 8};
  EXPECT_EQ(repackUnit(a.width(), format), 32);
  EXPECT_EQ(repackSize(a.width(), format, 2), 8);
  BinData b = repack(a, format, 1, 2);
  EXPECT_EQ(b.size(), 2);
  EXPECT_EQ(b.width(), 23);
  EXPECT_EQ(b.element64(0), 0x554433);
  EXPECT_EQ(b.element64(1), 0x198877);
}

TEST(Repack, Padded8to23LeftBig) {
  BinData a(8, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa});
  RepackFormat format{RepackEndian::BIG, 23, 9, 0};
  EXPECT_EQ(repackUnit(a.width(), format), 32);
  EXPECT_EQ(repackSize(a.width(), format, 2), 8);
  BinData b = repack(a, format, 1, 2);
  EXPECT_EQ(b.size(), 2);
  EXPECT_EQ(b.width(), 23);
  EXPECT_EQ(b.element64(0), 0x334455);
  EXPECT_EQ(b.element64(1), 0x778899);
}

TEST(Repack, Padded8to23RightBig) {
  BinData a(8, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa});
  RepackFormat format{RepackEndian::BIG, 23, 0, 9};
  EXPECT_EQ(repackUnit(a.width(), format), 32);
  EXPECT_EQ(repackSize(a.width(), format, 2), 8);
  BinData b = repack(a, format, 1, 2);
  EXPECT_EQ(b.size(), 2);
  EXPECT_EQ(b.width(), 23);
  EXPECT_EQ(b.element64(0), 0x1119a2);
  EXPECT_EQ(b.element64(1), 0x333bc4);
}

TEST(Repack, Padded8to23MixedBig) {
  BinData a(8, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xaa});
  RepackFormat format{RepackEndian::BIG, 23, 1, 8};
  EXPECT_EQ(repackUnit(a.width(), format), 32);
  EXPECT_EQ(repackSize(a.width(), format, 2), 8);
  BinData b = repack(a, format, 1, 2);
  EXPECT_EQ(b.size(), 2);
  EXPECT_EQ(b.width(), 23);
  EXPECT_EQ(b.element64(0), 0x223344);
  EXPECT_EQ(b.element64(1), 0x667788);
}

}
}
