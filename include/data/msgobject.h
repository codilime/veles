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

#ifndef VELES_DATA_MSGPACK_H
#define VELES_DATA_MSGPACK_H
#include <sstream>

#include <msgpack.hpp>

namespace veles {
namespace data {

class NodeID {
  static const size_t width = 24;
  unsigned char value[width];
 public:
  static const unsigned char NIL_VALUE[width];

  NodeID();
  NodeID(const unsigned char* data);
  NodeID(const std::string& data);

  std::string toString();
  std::vector<uint8_t> asVector() const {
    return std::vector<uint8_t>(value, value+width);
  }
};

}  // namespace data
}  // namespace veles
#endif // VELES_DATA_MSGPACK_H
