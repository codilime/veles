/*
 * Copyright 2017 CodiLime
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

#include "network/msgpackobject.h"

#include <cstdint>

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "proto/exceptions.h"

using namespace testing;

namespace veles {
namespace messages {

TEST(MsgpackObject, SimpleAccess) {
  MsgpackObject nil;
  EXPECT_EQ(nil.type(), ObjectType::NIL);
  EXPECT_THROW(nil.getBool(), proto::SchemaError);

  MsgpackObject pos_int(UINT64_C(42));
  EXPECT_EQ(pos_int.type(), ObjectType::UNSIGNED_INTEGER);
  EXPECT_EQ(pos_int.getUnsignedInt(), UINT64_C(42));
  EXPECT_EQ(pos_int.getSignedInt(), INT64_C(42));
  EXPECT_THROW(pos_int.getBool(), proto::SchemaError);

  MsgpackObject neg_int(INT64_C(-42));
  EXPECT_EQ(neg_int.type(), ObjectType::SIGNED_INTEGER);
  EXPECT_EQ(neg_int.getSignedInt(), INT64_C(-42));
  EXPECT_THROW(neg_int.getUnsignedInt(), proto::SchemaError);
  EXPECT_THROW(neg_int.getBool(), proto::SchemaError);

  MsgpackObject large_pos_int(UINT64_MAX);
  EXPECT_EQ(large_pos_int.type(), ObjectType::UNSIGNED_INTEGER);
  EXPECT_EQ(large_pos_int.getUnsignedInt(), UINT64_MAX);
  EXPECT_THROW(large_pos_int.getSignedInt(), proto::SchemaError);
  EXPECT_THROW(large_pos_int.getBool(), proto::SchemaError);

  MsgpackObject dbl(3.14);
  EXPECT_EQ(dbl.type(), ObjectType::DOUBLE);
  EXPECT_DOUBLE_EQ(dbl.getDouble(), 3.14);
  EXPECT_THROW(dbl.getBool(), proto::SchemaError);

  MsgpackObject boolean(false);
  EXPECT_EQ(boolean.type(), ObjectType::BOOLEAN);
  EXPECT_EQ(boolean.getBool(), false);
  EXPECT_THROW(boolean.getDouble(), proto::SchemaError);

  MsgpackObject const_char("FOOBAR");
  EXPECT_EQ(const_char.type(), ObjectType::STR);
  EXPECT_EQ(*const_char.getString(), "FOOBAR");
  EXPECT_THROW(const_char.getBool(), proto::SchemaError);

  MsgpackObject str(std::string("FOOBAR"));
  EXPECT_EQ(str.type(), ObjectType::STR);
  EXPECT_EQ(*str.getString(), "FOOBAR");
  EXPECT_THROW(str.getBool(), proto::SchemaError);

  std::vector<uint8_t> bin_data(5, 30);
  MsgpackObject bin(bin_data);
  EXPECT_EQ(bin.type(), ObjectType::BIN);
  EXPECT_THAT(*bin.getBin(), ContainerEq(bin_data));
  bin_data.push_back(29);
  EXPECT_THAT(*bin.getBin(), Not(ContainerEq(bin_data)));
  bin.getBin()->push_back(29);
  EXPECT_THAT(*bin.getBin(), ContainerEq(bin_data));
  EXPECT_THROW(bin.getBool(), proto::SchemaError);

  std::vector<std::shared_ptr<MsgpackObject>> array_data(5, std::make_shared<MsgpackObject>(true));
  MsgpackObject array(array_data);
  EXPECT_EQ(array.type(), ObjectType::ARRAY);
  EXPECT_THAT(*array.getArray(), ContainerEq(array_data));
  array_data.pop_back();
  EXPECT_THAT(*array.getArray(), Not(ContainerEq(array_data)));
  array.getArray()->pop_back();
  EXPECT_THAT(*array.getArray(), ContainerEq(array_data));
  EXPECT_THROW(array.getBool(), proto::SchemaError);

  std::map<std::string, std::shared_ptr<MsgpackObject>> map_data;
  map_data["foo"] = std::make_shared<MsgpackObject>("foo");
  map_data["bar"] = std::make_shared<MsgpackObject>("bar");
  MsgpackObject map(map_data);
  EXPECT_EQ(map.type(), ObjectType::MAP);
  EXPECT_THAT(*map.getMap(), ContainerEq(map_data));
  map_data["foo"] = std::make_shared<MsgpackObject>("bar");
  EXPECT_THAT(*map.getMap(), Not(ContainerEq(map_data)));
  EXPECT_THROW(map.getBool(), proto::SchemaError);

  MsgpackObject ext(30, bin_data);
  EXPECT_EQ(ext.type(), ObjectType::EXT);
  EXPECT_EQ(ext.getExt().first, 30);
  EXPECT_THAT(*ext.getExt().second, ContainerEq(bin_data));
  bin_data.pop_back();
  EXPECT_THAT(*ext.getExt().second, Not(ContainerEq(bin_data)));
  ext.getExt().second->pop_back();
  EXPECT_THAT(*ext.getExt().second, ContainerEq(bin_data));
}

TEST(MsgpackObject, TestComparison) {
  MsgpackObject nil;
  MsgpackObject nil2;
  EXPECT_EQ(nil, nil2);

  MsgpackObject tru(true);
  MsgpackObject tru2(true);
  MsgpackObject fals(false);
  EXPECT_EQ(tru, tru2);
  EXPECT_NE(tru, fals);
  EXPECT_NE(tru, nil);

  MsgpackObject pos(UINT64_C(42));
  MsgpackObject pos2(UINT64_C(42));
  MsgpackObject pos3(UINT64_C(24));
  EXPECT_EQ(pos, pos2);
  EXPECT_NE(pos, pos3);
  EXPECT_NE(pos, nil);

  MsgpackObject neg(INT64_C(-42));
  MsgpackObject neg2(INT64_C(-42));
  MsgpackObject neg3(INT64_C(-24));
  EXPECT_EQ(neg, neg2);
  EXPECT_NE(neg, neg3);
  EXPECT_NE(neg, nil);

  MsgpackObject dbl(5.0);
  MsgpackObject dbl2(5.0);
  MsgpackObject dbl3(5.1);
  EXPECT_EQ(dbl, dbl2);
  EXPECT_NE(dbl, dbl3);
  EXPECT_NE(dbl, nil);

  MsgpackObject str("FOOBAR");
  MsgpackObject str2(std::string("FOOBAR"));
  MsgpackObject str3("BARFOO");
  EXPECT_EQ(str, str2);
  EXPECT_NE(str, str3);
  EXPECT_NE(str, nil);

  MsgpackObject bin(std::vector<uint8_t>(5, 30));
  MsgpackObject bin2(std::vector<uint8_t>(5, 30));
  MsgpackObject bin3(std::vector<uint8_t>(6, 30));
  EXPECT_EQ(bin, bin2);
  EXPECT_NE(bin, bin3);
  EXPECT_NE(bin, nil);
  bin3.getBin()->pop_back();
  EXPECT_EQ(bin, bin3);

  MsgpackObject array(std::vector<std::shared_ptr<MsgpackObject>>(5, std::make_shared<MsgpackObject>(true)));
  MsgpackObject array2(std::vector<std::shared_ptr<MsgpackObject>>(5, std::make_shared<MsgpackObject>(true)));
  MsgpackObject array3(std::vector<std::shared_ptr<MsgpackObject>>(4, std::make_shared<MsgpackObject>(true)));
  EXPECT_EQ(array, array2);
  EXPECT_NE(array, array3);
  EXPECT_NE(array, nil);
  array3.getArray()->push_back(std::make_shared<MsgpackObject>(true));
  EXPECT_EQ(array, array3);

  std::map<std::string, std::shared_ptr<MsgpackObject>> map_data;
  map_data["foo"] = std::make_shared<MsgpackObject>("foo");
  map_data["bar"] = std::make_shared<MsgpackObject>("bar");
  MsgpackObject map(map_data);
  MsgpackObject map2(map_data);
  map_data["bar2"] = std::make_shared<MsgpackObject>("bar");
  MsgpackObject map3(map_data);
  EXPECT_EQ(map, map2);
  EXPECT_NE(map, map3);
  EXPECT_NE(map, nil);
  map3.getMap()->erase("bar2");
  EXPECT_EQ(map, map3);

  MsgpackObject ext(30, std::vector<uint8_t>(5, 30));
  MsgpackObject ext2(30, std::vector<uint8_t>(5, 30));
  MsgpackObject ext3(30, std::vector<uint8_t>(6, 30));
  EXPECT_EQ(ext, ext2);
  EXPECT_NE(ext, ext3);
  EXPECT_NE(ext, nil);
  ext3.getExt().second->pop_back();
  EXPECT_EQ(ext, ext3);
}

void pack_unpack_check(MsgpackObject& obj) {
  msgpack::sbuffer sbuf;
  msgpack::pack(sbuf, obj);
  msgpack::object mo = msgpack::unpack(sbuf.data(), sbuf.size()).get();
  auto unp_obj = std::make_shared<MsgpackObject>(obj);
  EXPECT_EQ(obj, *unp_obj);
}

TEST(MsgpackObject, SimpleMsgpackConversion) {
  MsgpackObject nil;
  pack_unpack_check(nil);

  MsgpackObject tru(true);
  pack_unpack_check(tru);

  MsgpackObject pos(UINT64_C(42));
  pack_unpack_check(pos);

  MsgpackObject neg(INT64_C(-42));
  pack_unpack_check(neg);

  MsgpackObject dbl(5.0);
  pack_unpack_check(dbl);

  MsgpackObject str("FOOBAR");
  pack_unpack_check(str);

  MsgpackObject bin(std::vector<uint8_t>(5, 30));
  pack_unpack_check(bin);

  MsgpackObject array(std::vector<std::shared_ptr<MsgpackObject>>(5, std::make_shared<MsgpackObject>(true)));
  pack_unpack_check(array);

  std::map<std::string, std::shared_ptr<MsgpackObject>> map_data;
  map_data["foo"] = std::make_shared<MsgpackObject>("foo");
  map_data["bar"] = std::make_shared<MsgpackObject>("bar");
  MsgpackObject map(map_data);
  pack_unpack_check(map);

  MsgpackObject ext(30, std::vector<uint8_t>(5, 30));
  pack_unpack_check(ext);
}

}  // namespace messages
}  // namespace veles
