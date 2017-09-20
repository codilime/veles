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
#include "ui/rootfileblobitem.h"

#include <QtAlgorithms>

#include "dbif/universe.h"
#include "ui/subchunkfileblobitem.h"

namespace veles {
namespace ui {

RootFileBlobItem::RootFileBlobItem(const dbif::ObjectHandle& obj,
                                   QObject* parent)
    : FileBlobItem("", "", "", 0, 0, parent) {
  dataObj_ = obj;
  auto childrenPromise = dataObj_->asyncSubInfo<dbif::ChildrenRequest>(this);
  connect(childrenPromise, &dbif::InfoPromise::gotInfo, this,
          &RootFileBlobItem::gotChildrenResponse);
}

void RootFileBlobItem::gotChildrenResponse(
    const veles::dbif::PInfoReply& reply) {
  FileBlobItem::removeOldChildren();
  auto objects = reply.dynamicCast<dbif::ChildrenRequest::ReplyType>()->objects;

  QList<FileBlobItem*> newChildren;

  for (auto& object : objects) {
    newChildren.append(new SubchunkFileBlobItem(object, this));
  }

  addChildren(newChildren);
}

}  // namespace ui
}  // namespace veles
