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

#include <cstdint>

#include <QtNetwork/QTcpSocket>
#include <msgpack.hpp>

#include "models.h"
#include "network/msgpackobject.h"
#include "proto/exceptions.h"

namespace veles {
namespace messages {

class MsgpackWrapper {
  msgpack::unpacker unp_;
  static const int READ_SIZE_ = 1024;

 public:
  static std::shared_ptr<proto::MsgpackMsg> parseMessage(
      msgpack::object_handle* handle) {
    msgpack::object obj = handle->get();
    return proto::MsgpackMsg::polymorphicLoad(obj);
  }

  template <class Packer, class T>
  static void dumpObject(Packer& pk, std::shared_ptr<T> ptr) {
    auto mss = toMsgpackObject(ptr);
    pk.pack(mss);
  }

  std::shared_ptr<proto::MsgpackMsg> loadMessage(QTcpSocket* connection) {
    // This method can throw msgpack::type_error when malformed message is read
    msgpack::object_handle handle;
    if (unp_.next(handle)) {
      return parseMessage(&handle);
    }

    while (true) {
      unp_.reserve_buffer(READ_SIZE_);
      qint64 read = connection->read(unp_.buffer(), READ_SIZE_);
      if (read <= 0) {
        return nullptr;
      }
      unp_.buffer_consumed(read);
      if (unp_.next(handle)) {
        return parseMessage(&handle);
      }
    }
  }
};

}  // namespace messages
}  // namespace veles
