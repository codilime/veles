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
#ifndef VELES_DBIF_PROMISE
#define VELES_DBIF_PROMISE

#include <QObject>

#include "dbif/types.h"

namespace veles {
namespace dbif {

class InfoPromise : public QObject {
  Q_OBJECT

 signals:
  void gotInfo(veles::dbif::PInfoReply x);
  void gotError(veles::dbif::PError x);
};

class MethodResultPromise : public QObject {
  Q_OBJECT

 signals:
  void gotResult(veles::dbif::PMethodReply x);
  void gotError(veles::dbif::PError x);
};

};
};

#endif
