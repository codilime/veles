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

#include "parser/utils.h"

namespace veles {
namespace db {

class DbThread : public QThread {
 protected:
  void run() {
    exec();
  }
};

dbif::ObjectHandle create_db() {
  ParserWorker *parser_worker = new ParserWorker;
  for (auto parser : parser::createAllParsers()) {
    parser_worker->registerParser(parser);
  }
  Universe *db = new Universe(parser_worker);
  PLocalObject root = RootLocalObject::create(db);
  db->setRoot(root);
  DbThread *thr = new DbThread;
  DbThread *parser_thr = new DbThread;
  db->moveToThread(thr);
  parser_worker->moveToThread(parser_thr);
  QObject::connect(db, &QObject::destroyed, thr, &QThread::quit);
  QObject::connect(parser_worker, &QObject::destroyed, parser_thr, &QThread::quit);
  QObject::connect(db, &QObject::destroyed, parser_worker, &QObject::deleteLater);
  QObject::connect(db, &Universe::parse, parser_worker, &ParserWorker::parse);
  QObject::connect(parser_worker, &ParserWorker::newParser, [root] {
    root.dynamicCast<RootLocalObject>()->parsers_list_updated();
  });
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

ParserWorker::~ParserWorker() { qDeleteAll(_parsers); }

void ParserWorker::registerParser(parser::Parser *parser) {
  _parsers.append(parser);
  emit newParser(parser->id());
}

QStringList ParserWorker::parserIdsList() {
  QStringList res;
  for (auto parser : _parsers) {
    res.append(parser->id());
  }
  return res;
}

void ParserWorker::parse(dbif::ObjectHandle blob, MethodRunner *runner,
                         QString parser_id) {
  for (auto parser : _parsers) {
    if (parser_id == "") {
      for (auto magic : parser->magic()) {
        auto data =
            blob->syncGetInfo<dbif::BlobDataRequest>(0, magic.size())->data;
        if (data == magic) {
          parser->parse(blob);
        }
      }
    } else {
      if (parser->id() == parser_id) {
        parser->parse(blob);
      }
    }
  }

  runner->sendResult<dbif::NullReply>();
  delete runner;
}
};
};
