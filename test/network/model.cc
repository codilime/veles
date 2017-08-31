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

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "models.h"
#include "network/msgpackobject.h"
#include "network/msgpackwrapper.h"

using testing::ContainerEq;

using veles::tests::schema::Any;
using veles::tests::schema::AnyOptional;
using veles::tests::schema::SmallInteger;
using veles::tests::schema::SmallIntegerOptional;
using veles::tests::schema::SmallUnsignedInteger;
using veles::tests::schema::SmallUnsignedIntegerOptional;
using veles::tests::schema::Boolean;
using veles::tests::schema::BooleanOptional;
using veles::tests::schema::Float;
using veles::tests::schema::FloatOptional;
using veles::tests::schema::String;
using veles::tests::schema::StringOptional;
using veles::tests::schema::Binary;
using veles::tests::schema::BinaryOptional;
using veles::tests::schema::NodeIDModel;
using veles::tests::schema::NodeIDModelOptional;
using veles::tests::schema::BinDataModel;
using veles::tests::schema::BinDataModelOptional;
using veles::tests::schema::List;
using veles::tests::schema::ListOptional;
using veles::tests::schema::Set;
using veles::tests::schema::SetOptional;
using veles::tests::schema::Map;
using veles::tests::schema::MapOptional;
using veles::tests::schema::Object;
using veles::tests::schema::ObjectOptional;
using veles::tests::schema::Enum;
using veles::tests::schema::EnumOptional;
using veles::tests::schema::SubType1;
using veles::tests::schema::SubType2;
using veles::tests::schema::TestEnum;
using veles::tests::schema::BaseModel;

namespace veles {
namespace messages {

TEST(TestModel, TestCommon) {
  msgpack::sbuffer sbuf;
  msgpack::pack(sbuf, 42);
  msgpack::object_handle oh = msgpack::unpack(sbuf.data(), sbuf.size());
  msgpack::object mo = oh.get();
  EXPECT_THROW(Any::loadMessagePack(mo), proto::SchemaError);

  sbuf.clear();
  std::map<std::string, std::string> map_data;
  map_data["b"] = "bar";
  msgpack::pack(sbuf, map_data);
  oh = msgpack::unpack(sbuf.data(), sbuf.size());
  mo = oh.get();
  EXPECT_THROW(Any::loadMessagePack(mo), proto::SchemaError);

  sbuf.clear();
  map_data.clear();
  map_data["a"] = "proper";
  msgpack::pack(sbuf, map_data);
  oh = msgpack::unpack(sbuf.data(), sbuf.size());
  mo = oh.get();
  auto recv = Any::loadMessagePack(mo);
  EXPECT_EQ(*recv->a->getString(), "proper");

  sbuf.clear();
  msgpack::packer<msgpack::sbuffer> packer(sbuf);
  auto obj = std::make_shared<Any>(std::make_shared<MsgpackObject>("asdf"));
  MsgpackWrapper::dumpObject(packer, obj);
  EXPECT_EQ(sbuf.size(), static_cast<size_t>(8));
  const uint8_t data[] = {0x81, 0xa1, 0x61, 0xa4, 0x61, 0x73, 0x64, 0x66};
  EXPECT_EQ(memcmp(sbuf.data(), data, 8), 0);

  obj->a = nullptr;
  EXPECT_THROW(MsgpackWrapper::dumpObject(packer, obj), proto::SchemaError);
}

std::shared_ptr<MsgpackObject> pack(std::shared_ptr<MsgpackObject> obj) {
  std::map<std::string, std::shared_ptr<MsgpackObject>> map_data;
  map_data["a"] = obj;
  return std::make_shared<MsgpackObject>(map_data);
}

TEST(TestModel, TestAny) {
  auto obj = pack(std::make_shared<MsgpackObject>(false));
  std::shared_ptr<Any> ptr;
  std::shared_ptr<AnyOptional> ptr2;
  fromMsgpackObject(obj, &ptr);
  fromMsgpackObject(obj, &ptr2);
  EXPECT_EQ(ptr->a->getBool(), false);
  EXPECT_EQ(ptr2->a.first, true);
  EXPECT_EQ(ptr2->a.second->getBool(), false);

  obj = pack(std::make_shared<MsgpackObject>());
  EXPECT_THROW(fromMsgpackObject(obj, &ptr), proto::SchemaError);
  fromMsgpackObject(obj, &ptr2);
  EXPECT_EQ(ptr2->a.first, false);
}

// TODO(mwk): Test integers that use bignums - currently it would be identical
// with small integers.

TEST(TestModel, TestSmallInteger) {
  auto obj = pack(std::make_shared<MsgpackObject>(INT64_C(-30)));
  std::shared_ptr<SmallInteger> ptr;
  std::shared_ptr<SmallIntegerOptional> ptr2;
  fromMsgpackObject(obj, &ptr);
  fromMsgpackObject(obj, &ptr2);
  EXPECT_EQ(ptr->a, INT64_C(-30));
  EXPECT_EQ(ptr2->a.first, true);
  EXPECT_EQ(ptr2->a.second, INT64_C(-30));

  obj = pack(std::make_shared<MsgpackObject>());
  EXPECT_THROW(fromMsgpackObject(obj, &ptr), proto::SchemaError);
  fromMsgpackObject(obj, &ptr2);
  EXPECT_EQ(ptr2->a.first, false);

  obj = pack(std::make_shared<MsgpackObject>(false));
  EXPECT_THROW(fromMsgpackObject(obj, &ptr), proto::SchemaError);
  EXPECT_THROW(fromMsgpackObject(obj, &ptr2), proto::SchemaError);

  SmallInteger::Builder b;
  ptr = b.build();
  EXPECT_EQ(ptr->a, INT64_C(-42));
}

TEST(TestModel, TestSmallUnsignedInteger) {
  auto obj = pack(std::make_shared<MsgpackObject>(UINT64_C(30)));
  std::shared_ptr<SmallUnsignedInteger> ptr;
  std::shared_ptr<SmallUnsignedIntegerOptional> ptr2;
  fromMsgpackObject(obj, &ptr);
  fromMsgpackObject(obj, &ptr2);
  EXPECT_EQ(ptr->a, UINT64_C(30));
  EXPECT_EQ(ptr2->a.first, true);
  EXPECT_EQ(ptr2->a.second, UINT64_C(30));

  obj = pack(std::make_shared<MsgpackObject>());
  EXPECT_THROW(fromMsgpackObject(obj, &ptr), proto::SchemaError);
  fromMsgpackObject(obj, &ptr2);
  EXPECT_EQ(ptr2->a.first, false);

  obj = pack(std::make_shared<MsgpackObject>(false));
  EXPECT_THROW(fromMsgpackObject(obj, &ptr), proto::SchemaError);
  EXPECT_THROW(fromMsgpackObject(obj, &ptr2), proto::SchemaError);

  SmallUnsignedInteger::Builder b;
  ptr = b.build();
  EXPECT_EQ(ptr->a, UINT64_C(42));
}

TEST(TestModel, TestBoolean) {
  auto obj = pack(std::make_shared<MsgpackObject>(true));
  std::shared_ptr<Boolean> ptr;
  std::shared_ptr<BooleanOptional> ptr2;
  fromMsgpackObject(obj, &ptr);
  fromMsgpackObject(obj, &ptr2);
  EXPECT_EQ(ptr->a, true);
  EXPECT_EQ(ptr2->a.first, true);
  EXPECT_EQ(ptr2->a.second, true);

  obj = pack(std::make_shared<MsgpackObject>());
  EXPECT_THROW(fromMsgpackObject(obj, &ptr), proto::SchemaError);
  fromMsgpackObject(obj, &ptr2);
  EXPECT_EQ(ptr2->a.first, false);

  obj = pack(std::make_shared<MsgpackObject>(INT64_C(2)));
  EXPECT_THROW(fromMsgpackObject(obj, &ptr), proto::SchemaError);
  EXPECT_THROW(fromMsgpackObject(obj, &ptr2), proto::SchemaError);

  Boolean::Builder b;
  ptr = b.build();
  EXPECT_EQ(ptr->a, true);
}

TEST(TestModel, TestFloat) {
  auto obj = pack(std::make_shared<MsgpackObject>(42.0));
  std::shared_ptr<Float> ptr;
  std::shared_ptr<FloatOptional> ptr2;
  fromMsgpackObject(obj, &ptr);
  fromMsgpackObject(obj, &ptr2);
  EXPECT_EQ(ptr->a, 42.0);
  EXPECT_EQ(ptr2->a.first, true);
  EXPECT_EQ(ptr2->a.second, 42.0);

  obj = pack(std::make_shared<MsgpackObject>());
  EXPECT_THROW(fromMsgpackObject(obj, &ptr), proto::SchemaError);
  fromMsgpackObject(obj, &ptr2);
  EXPECT_EQ(ptr2->a.first, false);

  obj = pack(std::make_shared<MsgpackObject>(false));
  EXPECT_THROW(fromMsgpackObject(obj, &ptr), proto::SchemaError);
  EXPECT_THROW(fromMsgpackObject(obj, &ptr2), proto::SchemaError);

  Float::Builder b;
  ptr = b.build();
  EXPECT_EQ(ptr->a, 5.0);
}

TEST(TestModel, TestString) {
  auto obj = pack(std::make_shared<MsgpackObject>("FOOBAR"));
  std::shared_ptr<String> ptr;
  std::shared_ptr<StringOptional> ptr2;
  fromMsgpackObject(obj, &ptr);
  fromMsgpackObject(obj, &ptr2);
  EXPECT_EQ(*ptr->a, "FOOBAR");
  EXPECT_EQ(ptr2->a.first, true);
  EXPECT_EQ(*ptr2->a.second, "FOOBAR");

  obj = pack(std::make_shared<MsgpackObject>());
  EXPECT_THROW(fromMsgpackObject(obj, &ptr), proto::SchemaError);
  fromMsgpackObject(obj, &ptr2);
  EXPECT_EQ(ptr2->a.first, false);

  obj = pack(std::make_shared<MsgpackObject>(false));
  EXPECT_THROW(fromMsgpackObject(obj, &ptr), proto::SchemaError);
  EXPECT_THROW(fromMsgpackObject(obj, &ptr2), proto::SchemaError);
}

TEST(TestModel, TestBinary) {
  std::vector<uint8_t> data(5, 30);
  auto obj = pack(std::make_shared<MsgpackObject>(data));
  std::shared_ptr<Binary> ptr;
  std::shared_ptr<BinaryOptional> ptr2;
  fromMsgpackObject(obj, &ptr);
  fromMsgpackObject(obj, &ptr2);
  EXPECT_THAT(*ptr->a, ContainerEq(data));
  EXPECT_EQ(ptr2->a.first, true);
  EXPECT_THAT(*ptr2->a.second, ContainerEq(data));

  obj = pack(std::make_shared<MsgpackObject>());
  EXPECT_THROW(fromMsgpackObject(obj, &ptr), proto::SchemaError);
  fromMsgpackObject(obj, &ptr2);
  EXPECT_EQ(ptr2->a.first, false);

  obj = pack(std::make_shared<MsgpackObject>(false));
  EXPECT_THROW(fromMsgpackObject(obj, &ptr), proto::SchemaError);
  EXPECT_THROW(fromMsgpackObject(obj, &ptr2), proto::SchemaError);
}

TEST(TestModel, TestNodeIDModel) {
  std::vector<uint8_t> ext_data(24, 0x30);
  auto obj = pack(std::make_shared<MsgpackObject>(
      static_cast<int>(proto::EXT_NODE_ID), ext_data));
  std::shared_ptr<NodeIDModel> ptr;
  std::shared_ptr<NodeIDModelOptional> ptr2;
  fromMsgpackObject(obj, &ptr);
  fromMsgpackObject(obj, &ptr2);
  EXPECT_THAT(ptr->a->asStdVector(), ContainerEq(ext_data));
  EXPECT_EQ(ptr2->a.first, true);
  EXPECT_THAT(ptr2->a.second->asStdVector(), ContainerEq(ext_data));

  obj = pack(std::make_shared<MsgpackObject>());
  EXPECT_THROW(fromMsgpackObject(obj, &ptr), proto::SchemaError);
  fromMsgpackObject(obj, &ptr2);
  EXPECT_EQ(ptr2->a.first, false);

  obj = pack(std::make_shared<MsgpackObject>(false));
  EXPECT_THROW(fromMsgpackObject(obj, &ptr), proto::SchemaError);
  EXPECT_THROW(fromMsgpackObject(obj, &ptr2), proto::SchemaError);

  obj = pack(std::make_shared<MsgpackObject>(
      static_cast<int>(proto::EXT_NODE_ID + 1), ext_data));
  EXPECT_THROW(fromMsgpackObject(obj, &ptr), proto::SchemaError);
  EXPECT_THROW(fromMsgpackObject(obj, &ptr2), proto::SchemaError);
}

TEST(TestModel, TestBinData) {
  std::vector<uint8_t> bin(4, 0);
  bin[0] = 8;
  std::vector<uint8_t> raw_data({11, 12, 13, 14});
  bin.insert(bin.end(), raw_data.begin(), raw_data.end());
  auto obj = pack(std::make_shared<MsgpackObject>(
      static_cast<int>(proto::EXT_BINDATA), bin));
  std::shared_ptr<BinDataModel> ptr;
  std::shared_ptr<BinDataModelOptional> ptr2;
  fromMsgpackObject(obj, &ptr);
  fromMsgpackObject(obj, &ptr2);
  EXPECT_EQ(ptr->a->width(), 8u);
  EXPECT_EQ(ptr2->a.first, true);
  EXPECT_EQ(ptr2->a.second->width(), 8u);
  for (int i = 0; i < 4; ++i) {
    EXPECT_EQ(ptr->a->rawData()[i], raw_data[i]);
    EXPECT_EQ(ptr2->a.second->rawData()[i], raw_data[i]);
  }
  auto obj2 = ptr->serializeToMsgpackObject();
  fromMsgpackObject(obj2, &ptr2);
  EXPECT_EQ(*ptr->a, *ptr2->a.second);

  obj = pack(std::make_shared<MsgpackObject>());
  EXPECT_THROW(fromMsgpackObject(obj, &ptr), proto::SchemaError);
  fromMsgpackObject(obj, &ptr2);
  EXPECT_EQ(ptr2->a.first, false);

  obj = pack(std::make_shared<MsgpackObject>(false));
  EXPECT_THROW(fromMsgpackObject(obj, &ptr), proto::SchemaError);
  EXPECT_THROW(fromMsgpackObject(obj, &ptr2), proto::SchemaError);

  obj = pack(std::make_shared<MsgpackObject>(
      static_cast<int>(proto::EXT_BINDATA + 1), bin));
  EXPECT_THROW(fromMsgpackObject(obj, &ptr), proto::SchemaError);
  EXPECT_THROW(fromMsgpackObject(obj, &ptr2), proto::SchemaError);
}

TEST(TestModel, TestList) {
  std::vector<std::shared_ptr<MsgpackObject>> data(
      5, std::make_shared<MsgpackObject>(INT64_C(30)));
  auto obj = pack(std::make_shared<MsgpackObject>(data));
  std::shared_ptr<List> ptr;
  std::shared_ptr<ListOptional> ptr2;
  fromMsgpackObject(obj, &ptr);
  fromMsgpackObject(obj, &ptr2);
  std::vector<int64_t> raw_data(5, INT64_C(30));
  EXPECT_THAT(*ptr->a, ContainerEq(raw_data));
  EXPECT_EQ(ptr2->a.first, true);
  EXPECT_THAT(*ptr2->a.second, ContainerEq(raw_data));

  obj = pack(std::make_shared<MsgpackObject>());
  EXPECT_THROW(fromMsgpackObject(obj, &ptr), proto::SchemaError);
  fromMsgpackObject(obj, &ptr2);
  EXPECT_EQ(ptr2->a.first, false);

  obj = pack(std::make_shared<MsgpackObject>(false));
  EXPECT_THROW(fromMsgpackObject(obj, &ptr), proto::SchemaError);
  EXPECT_THROW(fromMsgpackObject(obj, &ptr2), proto::SchemaError);
}

TEST(TestModel, TestSet) {
  std::vector<std::shared_ptr<MsgpackObject>> data(
      5, std::make_shared<MsgpackObject>(INT64_C(30)));
  auto obj = pack(std::make_shared<MsgpackObject>(data));
  std::shared_ptr<Set> ptr;
  std::shared_ptr<SetOptional> ptr2;
  fromMsgpackObject(obj, &ptr);
  fromMsgpackObject(obj, &ptr2);
  std::unordered_set<int64_t> raw_data({INT64_C(30)});
  EXPECT_THAT(*ptr->a, ContainerEq(raw_data));
  EXPECT_EQ(ptr2->a.first, true);
  EXPECT_THAT(*ptr2->a.second, ContainerEq(raw_data));

  obj = pack(std::make_shared<MsgpackObject>());
  EXPECT_THROW(fromMsgpackObject(obj, &ptr), proto::SchemaError);
  fromMsgpackObject(obj, &ptr2);
  EXPECT_EQ(ptr2->a.first, false);

  obj = pack(std::make_shared<MsgpackObject>(false));
  EXPECT_THROW(fromMsgpackObject(obj, &ptr), proto::SchemaError);
  EXPECT_THROW(fromMsgpackObject(obj, &ptr2), proto::SchemaError);
}

TEST(TestModel, TestMap) {
  std::map<std::string, std::shared_ptr<MsgpackObject>> data;
  data["foo"] = std::make_shared<MsgpackObject>(INT64_C(5));
  data["bar"] = std::make_shared<MsgpackObject>(INT64_C(42));
  auto obj = pack(std::make_shared<MsgpackObject>(data));
  std::shared_ptr<Map> ptr;
  std::shared_ptr<MapOptional> ptr2;
  fromMsgpackObject(obj, &ptr);
  fromMsgpackObject(obj, &ptr2);
  std::unordered_map<std::string, int64_t> raw_data;
  raw_data["foo"] = INT64_C(5);
  raw_data["bar"] = INT64_C(42);
  EXPECT_THAT(*ptr->a, ContainerEq(raw_data));
  EXPECT_EQ(ptr2->a.first, true);
  EXPECT_THAT(*ptr2->a.second, ContainerEq(raw_data));

  obj = pack(std::make_shared<MsgpackObject>());
  EXPECT_THROW(fromMsgpackObject(obj, &ptr), proto::SchemaError);
  fromMsgpackObject(obj, &ptr2);
  EXPECT_EQ(ptr2->a.first, false);

  obj = pack(std::make_shared<MsgpackObject>(false));
  EXPECT_THROW(fromMsgpackObject(obj, &ptr), proto::SchemaError);
  EXPECT_THROW(fromMsgpackObject(obj, &ptr2), proto::SchemaError);
}

TEST(TestModel, TestObject) {
  auto obj = pack(std::make_shared<MsgpackObject>("FOOBAR"));
  obj = pack(obj);
  std::shared_ptr<Object> ptr;
  std::shared_ptr<ObjectOptional> ptr2;
  fromMsgpackObject(obj, &ptr);
  fromMsgpackObject(obj, &ptr2);
  EXPECT_EQ(*ptr->a->a, "FOOBAR");
  EXPECT_EQ(ptr2->a.first, true);
  EXPECT_EQ(*ptr2->a.second->a, "FOOBAR");

  obj = pack(std::make_shared<MsgpackObject>());
  EXPECT_THROW(fromMsgpackObject(obj, &ptr), proto::SchemaError);
  fromMsgpackObject(obj, &ptr2);
  EXPECT_EQ(ptr2->a.first, false);

  obj = pack(std::make_shared<MsgpackObject>(false));
  EXPECT_THROW(fromMsgpackObject(obj, &ptr), proto::SchemaError);
  EXPECT_THROW(fromMsgpackObject(obj, &ptr2), proto::SchemaError);
}

TEST(TestModel, TestEnum) {
  auto obj = pack(std::make_shared<MsgpackObject>("OPT2"));
  std::shared_ptr<Enum> ptr;
  std::shared_ptr<EnumOptional> ptr2;
  fromMsgpackObject(obj, &ptr);
  fromMsgpackObject(obj, &ptr2);
  EXPECT_EQ(ptr->a, TestEnum::OPT2);
  EXPECT_EQ(ptr2->a.first, true);
  EXPECT_EQ(ptr2->a.second, TestEnum::OPT2);

  obj = pack(std::make_shared<MsgpackObject>());
  EXPECT_THROW(fromMsgpackObject(obj, &ptr), proto::SchemaError);
  fromMsgpackObject(obj, &ptr2);
  EXPECT_EQ(ptr2->a.first, false);

  obj = pack(std::make_shared<MsgpackObject>(false));
  EXPECT_THROW(fromMsgpackObject(obj, &ptr), proto::SchemaError);
  EXPECT_THROW(fromMsgpackObject(obj, &ptr2), proto::SchemaError);
}

TEST(TestModel, TestPolyModel) {
  std::map<std::string, std::shared_ptr<MsgpackObject>> data;
  data["a"] = std::make_shared<MsgpackObject>("test-base-attr");
  data["b"] = std::make_shared<MsgpackObject>("test-sub-attr");
  data["object_type"] = std::make_shared<MsgpackObject>("sub1");
  auto obj = std::make_shared<MsgpackObject>(data);
  std::shared_ptr<SubType1> ptr;
  std::shared_ptr<SubType2> ptr2;
  std::shared_ptr<BaseModel> ptr3;
  fromMsgpackObject(obj, &ptr);
  EXPECT_EQ(*ptr->a, "test-base-attr");
  EXPECT_EQ(*ptr->b, "test-sub-attr");
  EXPECT_THROW(fromMsgpackObject(obj, &ptr2), proto::SchemaError);
  fromMsgpackObject(obj, &ptr3);
  EXPECT_EQ(ptr3->object_type, "sub1");
  ptr = std::dynamic_pointer_cast<SubType1>(ptr3);
  EXPECT_NE(ptr, nullptr);
  EXPECT_EQ(*ptr->a, "test-base-attr");
  EXPECT_EQ(*ptr->b, "test-sub-attr");
  ptr2 = std::dynamic_pointer_cast<SubType2>(ptr3);
  EXPECT_EQ(ptr2, nullptr);

  ptr = nullptr;
  ptr2 = nullptr;
  ptr3 = nullptr;
  (*obj->getMap())["b"] = std::make_shared<MsgpackObject>(
      std::make_shared<std::vector<uint8_t>>(5, 30));
  EXPECT_THROW(fromMsgpackObject(obj, &ptr), proto::SchemaError);
  fromMsgpackObject(obj, &ptr2);
  EXPECT_EQ(*ptr2->a, "test-base-attr");
  EXPECT_THAT(*ptr2->b, ContainerEq(std::vector<uint8_t>(5, 30)));
  EXPECT_THROW(fromMsgpackObject(obj, &ptr3), proto::SchemaError);

  ptr = nullptr;
  ptr2 = nullptr;
  ptr3 = nullptr;
  (*obj->getMap())["object_type"] = std::make_shared<MsgpackObject>("sub2");
  EXPECT_THROW(fromMsgpackObject(obj, &ptr), proto::SchemaError);
  fromMsgpackObject(obj, &ptr2);
  EXPECT_EQ(*ptr2->a, "test-base-attr");
  EXPECT_THAT(*ptr2->b, ContainerEq(std::vector<uint8_t>(5, 30)));
  fromMsgpackObject(obj, &ptr3);
  EXPECT_EQ(ptr3->object_type, "sub2");
  ptr = std::dynamic_pointer_cast<SubType1>(ptr3);
  EXPECT_EQ(ptr, nullptr);
  ptr2 = std::dynamic_pointer_cast<SubType2>(ptr3);
  EXPECT_NE(ptr2, nullptr);
  EXPECT_EQ(*ptr2->a, "test-base-attr");
  EXPECT_THAT(*ptr2->b, ContainerEq(std::vector<uint8_t>(5, 30)));
}

}  // namespace messages
}  // namespace veles
