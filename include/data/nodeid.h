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
#pragma once

#include <vector>

#include <msgpack.hpp>

#include <QString>

namespace veles {
namespace data {

class NodeID {
 public:
  static const size_t WIDTH = 24;

 private:
  uint8_t value[WIDTH];

 public:
  static const uint8_t NIL_VALUE[WIDTH];
  static const uint8_t ROOT_VALUE[WIDTH];

  NodeID();
  explicit NodeID(const uint8_t* data);
  explicit NodeID(const std::string& data);
  NodeID(const NodeID& other);

  QString toHexString() const;
  static std::shared_ptr<NodeID> fromHexString(const QString& val);
  std::vector<uint8_t> asStdVector() const;
  // functions for more convenient getting of special values
  static std::shared_ptr<NodeID> getRootNodeId();
  static std::shared_ptr<NodeID> getNilId();
  bool operator==(const NodeID& other) const;
  bool operator!=(const NodeID& other) const;
  bool operator<(const NodeID& other) const;
  explicit operator bool() const;
};

}  // namespace data
}  // namespace veles
