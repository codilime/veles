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

#include <sstream>

#include <msgpack.hpp>

namespace veles {
namespace data {

class NodeID {
  static const size_t width = 24;
  uint8_t value[width];
 public:
  static const uint8_t NIL_VALUE[width];
  // TODO singleton
  static const uint8_t ROOT_VALUE[width];

  NodeID();
  NodeID(const uint8_t* data);
  NodeID(const std::string& data);

  std::string toHexString();
  std::vector<uint8_t> asVector() const {
    return std::vector<uint8_t>(value, value+width);
  }
};

}  // namespace data
}  // namespace veles
