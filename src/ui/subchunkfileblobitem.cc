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
#include "ui/subchunkfileblobitem.h"
#include "ui/simplefileblobitem.h"

#include "dbif/universe.h"

namespace veles {
namespace ui {

int SubchunkFileBlobItem::childrenCount() {
  subscribeInfo();
  return FileBlobItem::childrenCount();
}

void SubchunkFileBlobItem::gotChunkDataResponse(
    const veles::dbif::PInfoReply& reply) {
  FileBlobItem::removeOldChildren();

  auto items = reply.dynamicCast<dbif::ChunkDataRequest::ReplyType>()->items;

  QList<FileBlobItem*> newChildren;

  for (auto& item : items) {
    if (item.type == data::ChunkDataItem::ChunkDataItemType::SUBCHUNK) {
      newChildren.append(new SubchunkFileBlobItem(item.ref[0], this));
    } else if (item.type == data::ChunkDataItem::ChunkDataItemType::FIELD) {
      QString comment;
      if (item.num_elements > 1) {
        comment += QString::number(item.num_elements) + " x ";
      }
      comment += QString::number(item.repack.to_width) + "b (";
      if (item.repack.endian == veles::data::Endian::LITTLE) {
        comment += "LE";
      } else {
        comment += "BE";
      }
      comment += ")";
      newChildren.append(new FileBlobItem(item.name,
                                          item.raw_value.toString(16), comment,
                                          item.start, item.end, this));
    } else if (item.type == data::ChunkDataItem::ChunkDataItemType::SUBBLOB) {
      auto child = new SimpleFileBlobItem(item.name, "open in new tab", this);
      child->setIcon(QIcon::fromTheme(":/images/newTab.png"));
      child->setNewRoot(item.ref[0]);
      newChildren.append(child);
    } else {
      auto child = new SimpleFileBlobItem(item.name, "unsupported", this);
      child->setIcon(QIcon::fromTheme(":/images/error.ico"));
      newChildren.append(child);
    }
  }

  addChildren(newChildren);
}

void SubchunkFileBlobItem::gotChunkDescriptionResponse(
    const veles::dbif::PInfoReply& reply) {
  if (auto description = reply.dynamicCast<dbif::ChunkDescriptionReply>()) {
    setFields(description->name, description->comment, description->start,
              description->end);
    emit dataUpdated(this);
  }
}

void SubchunkFileBlobItem::gotError(const veles::dbif::PError& error) {
  if (error.dynamicCast<veles::dbif::ObjectGoneError>() != nullptr) {
    FileBlobItem::setFields("removed", "", 0, 0);
  } else {
    FileBlobItem::setFields("error", "", 0, 0);
  }
}

void SubchunkFileBlobItem::subscribeInfo() {
  if (infoSubscribed_) {
    return;
  }

  auto dataPromise = dataObj_->asyncSubInfo<dbif::ChunkDataRequest>(this);
  connect(dataPromise, SIGNAL(gotInfo(veles::dbif::PInfoReply)), this,
          SLOT(gotChunkDataResponse(veles::dbif::PInfoReply)));

  dbif::DescriptionRequest req;
  auto descriptionPromise =
      dataObj_->asyncSubInfo<dbif::DescriptionRequest>(this, req);
  connect(descriptionPromise, SIGNAL(gotInfo(veles::dbif::PInfoReply)), this,
          SLOT(gotChunkDescriptionResponse(veles::dbif::PInfoReply)));
  connect(descriptionPromise, SIGNAL(gotError(veles::dbif::PError)), this,
          SLOT(gotError(veles::dbif::PError)));
  infoSubscribed_ = true;
}

SubchunkFileBlobItem::SubchunkFileBlobItem(const dbif::ObjectHandle& obj,
                                           QObject* parent)
    : FileBlobItem("loading", "", "loading", 0, 0, parent),
      infoSubscribed_(false) {
  dataObj_ = obj;
}

QString SubchunkFileBlobItem::name() {
  subscribeInfo();
  return FileBlobItem::name();
}

void SubchunkFileBlobItem::setComment(const QString& comment) {
  if (dataObj_->type() == dbif::CHUNK) {
    dataObj_->asyncRunMethod<dbif::SetCommentRequest>(this, comment);
  }
}

}  // namespace ui
}  // namespace veles
