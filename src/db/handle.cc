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
#include "db/handle.h"
#include "db/universe.h"
#include "db/object.h"
#include "db/getter.h"
#include "dbif/info.h"
#include "dbif/error.h"
#include "dbif/method.h"
#include "dbif/promise.h"

namespace veles {
namespace db {

InfoPromise *LocalObjectHandle::getInfo(PInfoRequest req) {
  InfoPromise *promise = new InfoPromise;
  InfoGetter *getter = new InfoGetter;
  getter->moveToThread(db_->thread());
  QObject::connect(getter, &InfoGetter::gotInfo, promise, &InfoPromise::gotInfo);
  QObject::connect(getter, &InfoGetter::gotError, promise, &InfoPromise::gotError);
  QObject::connect(getter, &InfoGetter::gotInfo, promise, &QObject::deleteLater);
  QObject::connect(getter, &InfoGetter::gotError, promise, &QObject::deleteLater);
  QObject::connect(getter, &QObject::destroyed, promise, &QObject::deleteLater);
  QObject::connect(promise, &QObject::destroyed, getter, &QObject::deleteLater);
  QObject::connect(getter, &InfoGetter::getInfo, db_, &Universe::getInfo);
  emit getter->getInfo(obj_, getter, req, true);
  return promise;
}

InfoPromise *LocalObjectHandle::subInfo(PInfoRequest req) {
  InfoPromise *promise = new InfoPromise;
  InfoGetter *getter = new InfoGetter;
  getter->moveToThread(db_->thread());
  QObject::connect(getter, &InfoGetter::gotInfo, promise, &InfoPromise::gotInfo);
  QObject::connect(getter, &InfoGetter::gotError, promise, &InfoPromise::gotError);
  QObject::connect(getter, &InfoGetter::gotError, promise, &QObject::deleteLater);
  QObject::connect(getter, &QObject::destroyed, promise, &QObject::deleteLater);
  QObject::connect(promise, &QObject::destroyed, getter, &QObject::deleteLater);
  QObject::connect(getter, &InfoGetter::getInfo, db_, &Universe::getInfo);
  emit getter->getInfo(obj_, getter, req, false);
  return promise;
}

MethodResultPromise *LocalObjectHandle::runMethod(PMethodRequest req) {
  MethodResultPromise *promise = new MethodResultPromise;
  MethodRunner *runner = new MethodRunner;
  runner->moveToThread(db_->thread());
  QObject::connect(runner, &MethodRunner::gotResult, promise, &MethodResultPromise::gotResult);
  QObject::connect(runner, &MethodRunner::gotError, promise, &MethodResultPromise::gotError);
  QObject::connect(runner, &MethodRunner::gotResult, promise, &QObject::deleteLater);
  QObject::connect(runner, &MethodRunner::gotError, promise, &QObject::deleteLater);
  QObject::connect(runner, &QObject::destroyed, promise, &QObject::deleteLater);
  QObject::connect(promise, &QObject::destroyed, runner, &QObject::deleteLater);
  QObject::connect(runner, &MethodRunner::runMethod, db_, &Universe::runMethod);
  emit runner->runMethod(obj_, runner, req);
  return promise;
}

MethodRunner *MethodRunner::forwarder(QThread *thread) {
  MethodRunner *res = new MethodRunner;
  res->moveToThread(thread);
  QObject::connect(res, &MethodRunner::gotResult, this, &MethodRunner::gotResult);
  QObject::connect(res, &MethodRunner::gotError, this, &MethodRunner::gotError);
  return res;
}

};
};
