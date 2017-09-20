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
#pragma once

#include <QObject>

#include "dbif/types.h"

namespace veles {
namespace db {

class InfoGetter : public QObject {
  Q_OBJECT

 signals:
  void gotInfo(veles::dbif::PInfoReply x);
  void gotError(veles::dbif::PError x);

 public:
  template <typename Reply, typename... Args>
  void sendInfo(Args... args) {
    emit gotInfo(QSharedPointer<Reply>::create(args...));
  }
  template <typename Err, typename... Args>
  void sendError(Args... args) {
    emit gotError(QSharedPointer<Err>::create(args...));
  }
};

class MethodRunner : public QObject {
  Q_OBJECT

 signals:
  void gotResult(veles::dbif::PMethodReply x);
  void gotError(veles::dbif::PError x);

 public:
  template <typename Err, typename... Args>
  void sendError(Args... args) {
    emit gotError(QSharedPointer<Err>::create(args...));
  }
  template <typename Reply, typename... Args>
  void sendResult(Args... args) {
    emit gotResult(QSharedPointer<Reply>::create(args...));
  }
  MethodRunner* forwarder(QThread* thread);
};

}  // namespace db
}  // namespace veles
