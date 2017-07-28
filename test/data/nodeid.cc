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

#include "data/nodeid.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using testing::AllOf;
using testing::Ne;
using testing::ContainerEq;

namespace veles {
namespace data {

TEST(NodeID, SimpleCreation) {
  NodeID n1, n2, n3, n4;
  EXPECT_THAT(n1, AllOf(Ne(n2), Ne(n3), Ne(n4)));
  EXPECT_THAT(n2, AllOf(Ne(n1), Ne(n3), Ne(n4)));
  EXPECT_THAT(n3, AllOf(Ne(n2), Ne(n1), Ne(n4)));
  EXPECT_THAT(n4, AllOf(Ne(n2), Ne(n3), Ne(n1)));

  NodeID nil(NodeID::NIL_VALUE), root(NodeID::ROOT_VALUE);
  EXPECT_EQ(nil, *NodeID::getNilId());
  EXPECT_EQ(root, *NodeID::getRootNodeId());

  NodeID from_str("000000000000000000000000");
  const uint8_t data[] = {
      0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
      0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
  };
  NodeID from_char(data);
  EXPECT_EQ(from_str, from_char);
}

TEST(NodeID, ToFromHexString) {
  QString str(NodeID::WIDTH * 2, QChar('3'));
  auto id = NodeID::fromHexString(str);
  EXPECT_EQ(str, id->toHexString());
  NodeID from_str("333333333333333333333333");
  EXPECT_EQ(*id, from_str);
  EXPECT_EQ(from_str.toHexString(), str);
  QString str2(NodeID::WIDTH, QChar('3'));
  EXPECT_EQ(NodeID::fromHexString(str2), nullptr);
}

TEST(NodeID, ToVector) {
  EXPECT_THAT(NodeID::getNilId()->asStdVector(),
              ContainerEq(std::vector<uint8_t>(NodeID::WIDTH, 0)));
  EXPECT_THAT(NodeID::getRootNodeId()->asStdVector(),
              ContainerEq(std::vector<uint8_t>(NodeID::WIDTH, 255)));
}

TEST(NodeID, TestBoolConv) {
  NodeID n;
  EXPECT_TRUE(n);
  EXPECT_FALSE(*NodeID::getNilId());
  EXPECT_TRUE(*NodeID::getRootNodeId());
}

}  // namespace data
}  // namespace veles
