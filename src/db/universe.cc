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
#include <QThread>

#include "db/universe.h"
#include "dbif/promise.h"
#include "dbif/error.h"
#include "db/handle.h"
#include "db/object.h"
#include "db/getter.h"
#include "db/db.h"

#include "parser/unpyc.h"
#include "parser/unpng.h"

namespace veles {
namespace db {

class DbThread : public QThread {
 protected:
  void run() {
    exec();
  }
};

dbif::ObjectHandle create_db() {
  ParserWorker *parser = new ParserWorker;
  Universe *db = new Universe(parser);
  PLocalObject root = RootLocalObject::create(db);
  db->setRoot(root);
  DbThread *thr = new DbThread;
  DbThread *parser_thr = new DbThread;
  db->moveToThread(thr);
  parser->moveToThread(parser_thr);
  QObject::connect(db, &QObject::destroyed, thr, &QThread::quit);
  QObject::connect(parser, &QObject::destroyed, parser_thr, &QThread::quit);
  QObject::connect(db, &QObject::destroyed, parser, &QObject::deleteLater);
  QObject::connect(db, &Universe::parse, parser, &ParserWorker::parse);
  thr->start();
  parser_thr->start();
  return db->handle(root);
}

dbif::ObjectHandle Universe::handle(PLocalObject obj) {
  dbif::ObjectHandle objHandle;
  if (obj) {
    objHandle = QSharedPointer<LocalObjectHandle>::create(this, obj, obj->type());
  }
  return objHandle;
}

Universe::~Universe() {
  root_->kill();
}

void Universe::getInfo(PLocalObject obj, InfoGetter *getter, dbif::PInfoRequest req, bool once) {
  if (obj->dead()) {
    emit getter->gotError(QSharedPointer<dbif::ObjectGoneError>::create());
  } else {
    obj->getInfo(getter, req, once);
  }
}

void Universe::runMethod(PLocalObject obj, MethodRunner *runner, dbif::PMethodRequest req) {
  if (obj->dead()) {
    emit runner->gotError(QSharedPointer<dbif::ObjectGoneError>::create());
  } else {
    obj->runMethod(runner, req);
  }
}

void ParserWorker::parse(dbif::ObjectHandle blob, MethodRunner *runner) {
  auto data = blob->syncGetInfo<dbif::BlobDataRequest>(0, 4)->data;
  if (data.size() != 4)
    return;
  if (data.element64(0) == 0x89 && data.element64(1) == 'P' &&
      data.element64(2) == 'N' && data.element64(3) == 'G')
    veles::parser::unpngFileBlob(blob);
  else if (data.element64(2) == '\r' && data.element64(3) == '\n' &&
           ((data.element64(0) == 0x9e && data.element64(1) == 0x0c) ||
            (data.element64(0) == 0xee && data.element64(1) == 0x0c) ||
            (data.element64(0) == 0x16 && data.element64(1) == 0x0d)))
    veles::parser::unpycFileBlob(blob);
  runner->sendResult<dbif::NullReply>();
  delete runner;
}

};
};
