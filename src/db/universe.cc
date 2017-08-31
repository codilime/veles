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

#include "db/getter.h"
#include "db/universe.h"
#include "dbif/error.h"
#include "dbif/method.h"
#include "dbif/promise.h"

#include "parser/utils.h"

namespace veles {
namespace db {

ParserWorker::~ParserWorker() { qDeleteAll(_parsers); }

void ParserWorker::registerParser(parser::Parser* parser) {
  _parsers.append(parser);
  emit newParser(parser->id());
}

QStringList ParserWorker::parserIdsList() {
  QStringList res;
  for (auto parser : _parsers) {
    res.append(parser->id());
  }
  res.sort();
  return res;
}

void ParserWorker::parse(const dbif::ObjectHandle& blob, MethodRunner* runner,
                         const QString& parser_id, quint64 start,
                         const veles::dbif::ObjectHandle& parent_chunk) {
  for (auto parser : _parsers) {
    if (parser_id == "" && !parser->magic().empty()) {
      if (parser->verifyAndParse(blob, start, parent_chunk)) {
        break;
      }
    } else if (parser->id() == parser_id) {
      parser->verifyAndParse(blob, start, parent_chunk);
      break;
    }
  }

  runner->sendResult<dbif::NullReply>();
  delete runner;
}

}  // namespace db
}  // namespace veles
