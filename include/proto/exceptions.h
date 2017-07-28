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

#include <string>

namespace veles {
namespace proto {

class VelesException {
 public:
  std::string code;
  std::string msg;
  VelesException(const std::string& code, const std::string& msg)
      : code(code), msg(msg) {}
  virtual ~VelesException() {}
};

class SchemaError : public VelesException {
 public:
  explicit SchemaError(const std::string& msg)
      : VelesException("schema_error", msg) {}
};

}  // namespace proto
}  // namespace veles
