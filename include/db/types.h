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
#ifndef VELES_DB_TYPES
#define VELES_DB_TYPES

#include <QObject>
#include <QSharedPointer>
#include <QWeakPointer>
#include <memory>
#include "dbif/types.h"

namespace veles {
namespace db {

class LocalObject;
class Universe;
class InfoGetter;
class MethodRunner;

using dbif::InfoPromise;
using dbif::MethodResultPromise;
using dbif::PInfoRequest;
using dbif::PInfoReply;
using dbif::PMethodRequest;
using dbif::PMethodReply;
using dbif::PError;

typedef QSharedPointer<LocalObject> PLocalObject;
typedef QWeakPointer<LocalObject> WLocalObject;

};
};

#endif
