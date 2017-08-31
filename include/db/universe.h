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
#include <QStringList>
#include "data/bindata.h"
#include "db/getter.h"
#include "dbif/types.h"
#include "parser/parser.h"

namespace veles {
namespace db {

class ParserWorker : public QObject {
  Q_OBJECT

 public slots:
  void parse(const veles::dbif::ObjectHandle& blob, MethodRunner* runner,
             const QString& parser_id, quint64 start = 0,
             const veles::dbif::ObjectHandle& parent_chunk =
                 veles::dbif::ObjectHandle());

 public:
  void registerParser(parser::Parser* parser);
  QStringList parserIdsList();
  ~ParserWorker() override;

 private:
  QList<parser::Parser*> _parsers;

 signals:
  void newParser(QString id);
};

}  // namespace db
}  // namespace veles
