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

#include <QList>

#include "dbif/info.h"
#include "parser/parser.h"

namespace veles {
namespace parser {

dbif::ObjectHandle findSubChunk(dbif::ObjectHandle parent, const QString &name);
std::shared_ptr<proto::ChunkDataItem> findField(dbif::ObjectHandle parent, const QString &name);
dbif::ObjectHandle makeSubBlob(dbif::ObjectHandle parent, const QString &name,
                               const data::BinData &data);
QList<Parser *> createAllParsers();

}  // namespace parser
}  // namespace veles
