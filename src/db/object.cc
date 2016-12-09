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
#include "db/handle.h"
#include "db/object.h"
#include "db/getter.h"
#include "db/universe.h"
#include "dbif/error.h"
#include "dbif/info.h"
#include "dbif/method.h"

namespace veles {
namespace db {

void LocalObject::getInfo(InfoGetter *getter, PInfoRequest req, bool once) {
  if (req.dynamicCast<dbif::ChildrenRequest>()) {
    children_reply(getter);
    if (!once) {
      children_watchers_.insert(getter);
      auto shared_this = sharedFromThis();
      QObject::connect(getter, &QObject::destroyed, [shared_this, getter] () {
        shared_this->remove_children_watcher(getter);
      });
    }
  } else if (req.dynamicCast<dbif::DescriptionRequest>()) {
    description_reply(getter);
    if (!once) {
      description_watchers_.insert(getter);
      auto shared_this = sharedFromThis();
      QObject::connect(getter, &QObject::destroyed, [shared_this, getter] () {
        shared_this->remove_description_watcher(getter);
      });
    }
  } else {
    getter->sendError<dbif::ObjectInvalidRequestError>();
  }
}

void LocalObject::remove_description_watcher(InfoGetter * getter) {
  description_watchers_.remove(getter);
}

void LocalObject::remove_children_watcher(InfoGetter * getter) {
  children_watchers_.remove(getter);
}

void LocalObject::runMethod(MethodRunner *runner, PMethodRequest req) {
  if (req.dynamicCast<dbif::DeleteRequest>()) {
    kill();
    runner->sendResult<dbif::NullReply>();
  } else if (auto nreq = req.dynamicCast<dbif::SetNameRequest>()) {
    name_ = nreq->name;
    description_updated();
    runner->sendResult<dbif::NullReply>();
  } else if (auto creq = req.dynamicCast<dbif::SetCommentRequest>()) {
    comment_ = creq->comment;
    description_updated();
    runner->sendResult<dbif::NullReply>();
  } else {
    runner->sendError<dbif::ObjectInvalidRequestError>();
  }
}

void LocalObject::addChild(PLocalObject obj) {
  children_.insert(obj);
  children_updated();
}

void LocalObject::delChild(PLocalObject obj) {
  children_.remove(obj);
  children_updated();
}

void LocalObject::children_reply(InfoGetter *getter) {
  std::vector<dbif::ObjectHandle> res;
  for (PLocalObject obj : children_) {
    res.push_back(db()->handle(obj));
  }
  getter->sendInfo<dbif::ChildrenReply>(res);
}

void LocalObject::description_reply(InfoGetter *getter) {
  getter->sendInfo<dbif::DescriptionReply>(name(), comment());
}

void LocalObject::children_updated() {
  for (InfoGetter *getter : children_watchers_) {
    children_reply(getter);
  }
}

void LocalObject::description_updated() {
  for (InfoGetter *getter : description_watchers_) {
    description_reply(getter);
  }
}

void LocalObject::kill() {
  db_ = nullptr;
  killed();

  auto children = children_;
  for (auto obj: children) {
    obj->kill();
  }

  auto children_watchers = children_watchers_;
  for (auto getter: children_watchers) {
    getter->sendError<dbif::ObjectGoneError>();
  }

  auto description_watchers = description_watchers_;
  for (auto getter: description_watchers) {
    getter->sendError<dbif::ObjectGoneError>();
  }

  Q_ASSERT(children_.empty());
}

void RootLocalObject::runMethod(MethodRunner *runner, PMethodRequest req) {
  if (auto blobreq = req.dynamicCast<dbif::RootCreateFileBlobFromDataRequest>()) {
    PLocalObject obj = FileBlobObject::create(this, blobreq->data, blobreq->path);
    runner->sendResult<dbif::CreatedReply>(db()->handle(obj));
  } else {
    LocalObject::runMethod(runner, req);
  }
}

void DataBlobObject::description_reply(InfoGetter *getter) {
  getter->sendInfo<dbif::BlobDescriptionReply>(
    name(), comment(), 0, data().size(), 8
  );
}

void DataBlobObject::data_reply(InfoGetter *getter, uint64_t start, uint64_t end) {
    end = std::min(end, uint64_t(data_.size()));
    getter->sendInfo<dbif::BlobDataReply>(data_.data(start, end));
}

void DataBlobObject::remove_data_watcher(InfoGetter *getter) {
  data_watchers_.remove(getter);
}

void DataBlobObject::getInfo(InfoGetter *getter, PInfoRequest req, bool once) {
  if (auto datareq = req.dynamicCast<dbif::BlobDataRequest>()) {
    if (datareq->start > data_.size()) {
      getter->sendError<dbif::BlobDataInvalidRangeError>();
      return;
    }
    data_reply(getter, datareq->start, datareq->end);
    if (!once) {
      data_watchers_[getter] = { datareq->start, datareq->end };
      auto shared_this = sharedFromThis();
      QObject::connect(getter, &QObject::destroyed, [shared_this, getter] () {
        shared_this.dynamicCast<DataBlobObject>()->remove_data_watcher(getter);
      });
    }
  } else {
    LocalObject::getInfo(getter, req, once);
  }
}

void DataBlobObject::runMethod(MethodRunner *runner, PMethodRequest req) {
  if (auto datareq = req.dynamicCast<dbif::ChangeDataRequest>()) {
    if (datareq->start >= data_.size()) {
      runner->sendError<dbif::BlobDataInvalidRangeError>();
      return;
    }
    uint64_t start = datareq->start;
    uint64_t end = std::min(datareq->end, uint64_t(data_.size()));
    uint64_t oldsize = end - start;
    const data::BinData &newdata = datareq->data;
    if (newdata.width() != data_.width()) {
      runner->sendError<dbif::BlobDataInvalidWidthError>();
      return;
    }
    if (oldsize == newdata.size()) {
      data_.setData(start, end, newdata);
    } else {
      data::BinData merged(data_.width(), data_.size() - oldsize + newdata.size());
      merged.setData(0, start, data_.data(0, start));
      uint64_t newend = start + newdata.size();
      merged.setData(start, newend, newdata);
      merged.setData(newend, merged.size(), data_.data(end, data_.size()));
      std::swap(data_, merged);
    }
    bool moved = newdata.size() != oldsize;
    for (auto iter = data_watchers_.begin(); iter != data_watchers_.end(); iter++) {
      if (iter.value().second >= start &&
          (moved || iter.value().first <= end)) {
        data_reply(iter.key(), iter.value().first, iter.value().second);
      }
    }
    runner->sendResult<dbif::NullReply>();
  } else if (auto chreq = req.dynamicCast<dbif::ChunkCreateRequest>()) {
    PLocalObject parent_chunk;
    if (chreq->parent_chunk) {
      parent_chunk = chreq->parent_chunk.dynamicCast<LocalObjectHandle>()->obj();
      if (!parent_chunk.dynamicCast<ChunkObject>()) {
        runner->sendError<dbif::InvalidTypeError>();
        return;
      }
    }
    PLocalObject obj = ChunkObject::create(sharedFromThis(), parent_chunk,
      chreq->start, chreq->end, chreq->chunk_type, chreq->name);
    runner->sendResult<dbif::CreatedReply>(db()->handle(obj));
  } else if (req.dynamicCast<dbif::BlobParseRequest>()) {
    emit db()->parse(db()->handle(sharedFromThis()), runner->forwarder(db()->parserThread()));
  } else {
    LocalObject::runMethod(runner, req);
  }
}

void DataBlobObject::killed() {
  LocalObject::killed();
  parent_->delChild(sharedFromThis());
  auto data_watchers = data_watchers_.keys();
  for (auto getter: data_watchers) {
    getter->sendError<dbif::ObjectGoneError>();
  }
}

void FileBlobObject::description_reply(InfoGetter *getter) {
  getter->sendInfo<dbif::FileBlobDescriptionReply>(
    name(), comment(), 0, data().size(), 8, path()
  );
}

void SubBlobObject::description_reply(InfoGetter *getter) {
  getter->sendInfo<dbif::SubBlobDescriptionReply>(
    name(), comment(), 0, data().size(), 8, db()->handle(parent()->sharedFromThis())
  );
}

void ChunkObject::children_updated() {
  LocalObject::children_updated();
  parse_updated();
}

void ChunkObject::parse_updated() {
  calcParseReplyItems();
  for (InfoGetter *getter : parse_watchers_) {
    parse_reply(getter);
  }
}

void ChunkObject::calcParseReplyItems() {
  parseReplyItems_ = items_;

  QSet<PLocalObject> chunksInItems;
  for (auto &item : items_) {
    if (item.type != data::ChunkDataItem::SUBCHUNK) {
      continue;
    }
    if (auto localObjectHandle = item.ref[0].dynamicCast<LocalObjectHandle>()) {
      chunksInItems.insert(localObjectHandle->obj());
    }
  }

  for (PLocalObject obj : children()) {
    if (chunksInItems.contains(obj)) {
      continue;
    }
    if (auto chunkObj = obj.dynamicCast<ChunkObject>()) {
      parseReplyItems_.push_back(
          data::ChunkDataItem::subchunk(chunkObj->start_, chunkObj->end_,
                                        chunkObj->name(), db()->handle(obj)));
    } else if (auto subBlobObj = obj.dynamicCast<SubBlobObject>()) {
      parseReplyItems_.push_back(
          data::ChunkDataItem::subblob(subBlobObj->name(), db()->handle(obj)));
    }
  }
}

void ChunkObject::parse_reply(InfoGetter *getter) {
  getter->sendInfo<dbif::ChunkDataReply>(parseReplyItems_);
}

void ChunkObject::description_reply(InfoGetter *getter) {
  getter->sendInfo<dbif::ChunkDescriptionReply>(
    name(), comment(), db()->handle(blob_), db()->handle(parent_chunk_),
    start_, end_, chunk_type_
  );
}

void ChunkObject::remove_parse_watcher(InfoGetter *getter) {
  parse_watchers_.remove(getter);
}

void ChunkObject::getInfo(InfoGetter *getter, PInfoRequest req, bool once) {
  if (auto datareq = req.dynamicCast<dbif::ChunkDataRequest>()) {
    parse_reply(getter);
    if (!once) {
      parse_watchers_.insert(getter);
      auto shared_this = sharedFromThis();
      QObject::connect(getter, &QObject::destroyed, [shared_this, getter] () {
        shared_this.dynamicCast<ChunkObject>()->remove_parse_watcher(getter);
      });
    }
  } else {
    LocalObject::getInfo(getter, req, once);
  }
}

void ChunkObject::runMethod(MethodRunner *runner, PMethodRequest req) {
  if (auto chreq = req.dynamicCast<dbif::SetChunkBoundsRequest>()) {
    start_ = chreq->start;
    end_ = chreq->end;
    description_updated();
    runner->sendResult<dbif::NullReply>();
  } else if (auto preq = req.dynamicCast<dbif::SetChunkParseRequest>()) {
    start_ = preq->start;
    end_ = preq->end;
    items_ = preq->items;
    description_updated();
    parse_updated();
    runner->sendResult<dbif::NullReply>();
  } else if (auto blobreq = req.dynamicCast<dbif::ChunkCreateSubBlobRequest>()) {
    PLocalObject obj = SubBlobObject::create(this, blobreq->data, blobreq->name);
    runner->sendResult<dbif::CreatedReply>(db()->handle(obj));
  } else {
    LocalObject::runMethod(runner, req);
  }
}

void ChunkObject::killed() {
  LocalObject::killed();
  if (parent_chunk_)
    parent_chunk_->delChild(sharedFromThis());
  else
    blob_->delChild(sharedFromThis());

  auto parse_watchers = parse_watchers_;
  for (auto getter: parse_watchers) {
    getter->sendError<dbif::ObjectGoneError>();
  }
}

namespace {
class Register {
 public:
  Register() {
    qRegisterMetaType<veles::db::PLocalObject>("veles::db::PLocalObject");
  }
} _;
};

};
};
