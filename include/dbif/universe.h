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

#include <QCoreApplication>
#include <QObject>
#include <QPointer>

#include "dbif/error.h"
#include "dbif/info.h"
#include "dbif/method.h"
#include "dbif/promise.h"
#include "dbif/types.h"

namespace veles {
namespace dbif {

class ObjectHandleBase {
  PInfoReply baseSyncGetInfo(const PInfoRequest& req) {
    QPointer<InfoPromise> promise = getInfo(req);
    PInfoReply res;
    PError err;
    QObject::connect(static_cast<InfoPromise*>(promise), &InfoPromise::gotInfo,
                     [&res](PInfoReply reply) { res = reply; });
    QObject::connect(static_cast<InfoPromise*>(promise), &InfoPromise::gotError,
                     [&err](PError error) { err = error; });
    while (true) {
      if (res != nullptr) {
        if (!promise.isNull()) {
          delete static_cast<InfoPromise*>(promise);
        }
        return res;
      }
      if (err != nullptr) {
        if (!promise.isNull()) {
          delete static_cast<InfoPromise*>(promise);
        }
        throw PError(err);
      }
      QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents);
    }
  }

  PMethodReply baseSyncRunMethod(const PMethodRequest& req) {
    QPointer<MethodResultPromise> promise = runMethod(req);
    PMethodReply res;
    PError err;
    QObject::connect(static_cast<MethodResultPromise*>(promise),
                     &MethodResultPromise::gotResult,
                     [&res](PMethodReply reply) { res = reply; });
    QObject::connect(static_cast<MethodResultPromise*>(promise),
                     &MethodResultPromise::gotError,
                     [&err](PError error) { err = error; });
    while (true) {
      if (res != nullptr) {
        if (!promise.isNull()) {
          delete static_cast<MethodResultPromise*>(promise);
        }
        return res;
      }
      if (err != nullptr) {
        if (!promise.isNull()) {
          delete static_cast<MethodResultPromise*>(promise);
        }
        throw PError(err);
      }
      QCoreApplication::processEvents(QEventLoop::WaitForMoreEvents);
    }
  }

 public:
  virtual ~ObjectHandleBase() = default;

  virtual InfoPromise* getInfo(const PInfoRequest& req) = 0;
  virtual InfoPromise* subInfo(const PInfoRequest& req) = 0;
  virtual MethodResultPromise* runMethod(const PMethodRequest& req) = 0;
  virtual ObjectType type() const = 0;

  template <typename Request, typename... Args>
  QSharedPointer<typename Request::ReplyType> syncGetInfo(Args... args) {
    PInfoReply res = baseSyncGetInfo(QSharedPointer<Request>::create(args...));
    return res.dynamicCast<typename Request::ReplyType>();
  }

  template <typename Request, typename... Args>
  QSharedPointer<typename Request::ReplyType> syncRunMethod(Args... args) {
    PMethodReply res =
        baseSyncRunMethod(QSharedPointer<Request>::create(args...));
    return res.dynamicCast<typename Request::ReplyType>();
  }

  template <typename Request, typename... Args>
  InfoPromise* asyncGetInfo(QObject* parent, Args... args) {
    InfoPromise* res = getInfo(QSharedPointer<Request>::create(args...));
    if (parent) {
      res->setParent(parent);
    }
    return res;
  }

  template <typename Request, typename... Args>
  InfoPromise* asyncSubInfo(QObject* parent, Args... args) {
    InfoPromise* res = subInfo(QSharedPointer<Request>::create(args...));
    if (parent) {
      res->setParent(parent);
    }
    return res;
  }

  template <typename Request, typename... Args>
  MethodResultPromise* asyncRunMethod(QObject* parent, Args... args) {
    MethodResultPromise* res =
        runMethod(QSharedPointer<Request>::create(args...));
    if (parent) {
      res->setParent(parent);
    }
    return res;
  }
};

}  // namespace dbif
}  // namespace veles
