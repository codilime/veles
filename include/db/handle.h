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
#ifndef VELES_DB_HANDLE_H
#define VELES_DB_HANDLE_H

#include "dbif/universe.h"
#include "dbif/types.h"
#include "db/types.h"

namespace veles {
namespace db {

class LocalObjectHandle : public dbif::ObjectHandleBase {
  Universe *db_;
  PLocalObject obj_;
  dbif::ObjectType type_;
 public:
  LocalObjectHandle(Universe *db, PLocalObject obj, dbif::ObjectType type) :
    db_(db), obj_(obj), type_(type) {}
  dbif::InfoPromise *getInfo(dbif::PInfoRequest req) override;
  dbif::InfoPromise *subInfo(dbif::PInfoRequest req) override;
  dbif::MethodResultPromise *runMethod(dbif::PMethodRequest req) override;
  dbif::ObjectType type() const override {
    return type_;
  }
  PLocalObject obj() const { return obj_; }
};

};
};

#endif
