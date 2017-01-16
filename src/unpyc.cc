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
#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include <QFile>
#include "parser/unpyc.h"
#include "db/db.h"
#include "dbif/info.h"
#include "dbif/method.h"
#include "dbif/promise.h"
#include "dbif/error.h"
#include "dbif/universe.h"
#include "parser/stream.h"
#include "data/field.h"

int main(int argc, char **argv) {
  QCoreApplication app(argc, argv);
  QFile f(argv[1]);
  bool ok = f.open(QIODevice::ReadOnly);
  if (!ok)
    return 1;
  uint64_t size = f.size();
  QByteArray qdata = f.read(size);
  veles::dbif::ObjectHandle obj = veles::db::create_db();
  veles::data::BinData vec(8, qdata.size(), reinterpret_cast<uint8_t *>(qdata.data()));
  auto blob = obj->syncRunMethod<veles::dbif::RootCreateFileBlobFromDataRequest>(vec, argv[1])->object;
  veles::parser::unpycFileBlob(blob);
  return 0;
}
