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
#include "dbif/info.h"
#include "dbif/method.h"
#include "dbif/error.h"
#include "dbif/universe.h"

namespace veles {
namespace dbif {

void MethodRequest::key() {}

namespace {
class Register {
 public:
  Register() {
    qRegisterMetaType<veles::dbif::ObjectHandle>("veles::dbif::ObjectHandle");
    qRegisterMetaType<veles::dbif::PInfoRequest>("veles::dbif::PInfoRequest");
    qRegisterMetaType<veles::dbif::PInfoReply>("veles::dbif::PInfoReply");
    qRegisterMetaType<veles::dbif::PMethodRequest>("veles::dbif::PMethodRequest");
    qRegisterMetaType<veles::dbif::PMethodReply>("veles::dbif::PMethodReply");
    qRegisterMetaType<veles::dbif::PError>("veles::dbif::PError");
  }
} _;
};

};
};

