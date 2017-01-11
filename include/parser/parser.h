/*
 * Copyright 2017 CodiLime
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
#ifndef VELES_PARSER_PARSER_H
#define VELES_PARSER_PARSER_H

#include <QString>
#include "data/bindata.h"
#include "data/types.h"

namespace veles {
namespace parser {

class Parser {
 public:
  virtual ~Parser() {}
  Parser(QString id, QList<data::BinData> magic) : _id(id), _magic(magic) {}
  Parser(QString id, data::BinData magic) : _id(id) { _magic.append(magic); }
  Parser(QString id) : _id(id) {}
  QString id() { return _id; }
  QList<data::BinData> magic() { return _magic; }
  bool verifyAndParse(dbif::ObjectHandle blob, uint64_t start = 0,
                      dbif::ObjectHandle parent_chunk = dbif::ObjectHandle());
  virtual void parse(
      dbif::ObjectHandle blob, uint64_t start = 0,
      dbif::ObjectHandle parent_chunk = dbif::ObjectHandle()) = 0;

 private:
  QString _id;
  QList<data::BinData> _magic;
};

};  // namespace parser
};  // namespace veles

#endif  // VELES_PARSER_PARSER_H
