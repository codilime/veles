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

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "proto/exceptions.h"

using testing::ContainerEq;

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

  std::vector<std::shared_ptr<MsgpackObject>> array_data(
      5, std::make_shared<MsgpackObject>(true));
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

  MsgpackObject sign_pos(INT64_C(42));
  EXPECT_EQ(pos, sign_pos);
  MsgpackObject opposite(uint64_t(-42));
  EXPECT_EQ(opposite.getUnsignedInt(),
            static_cast<uint64_t>(neg.getSignedInt()));
  EXPECT_NE(opposite, neg);

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

  MsgpackObject array(std::vector<std::shared_ptr<MsgpackObject>>(
      5, std::make_shared<MsgpackObject>(true)));
  MsgpackObject array2(std::vector<std::shared_ptr<MsgpackObject>>(
      5, std::make_shared<MsgpackObject>(true)));
  MsgpackObject array3(std::vector<std::shared_ptr<MsgpackObject>>(
      4, std::make_shared<MsgpackObject>(true)));
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

void pack_unpack_check(const MsgpackObject& obj, const std::string& msg) {
  msgpack::sbuffer sbuf;
  msgpack::pack(sbuf, obj);
  msgpack::object_handle oh = msgpack::unpack(sbuf.data(), sbuf.size());
  msgpack::object mo = oh.get();
  auto unp_obj = std::make_shared<MsgpackObject>(mo);
  EXPECT_EQ(obj, *unp_obj) << msg;
}

TEST(MsgpackObject, SimpleMsgpackConversion) {
  MsgpackObject nil;
  pack_unpack_check(nil, "nil");

  MsgpackObject tru(true);
  pack_unpack_check(tru, "bool");

  MsgpackObject pos(UINT64_C(42));
  pack_unpack_check(pos, "pos_int");

  MsgpackObject neg(INT64_C(-42));
  pack_unpack_check(neg, "neg_int");

  MsgpackObject dbl(5.0);
  pack_unpack_check(dbl, "double");

  MsgpackObject str("FOOBAR");
  pack_unpack_check(str, "string");

  MsgpackObject bin(std::vector<uint8_t>(5, 30));
  pack_unpack_check(bin, "bin");

  MsgpackObject array(std::vector<std::shared_ptr<MsgpackObject>>(
      5, std::make_shared<MsgpackObject>(true)));
  pack_unpack_check(array, "array");

  std::map<std::string, std::shared_ptr<MsgpackObject>> map_data;
  map_data["foo"] = std::make_shared<MsgpackObject>("foo");
  map_data["bar"] = std::make_shared<MsgpackObject>("bar");
  MsgpackObject map(map_data);
  pack_unpack_check(map, "map");

  MsgpackObject ext(30, std::vector<uint8_t>(5, 30));
  pack_unpack_check(ext, "ext");
}

TEST(MsgpackObject, TestUnpack) {
  msgpack::sbuffer sbuf;
  msgpack::packer<msgpack::sbuffer> packer(sbuf);
  packer.pack_nil();
  msgpack::object_handle oh = msgpack::unpack(sbuf.data(), sbuf.size());
  msgpack::object mo = oh.get();
  std::shared_ptr<MsgpackObject> unp_obj = std::make_shared<MsgpackObject>(mo);
  EXPECT_EQ(unp_obj->type(), ObjectType::NIL);

  sbuf.clear();
  packer.pack(42);
  oh = msgpack::unpack(sbuf.data(), sbuf.size());
  mo = oh.get();
  unp_obj = std::make_shared<MsgpackObject>(mo);
  EXPECT_EQ(unp_obj->getUnsignedInt(), static_cast<uint64_t>(42));

  sbuf.clear();
  packer.pack(-42);
  oh = msgpack::unpack(sbuf.data(), sbuf.size());
  mo = oh.get();
  unp_obj = std::make_shared<MsgpackObject>(mo);
  EXPECT_EQ(unp_obj->getSignedInt(), -42);

  sbuf.clear();
  packer.pack(true);
  oh = msgpack::unpack(sbuf.data(), sbuf.size());
  mo = oh.get();
  unp_obj = std::make_shared<MsgpackObject>(mo);
  EXPECT_EQ(unp_obj->getBool(), true);

  sbuf.clear();
  packer.pack(5.0);
  oh = msgpack::unpack(sbuf.data(), sbuf.size());
  mo = oh.get();
  unp_obj = std::make_shared<MsgpackObject>(mo);
  EXPECT_DOUBLE_EQ(unp_obj->getDouble(), 5.0);

  sbuf.clear();
  packer.pack("FOOBAR");
  oh = msgpack::unpack(sbuf.data(), sbuf.size());
  mo = oh.get();
  unp_obj = std::make_shared<MsgpackObject>(mo);
  EXPECT_EQ(*unp_obj->getString(), "FOOBAR");

  sbuf.clear();
  packer.pack(std::vector<uint8_t>(5, 30));
  oh = msgpack::unpack(sbuf.data(), sbuf.size());
  mo = oh.get();
  unp_obj = std::make_shared<MsgpackObject>(mo);
  EXPECT_EQ(*unp_obj->getBin(), std::vector<uint8_t>(5, 30));

  sbuf.clear();
  packer.pack(std::vector<uint64_t>(5, 30));
  oh = msgpack::unpack(sbuf.data(), sbuf.size());
  mo = oh.get();
  unp_obj = std::make_shared<MsgpackObject>(mo);
  EXPECT_EQ(unp_obj->getArray()->size(), 5u);
  for (auto i : *unp_obj->getArray()) {
    EXPECT_EQ(i->getUnsignedInt(), 30u);
  }

  sbuf.clear();
  std::map<std::string, std::shared_ptr<MsgpackObject>> map_data;
  map_data["foo"] = std::make_shared<MsgpackObject>("foo");
  map_data["bar"] = std::make_shared<MsgpackObject>("bar");
  packer.pack(map_data);
  oh = msgpack::unpack(sbuf.data(), sbuf.size());
  mo = oh.get();
  unp_obj = std::make_shared<MsgpackObject>(mo);
  EXPECT_EQ(unp_obj->getMap()->size(), 2u);
  EXPECT_EQ(*(*unp_obj->getMap())["foo"]->getString(), "foo");
  EXPECT_EQ(*(*unp_obj->getMap())["bar"]->getString(), "bar");

  sbuf.clear();
  packer.pack_ext(5, 30);
  const char data[] = {0x00, 0x01, 0x02, 0x03, 0x04};
  packer.pack_ext_body(data, 5);
  oh = msgpack::unpack(sbuf.data(), sbuf.size());
  mo = oh.get();
  unp_obj = std::make_shared<MsgpackObject>(mo);
  EXPECT_EQ(unp_obj->getExt().first, 30);
  EXPECT_THAT(
      *unp_obj->getExt().second,
      ContainerEq(std::vector<uint8_t>({0x00, 0x01, 0x02, 0x03, 0x04})));
}

TEST(MsgpackObject, TestPack) {
  msgpack::sbuffer sbuf;
  msgpack::packer<msgpack::sbuffer> packer(sbuf);
  MsgpackObject nil;
  packer.pack(nil);
  msgpack::object_handle oh = msgpack::unpack(sbuf.data(), sbuf.size());
  msgpack::object mo = oh.get();
  EXPECT_TRUE(mo.is_nil());

  sbuf.clear();
  MsgpackObject tru(true);
  packer.pack(tru);
  oh = msgpack::unpack(sbuf.data(), sbuf.size());
  mo = oh.get();
  EXPECT_EQ(mo.as<bool>(), true);

  sbuf.clear();
  MsgpackObject pos(UINT64_C(42));
  packer.pack(pos);
  oh = msgpack::unpack(sbuf.data(), sbuf.size());
  mo = oh.get();
  EXPECT_EQ(mo.as<uint64_t>(), 42u);

  sbuf.clear();
  MsgpackObject neg(INT64_C(-42));
  packer.pack(neg);
  oh = msgpack::unpack(sbuf.data(), sbuf.size());
  mo = oh.get();
  EXPECT_EQ(mo.as<int64_t>(), -42);

  sbuf.clear();
  MsgpackObject dbl(5.0);
  packer.pack(dbl);
  oh = msgpack::unpack(sbuf.data(), sbuf.size());
  mo = oh.get();
  EXPECT_DOUBLE_EQ(mo.as<double>(), 5.0);

  sbuf.clear();
  MsgpackObject str("FOOBAR");
  packer.pack(str);
  oh = msgpack::unpack(sbuf.data(), sbuf.size());
  mo = oh.get();
  EXPECT_EQ(mo.as<std::string>(), "FOOBAR");

  sbuf.clear();
  MsgpackObject bin(std::vector<uint8_t>(5, 30));
  packer.pack(bin);
  oh = msgpack::unpack(sbuf.data(), sbuf.size());
  mo = oh.get();
  EXPECT_THAT(mo.as<std::vector<uint8_t>>(),
              ContainerEq(std::vector<uint8_t>(5, 30)));

  sbuf.clear();
  MsgpackObject array(std::vector<std::shared_ptr<MsgpackObject>>(
      5, std::make_shared<MsgpackObject>(true)));
  packer.pack(array);
  oh = msgpack::unpack(sbuf.data(), sbuf.size());
  mo = oh.get();
  EXPECT_THAT(mo.as<std::vector<bool>>(),
              ContainerEq(std::vector<bool>(5, true)));

  sbuf.clear();
  std::map<std::string, std::shared_ptr<MsgpackObject>> map_data;
  map_data["foo"] = std::make_shared<MsgpackObject>("foo");
  map_data["bar"] = std::make_shared<MsgpackObject>("bar");
  MsgpackObject map(map_data);
  packer.pack(map);
  oh = msgpack::unpack(sbuf.data(), sbuf.size());
  mo = oh.get();
  EXPECT_EQ((mo.as<std::map<std::string, std::string>>().size()), 2u);
  EXPECT_EQ((mo.as<std::map<std::string, std::string>>()["foo"]), "foo");
  EXPECT_EQ((mo.as<std::map<std::string, std::string>>()["bar"]), "bar");

  sbuf.clear();
  MsgpackObject ext(30, std::vector<uint8_t>(5, 30));
  packer.pack(ext);
  oh = msgpack::unpack(sbuf.data(), sbuf.size());
  mo = oh.get();
  EXPECT_EQ(mo.via.ext.type(), 30);
  EXPECT_EQ(mo.via.ext.size, static_cast<uint32_t>(5));
  for (int i = 0; i < 5; ++i) {
    EXPECT_EQ(mo.via.ext.data()[i], 30);
  }
}

}  // namespace messages
}  // namespace veles
