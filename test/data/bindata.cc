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

#include <algorithm>

#include "gtest/gtest.h"

namespace veles {
namespace data {

TEST(BinData, SimpleInline8) {
  uint8_t d[1] = {0x12};
  BinData a(8, 1, d);
  EXPECT_EQ(a.rawData()[0], 0x12);
  EXPECT_EQ(a.rawData(0)[0], 0x12);
  EXPECT_EQ(a.width(), 8u);
  EXPECT_EQ(a.size(), 1u);
  EXPECT_EQ(a.octetsPerElement(), 1u);
  EXPECT_EQ(a.octets(), 1u);
}

TEST(BinData, SimpleLong8) {
  uint8_t d[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  BinData a(8, 10, d);
  for (int i = 0; i < 10; i++) {
    EXPECT_EQ(a.rawData()[i], i + 1);
  }
  for (int i = 0; i < 10; i++) {
    EXPECT_EQ(a.rawData(i)[0], i + 1);
  }
  EXPECT_EQ(a.width(), 8u);
  EXPECT_EQ(a.size(), 10u);
  EXPECT_EQ(a.octetsPerElement(), 1u);
  EXPECT_EQ(a.octets(), 10u);
}

TEST(BinData, SimpleInit) {
  BinData a(8, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10});
  for (int i = 0; i < 10; i++) {
    EXPECT_EQ(a.rawData()[i], i + 1);
  }
  for (int i = 0; i < 10; i++) {
    EXPECT_EQ(a.rawData(i)[0], i + 1);
  }
  EXPECT_EQ(a.width(), 8u);
  EXPECT_EQ(a.size(), 10u);
  EXPECT_EQ(a.octetsPerElement(), 1u);
  EXPECT_EQ(a.octets(), 10u);
}

TEST(BinData, SimpleInline16) {
  uint8_t d[2] = {1, 2};
  BinData a(16, 1, d);
  EXPECT_EQ(a.rawData()[0], 1);
  EXPECT_EQ(a.rawData()[1], 2);
  EXPECT_EQ(a.width(), 16u);
  EXPECT_EQ(a.size(), 1u);
  EXPECT_EQ(a.octetsPerElement(), 2u);
  EXPECT_EQ(a.octets(), 2u);
}

TEST(BinData, SimpleVec16) {
  uint8_t d[4] = {1, 2, 3, 4};
  BinData a(16, 2, d);
  EXPECT_EQ(a.rawData()[0], 1);
  EXPECT_EQ(a.rawData()[1], 2);
  EXPECT_EQ(a.rawData()[2], 3);
  EXPECT_EQ(a.rawData()[3], 4);
  EXPECT_EQ(a.rawData(0)[0], 1);
  EXPECT_EQ(a.rawData(0)[1], 2);
  EXPECT_EQ(a.rawData(1)[0], 3);
  EXPECT_EQ(a.rawData(1)[1], 4);
  EXPECT_EQ(a.width(), 16u);
  EXPECT_EQ(a.size(), 2u);
  EXPECT_EQ(a.octetsPerElement(), 2u);
  EXPECT_EQ(a.octets(), 4u);
}

TEST(BinData, SimpleVec15) {
  uint8_t d[4] = {1, 2, 3, 4};
  BinData a(15, 2, d);
  EXPECT_EQ(a.rawData()[0], 1);
  EXPECT_EQ(a.rawData()[1], 2);
  EXPECT_EQ(a.rawData()[2], 3);
  EXPECT_EQ(a.rawData()[3], 4);
  EXPECT_EQ(a.rawData(0)[0], 1);
  EXPECT_EQ(a.rawData(0)[1], 2);
  EXPECT_EQ(a.rawData(1)[0], 3);
  EXPECT_EQ(a.rawData(1)[1], 4);
  EXPECT_EQ(a.width(), 15u);
  EXPECT_EQ(a.size(), 2u);
  EXPECT_EQ(a.octetsPerElement(), 2u);
  EXPECT_EQ(a.octets(), 4u);
}

TEST(BinData, SimpleVec19) {
  uint8_t d[6] = {1, 2, 3, 4, 5, 6};
  BinData a(19, 2, d);
  EXPECT_EQ(a.rawData()[0], 1);
  EXPECT_EQ(a.rawData()[1], 2);
  EXPECT_EQ(a.rawData()[2], 3);
  EXPECT_EQ(a.rawData()[3], 4);
  EXPECT_EQ(a.rawData()[4], 5);
  EXPECT_EQ(a.rawData()[5], 6);
  EXPECT_EQ(a.rawData(0)[0], 1);
  EXPECT_EQ(a.rawData(0)[1], 2);
  EXPECT_EQ(a.rawData(0)[2], 3);
  EXPECT_EQ(a.rawData(1)[0], 4);
  EXPECT_EQ(a.rawData(1)[1], 5);
  EXPECT_EQ(a.rawData(1)[2], 6);
  EXPECT_EQ(a.width(), 19u);
  EXPECT_EQ(a.size(), 2u);
  EXPECT_EQ(a.octetsPerElement(), 3u);
  EXPECT_EQ(a.octets(), 6u);
}

TEST(BinData, Simple128) {
  uint8_t d[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
  BinData a(128, 1, d);
  for (int i = 0; i < 16; i++) {
    EXPECT_EQ(a.rawData()[i], i + 1);
  }
  EXPECT_EQ(a.width(), 128u);
  EXPECT_EQ(a.size(), 1u);
  EXPECT_EQ(a.octetsPerElement(), 16u);
  EXPECT_EQ(a.octets(), 16u);
}

TEST(BinData, SimpleZero13) {
  BinData a(13, 1);
  EXPECT_EQ(a.rawData()[0], 0);
  EXPECT_EQ(a.rawData()[1], 0);
  EXPECT_EQ(a.width(), 13u);
  EXPECT_EQ(a.size(), 1u);
  EXPECT_EQ(a.octetsPerElement(), 2u);
  EXPECT_EQ(a.octets(), 2u);
}

TEST(BinData, SimpleZero129) {
  BinData a(129, 1);
  for (int i = 0; i < 17; i++) {
    EXPECT_EQ(a.rawData()[i], 0);
  }
  EXPECT_EQ(a.width(), 129u);
  EXPECT_EQ(a.size(), 1u);
  EXPECT_EQ(a.octetsPerElement(), 17u);
  EXPECT_EQ(a.octets(), 17u);
}

TEST(BinData, CopyInline) {
  uint8_t d[] = {3, 2, 1};
  BinData a(17, 1, d);
  BinData b(a);
  EXPECT_EQ(a.width(), 17u);
  EXPECT_EQ(a.size(), 1u);
  EXPECT_EQ(a.octetsPerElement(), 3u);
  EXPECT_EQ(a.octets(), 3u);
  EXPECT_EQ(a.rawData()[0], 3);
  EXPECT_EQ(a.rawData()[1], 2);
  EXPECT_EQ(a.rawData()[2], 1);
  EXPECT_EQ(b.width(), 17u);
  EXPECT_EQ(b.size(), 1u);
  EXPECT_EQ(b.octetsPerElement(), 3u);
  EXPECT_EQ(b.octets(), 3u);
  EXPECT_EQ(b.rawData()[0], 3);
  EXPECT_EQ(b.rawData()[1], 2);
  EXPECT_EQ(b.rawData()[2], 1);
  a.rawData()[0] = 4;
  EXPECT_EQ(a.rawData()[0], 4);
  EXPECT_EQ(b.rawData()[0], 3);
}

TEST(BinData, CopyNonInline) {
  uint8_t d[] = {3, 2, 1};
  BinData a(3, 3, d);
  BinData b(a);
  EXPECT_EQ(a.width(), 3u);
  EXPECT_EQ(a.size(), 3u);
  EXPECT_EQ(a.octetsPerElement(), 1u);
  EXPECT_EQ(a.octets(), 3u);
  EXPECT_EQ(a.rawData()[0], 3);
  EXPECT_EQ(a.rawData()[1], 2);
  EXPECT_EQ(a.rawData()[2], 1);
  EXPECT_EQ(b.width(), 3u);
  EXPECT_EQ(b.size(), 3u);
  EXPECT_EQ(b.octetsPerElement(), 1u);
  EXPECT_EQ(b.octets(), 3u);
  EXPECT_EQ(b.rawData()[0], 3);
  EXPECT_EQ(b.rawData()[1], 2);
  EXPECT_EQ(b.rawData()[2], 1);
  a.rawData()[0] = 4;
  EXPECT_EQ(a.rawData()[0], 4);
  EXPECT_EQ(b.rawData()[0], 3);
}

TEST(BinData, SwapInline) {
  uint8_t d[] = {3, 2, 1};
  uint8_t e[] = {6, 5, 4};
  BinData a(17, 1, d);
  BinData b(19, 1, e);
  std::swap(a, b);
  EXPECT_EQ(a.width(), 19u);
  EXPECT_EQ(a.size(), 1u);
  EXPECT_EQ(a.octetsPerElement(), 3u);
  EXPECT_EQ(a.octets(), 3u);
  EXPECT_EQ(a.rawData()[0], 6);
  EXPECT_EQ(a.rawData()[1], 5);
  EXPECT_EQ(a.rawData()[2], 4);
  EXPECT_EQ(b.width(), 17u);
  EXPECT_EQ(b.size(), 1u);
  EXPECT_EQ(b.octetsPerElement(), 3u);
  EXPECT_EQ(b.octets(), 3u);
  EXPECT_EQ(b.rawData()[0], 3);
  EXPECT_EQ(b.rawData()[1], 2);
  EXPECT_EQ(b.rawData()[2], 1);
}

TEST(BinData, SwapNonInline) {
  uint8_t d[] = {3, 2, 1};
  uint8_t e[] = {6, 5, 4, 3};
  BinData a(2, 3, d);
  BinData b(3, 4, e);
  uint8_t* p1 = a.rawData();
  uint8_t* p2 = b.rawData();
  std::swap(a, b);
  EXPECT_EQ(p1, b.rawData());
  EXPECT_EQ(p2, a.rawData());
  EXPECT_EQ(a.width(), 3u);
  EXPECT_EQ(a.size(), 4u);
  EXPECT_EQ(a.octetsPerElement(), 1u);
  EXPECT_EQ(a.octets(), 4u);
  EXPECT_EQ(a.rawData()[0], 6);
  EXPECT_EQ(a.rawData()[1], 5);
  EXPECT_EQ(a.rawData()[2], 4);
  EXPECT_EQ(a.rawData()[3], 3);
  EXPECT_EQ(b.width(), 2u);
  EXPECT_EQ(b.size(), 3u);
  EXPECT_EQ(b.octetsPerElement(), 1u);
  EXPECT_EQ(b.octets(), 3u);
  EXPECT_EQ(b.rawData()[0], 3);
  EXPECT_EQ(b.rawData()[1], 2);
  EXPECT_EQ(b.rawData()[2], 1);
}

TEST(BinData, SwapMixed) {
  uint8_t d[] = {3, 2, 1};
  uint8_t e[] = {6, 5, 4, 3};
  BinData a(17, 1, d);
  BinData b(3, 4, e);
  uint8_t* p1 = b.rawData();
  std::swap(a, b);
  EXPECT_EQ(p1, a.rawData());
  EXPECT_EQ(a.width(), 3u);
  EXPECT_EQ(a.size(), 4u);
  EXPECT_EQ(a.octetsPerElement(), 1u);
  EXPECT_EQ(a.octets(), 4u);
  EXPECT_EQ(a.rawData()[0], 6);
  EXPECT_EQ(a.rawData()[1], 5);
  EXPECT_EQ(a.rawData()[2], 4);
  EXPECT_EQ(a.rawData()[3], 3);
  EXPECT_EQ(b.width(), 17u);
  EXPECT_EQ(b.size(), 1u);
  EXPECT_EQ(b.octetsPerElement(), 3u);
  EXPECT_EQ(b.octets(), 3u);
  EXPECT_EQ(b.rawData()[0], 3);
  EXPECT_EQ(b.rawData()[1], 2);
  EXPECT_EQ(b.rawData()[2], 1);
}

TEST(BinData, AssignInline) {
  uint8_t d[] = {3, 2, 1};
  uint8_t e[] = {6, 5, 4, 3};
  BinData a(17, 1, d);
  BinData b(28, 1, e);
  EXPECT_EQ(b.width(), 28u);
  EXPECT_EQ(b.size(), 1u);
  EXPECT_EQ(b.octetsPerElement(), 4u);
  EXPECT_EQ(b.octets(), 4u);
  EXPECT_EQ(b.rawData()[0], 6);
  EXPECT_EQ(b.rawData()[1], 5);
  EXPECT_EQ(b.rawData()[2], 4);
  EXPECT_EQ(b.rawData()[3], 3);
  b = a;
  EXPECT_EQ(b.width(), 17u);
  EXPECT_EQ(b.size(), 1u);
  EXPECT_EQ(b.octetsPerElement(), 3u);
  EXPECT_EQ(b.octets(), 3u);
  EXPECT_EQ(b.rawData()[0], 3);
  EXPECT_EQ(b.rawData()[1], 2);
  EXPECT_EQ(b.rawData()[2], 1);
}

TEST(BinData, AssignNonInline) {
  uint8_t d[] = {3, 2, 1};
  uint8_t e[] = {6, 5, 4, 3};
  BinData a(7, 3, d);
  BinData b(7, 4, e);
  EXPECT_EQ(b.width(), 7u);
  EXPECT_EQ(b.size(), 4u);
  EXPECT_EQ(b.octetsPerElement(), 1u);
  EXPECT_EQ(b.octets(), 4u);
  EXPECT_EQ(b.rawData()[0], 6);
  EXPECT_EQ(b.rawData()[1], 5);
  EXPECT_EQ(b.rawData()[2], 4);
  EXPECT_EQ(b.rawData()[3], 3);
  b = a;
  EXPECT_EQ(b.width(), 7u);
  EXPECT_EQ(b.size(), 3u);
  EXPECT_EQ(b.octetsPerElement(), 1u);
  EXPECT_EQ(b.octets(), 3u);
  EXPECT_EQ(b.rawData()[0], 3);
  EXPECT_EQ(b.rawData()[1], 2);
  EXPECT_EQ(b.rawData()[2], 1);
}

TEST(BinData, Data7) {
  BinData a(7, {1, 2, 3, 4, 5, 6});
  BinData b = a.data(2, 5);
  BinData c = a[5];
  EXPECT_EQ(b.width(), 7u);
  EXPECT_EQ(b.size(), 3u);
  EXPECT_EQ(b.octets(), 3u);
  EXPECT_EQ(b.rawData()[0], 3);
  EXPECT_EQ(b.rawData()[1], 4);
  EXPECT_EQ(b.rawData()[2], 5);
  EXPECT_EQ(c.width(), 7u);
  EXPECT_EQ(c.size(), 1u);
  EXPECT_EQ(c.octets(), 1u);
  EXPECT_EQ(c.rawData()[0], 6);
}

TEST(BinData, Data23) {
  BinData a = BinData::fromRawData(
      23, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15});
  BinData b = a.data(2, 4);
  BinData c = a[1];
  EXPECT_EQ(b.width(), 23u);
  EXPECT_EQ(b.size(), 2u);
  EXPECT_EQ(b.octets(), 6u);
  EXPECT_EQ(b.rawData()[0], 7);
  EXPECT_EQ(b.rawData()[1], 8);
  EXPECT_EQ(b.rawData()[2], 9);
  EXPECT_EQ(b.rawData()[3], 10);
  EXPECT_EQ(b.rawData()[4], 11);
  EXPECT_EQ(b.rawData()[5], 12);
  EXPECT_EQ(c.width(), 23u);
  EXPECT_EQ(c.size(), 1u);
  EXPECT_EQ(c.octets(), 3u);
  EXPECT_EQ(c.rawData()[0], 4);
  EXPECT_EQ(c.rawData()[1], 5);
  EXPECT_EQ(c.rawData()[2], 6);
}

TEST(BinData, SetData23) {
  BinData a = BinData::fromRawData(
      23, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15});
  BinData b = BinData::fromRawData(23, {101, 102, 103, 104, 105, 106});
  a.setData(2, 4, b);
  EXPECT_EQ(a.rawData()[5], 6);
  EXPECT_EQ(a.rawData()[6], 101);
  EXPECT_EQ(a.rawData()[7], 102);
  EXPECT_EQ(a.rawData()[8], 103);
  EXPECT_EQ(a.rawData()[9], 104);
  EXPECT_EQ(a.rawData()[10], 105);
  EXPECT_EQ(a.rawData()[11], 106);
  EXPECT_EQ(a.rawData()[12], 13);
}

TEST(BinData, Bits64) {
  BinData a = BinData::fromRawData(23, {1, 2, 3, 4, 5, 6});
  EXPECT_EQ(a.bits64(1, 0, 23), 0x060504u);
  EXPECT_EQ(a.bits64(1, 0, 16), 0x0504u);
  EXPECT_EQ(a.bits64(1, 8, 15), 0x0605u);
  BinData b =
      BinData::fromRawData(120,
                           {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
                            0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff});
  EXPECT_EQ(b.bits64(0, 0, 20), 0x32211u);
  EXPECT_EQ(b.bits64(0, 0, 64), 0x8877665544332211ull);
  EXPECT_EQ(b.bits64(0, 56, 64), 0xffeeddccbbaa9988ull);
  EXPECT_EQ(b.bits64(0, 64, 56), 0xffeeddccbbaa99ull);
  EXPECT_EQ(b.bits64(0, 60, 60), 0xffeeddccbbaa998ull);
}

TEST(BinData, SetBits64) {
  BinData a = BinData::fromRawData(23, {1, 2, 3, 4, 5, 6});
  a.setBits64(0, 0, 23, 0x789abc);
  EXPECT_EQ(a.bits64(0, 0, 23), 0x789abcu);
  a.setBits64(1, 0, 17, 0x11111);
  EXPECT_EQ(a.bits64(1, 0, 23), 0x71111u);
  BinData b =
      BinData::fromRawData(120,
                           {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
                            0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff});
  b.setBits64(0, 30, 60, 0x123456789abcdefull);
  EXPECT_EQ(b.bits64(0, 0, 20), 0x32211u);
  EXPECT_EQ(b.bits64(0, 0, 64), 0xe26af37bc4332211ull);
  EXPECT_EQ(b.bits64(0, 56, 64), 0xffeeddcc48d159e2ull);
  EXPECT_EQ(b.bits64(0, 64, 56), 0xffeeddcc48d159ull);
}

TEST(BinData, Element64) {
  BinData a = BinData::fromRawData(23, {1, 2, 3, 4, 5, 6});
  EXPECT_EQ(a.element64(1), 0x060504u);
  EXPECT_EQ(a.element64(0), 0x030201u);
  BinData b = BinData::fromRawData(23, {0x11, 0x22, 0x33});
  EXPECT_EQ(b.element64(), 0x332211u);
}

TEST(BinData, SetElement64) {
  BinData a = BinData::fromRawData(23, {1, 2, 3, 4, 5, 6});
  a.setElement64(0, 0x789abcu);
  EXPECT_EQ(a.bits64(0, 0, 23), 0x789abcu);
  BinData b = BinData::fromRawData(23, {0x11, 0x22, 0x33});
  b.setElement64(0xabcd);
  EXPECT_EQ(b.element64(), 0xabcdu);
}

TEST(BinData, Bits) {
  BinData a = BinData::fromRawData(23, {1, 2, 3, 4, 5, 6});
  BinData a1 = a.bits(1, 0, 23);
  BinData a2 = a.bits(1, 0, 16);
  BinData a3 = a.bits(1, 8, 15);
  EXPECT_EQ(a1.size(), 1u);
  EXPECT_EQ(a2.size(), 1u);
  EXPECT_EQ(a3.size(), 1u);
  EXPECT_EQ(a1.width(), 23u);
  EXPECT_EQ(a2.width(), 16u);
  EXPECT_EQ(a3.width(), 15u);
  EXPECT_EQ(a1.bits64(0, 0, 23), 0x060504u);
  EXPECT_EQ(a2.bits64(0, 0, 16), 0x0504u);
  EXPECT_EQ(a3.bits64(0, 0, 15), 0x0605u);
  BinData b =
      BinData::fromRawData(120,
                           {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
                            0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff});
  BinData b1 = b.bits(0, 20, 80);
  EXPECT_EQ(b1.size(), 1u);
  EXPECT_EQ(b1.width(), 80u);
  EXPECT_EQ(b1.bits64(0, 0, 64), 0xbaa9988776655443ull);
  EXPECT_EQ(b1.bits64(0, 16, 64), 0xdccbbaa998877665ull);
}

TEST(BinData, SetBits) {
  BinData a = BinData::fromRawData(23, {1, 2, 3, 4, 5, 6});
  a.setBits(0, 0, 23, BinData(23, {0x789abc}));
  EXPECT_EQ(a.bits64(0, 0, 23), 0x789abcu);
  a.setBits(1, 0, 17, BinData(17, {0x11111}));
  EXPECT_EQ(a.bits64(1, 0, 23), 0x71111u);
  BinData b =
      BinData::fromRawData(120,
                           {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88,
                            0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff});
  b.setBits(0, 20, 80, BinData::fromRawData(80,
                                            {0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5,
                                             0xa6, 0xa7, 0xa8, 0xa9}));
  EXPECT_EQ(b.bits64(0, 0, 20), 0x32211u);
  EXPECT_EQ(b.bits64(0, 0, 64), 0x5a4a3a2a1a032211ull);
  EXPECT_EQ(b.bits64(0, 56, 64), 0xffeeda9a8a7a6a5aull);
  EXPECT_EQ(b.bits64(0, 64, 56), 0xffeeda9a8a7a6aull);
}

TEST(BinData, OperatorPlus) {
  BinData a = BinData::fromRawData(8, {1});
  BinData b = BinData::fromRawData(8, {2, 3});
  BinData res = a + b;
  EXPECT_EQ(res.width(), 8u);
  EXPECT_EQ(res.size(), 3u);
  EXPECT_EQ(res.element64(), 0x01u);
  EXPECT_EQ(res.element64(1), 0x02u);
  EXPECT_EQ(res.element64(2), 0x03u);
}

TEST(BinData, ToString) {
  EXPECT_EQ(BinData::fromRawData(8, {1}).toString(), "0x01");
  EXPECT_EQ(BinData::fromRawData(9, {1, 0}).toString(), "0x001");
  EXPECT_EQ(BinData::fromRawData(8, {1, 2}).toString(), "0x01, 0x02");
  EXPECT_EQ(BinData::fromRawData(8, {1, 2, 3}).toString(2), "0x01, 0x02, ...");
  EXPECT_EQ(BinData::fromRawData(72, {1, 2, 3, 4, 5, 6, 7, 8, 9}).toString(2),
            "0x090807060504030201");
}

TEST(BinData, Equal) {
  EXPECT_TRUE(BinData::fromRawData(8, {1}) == BinData::fromRawData(8, {1}));
  EXPECT_TRUE(BinData::fromRawData(9, {1, 2, 3, 4}) ==
              BinData::fromRawData(9, {1, 2, 3, 4}));
  EXPECT_FALSE(BinData::fromRawData(8, {1}) == BinData::fromRawData(8, {1, 2}));
  EXPECT_FALSE(BinData::fromRawData(8, {1}) == BinData::fromRawData(7, {1}));
}

}  // namespace data
}  // namespace veles
