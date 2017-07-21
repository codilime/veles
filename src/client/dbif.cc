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

#include <memory>
#include <vector>

#include <QSharedPointer>

#include "db/getter.h"
#include "data/types.h"
#include "network/msgpackwrapper.h"
#include "network/msgpackobject.h"
#include "client/dbif.h"
#include "parser/utils.h"

namespace veles {
namespace client {

/*****************************************************************************/
/* NCObjectHandle */
/*****************************************************************************/

NCObjectHandle::NCObjectHandle(NCWrapper* nc, data::NodeID id,
    dbif::ObjectType type) : nc_(nc), id_(id), type_(type) {
}

data::NodeID NCObjectHandle::id() {
  return id_;
}

dbif::InfoPromise* NCObjectHandle::getInfo(dbif::PInfoRequest req) {
  return nc_->getInfo(req, id_);
}

dbif::InfoPromise* NCObjectHandle::subInfo(dbif::PInfoRequest req) {
  return nc_->subInfo(req, id_);
}

dbif::MethodResultPromise* NCObjectHandle::runMethod(
    dbif::PMethodRequest req) {
  return nc_->runMethod(req, id_);
}

dbif::ObjectType NCObjectHandle::type() const {
  return type_;
}

bool NCObjectHandle::operator==(NCObjectHandle& other) {
  return id_ == other.id_;
}

/*****************************************************************************/
/* ChunkDataItemQuery */
/*****************************************************************************/

ChunkDataItemQuery::ChunkDataItemQuery(
    uint64_t children_qid, uint64_t data_items_qid,
    QPointer<dbif::InfoPromise> promise, data::NodeID id, bool sub)
    : children_qid(children_qid), data_items_qid(data_items_qid),
    children_loaded(false), data_items_loaded(false), promise(promise),
    id(id), sub(sub) {}

bool ChunkDataItemQuery::ready() {
  return children_loaded && data_items_loaded;
}

/*****************************************************************************/
/* NCWrapper */
/*****************************************************************************/

NCWrapper::NCWrapper(NetworkClient* network_client, QObject* parent)
    : QObject(parent), nc_(network_client), detailed_debug_info_(false) {
  if (nc_) {
    message_handlers_["get_list_reply"]
        = &NCWrapper::handleGetListReplyMessage;
    message_handlers_["request_ack"]
        = &NCWrapper::handleRequestAckMessage;
    message_handlers_["get_reply"]
        = &NCWrapper::handleGetReplyMessage;
    message_handlers_["get_data_reply"]
        = &NCWrapper::handleGetDataReplyMessage;
    message_handlers_["get_bindata_reply"]
        = &NCWrapper::handleGetBinDataReplyMessage;
    message_handlers_["query_error"]
        = &NCWrapper::handleQueryErrorMessage;

    connect(nc_, &NetworkClient::connectionStatusChanged,
        this, &NCWrapper::updateConnectionStatus);
    connect(nc_, &NetworkClient::messageReceived,
        this, &NCWrapper::messageReceived);

    db::ParserWorker* parser_worker = new db::ParserWorker;
    qRegisterMetaType<veles::dbif::ObjectHandle>("dbif::ObjectHandle");
    QObject::connect(this, &NCWrapper::parse,
        parser_worker, &db::ParserWorker::parse);
    QObject::connect(parser_worker, &db::ParserWorker::newParser,
        this, &NCWrapper::newParser);
    QObject::connect(this, &NCWrapper::requestReplyForParsersListRequest,
        this, &NCWrapper::replyForParsersListRequest, Qt::QueuedConnection);
    for (auto parser : parser::createAllParsers()) {
      parser_worker->registerParser(parser);
    }
  }
}

dbif::ObjectType NCWrapper::typeFromTags(
      std::shared_ptr<std::unordered_set<std::shared_ptr<std::string>>> tags) {
  bool blob_stored = false;
  bool blob_file = false;
  bool chunk_stored = false;

  for (auto tag : *tags) {
    if(*tag == "blob.stored") {
      blob_stored = true;
    } else if(*tag == "blob.file") {
      blob_file = true;
    } else if(*tag == "chunk.stored") {
      chunk_stored = true;
    }
  }

  if (blob_stored) {
    if (blob_file) {
      return dbif::ObjectType::FILE_BLOB;
    } else {
      return dbif::ObjectType::SUB_BLOB;
    }
  } else if (chunk_stored) {
    return dbif::ObjectType::CHUNK;
  } else {
    return dbif::ObjectType::ROOT;
  }
}

/*****************************************************************************/
/* NCWrapper - dbif interface */
/*****************************************************************************/

dbif::InfoPromise* NCWrapper::getInfo(dbif::PInfoRequest req,
    data::NodeID id) {
  return info(req, id, false);
}

dbif::InfoPromise* NCWrapper::subInfo(dbif::PInfoRequest req,
    data::NodeID id) {
  return info(req, id, true);
}

dbif::InfoPromise* NCWrapper::info(dbif::PInfoRequest req, data::NodeID id,
    bool sub) {
  if (req.dynamicCast<dbif::DescriptionRequest>()) {
    return handleDescriptionRequest(id, sub);
  } else if (req.dynamicCast<dbif::ChildrenRequest>()) {
    return handleChildrenRequest(id, sub);
  } else if (req.dynamicCast<dbif::ParsersListRequest>()) {
    return handleParsersListRequest(sub);
  } else if (auto blob_data_request
      = req.dynamicCast<dbif::BlobDataRequest>()) {
    return handleBlobDataRequest(id, blob_data_request->start,
        blob_data_request->end, sub);
  } else if (req.dynamicCast<dbif::ChunkDataRequest>()) {
    return handleChunkDataRequest(id, sub);
  }

  if (nc_->output()) {
    *nc_->output() << "NCWrapper: unknown InfoRequest." << endl;
  }

  return new dbif::InfoPromise;
}

dbif::MethodResultPromise* NCWrapper::runMethod(
    dbif::PMethodRequest req, data::NodeID id) {
  if (auto create_file_blob_request
      = req.dynamicCast<dbif::RootCreateFileBlobFromDataRequest>()) {
    return handleRootCreateFileBlobFromDataRequest(create_file_blob_request);
  } else if (auto chunk_create_request
      = req.dynamicCast<dbif::ChunkCreateRequest>()) {
    return handleChunkCreateRequest(id, chunk_create_request);
  } else if (auto chunk_create_sub_blob_request
      = req.dynamicCast<dbif::ChunkCreateSubBlobRequest>()) {
    return handleChunkCreateSubBlobRequest(id, chunk_create_sub_blob_request);
  } else if (auto delete_request
      = req.dynamicCast<dbif::DeleteRequest>()) {
    return handleDeleteRequest(id);
  } else if (auto set_name_request
      = req.dynamicCast<dbif::SetNameRequest>()) {
    return handleSetNameRequest(id, set_name_request->name.toStdString());
  } else if (auto set_comment_request
      = req.dynamicCast<dbif::SetCommentRequest>()) {
    return handleSetCommentRequest(id,
        set_comment_request->comment.toStdString());
  } else if (auto change_data_request
      = req.dynamicCast<dbif::ChangeDataRequest>()) {
    return handleChangeDataRequest(id, change_data_request);
  } else if (auto set_chunk_bounds_request
      = req.dynamicCast<dbif::SetChunkBoundsRequest>()) {
    return handleSetChunkBoundsRequest(
        id, set_chunk_bounds_request->start, set_chunk_bounds_request->end);
  } else if (auto chunk_parse_request
      = req.dynamicCast<dbif::SetChunkParseRequest>()) {
    return handleSetChunkParseRequest(id, chunk_parse_request);
  } else if (auto blob_parse_request
      = req.dynamicCast<dbif::BlobParseRequest>()) {
    return handleBlobParseRequest(id, blob_parse_request);
  }

  if (nc_->output()) {
    *nc_->output() << "NCWrapper: unknown MethodRequest." << endl;
  }

  return new dbif::MethodResultPromise;
}

/*****************************************************************************/
/* NCWrapper - NetworkClient message handlers */
/*****************************************************************************/

void NCWrapper::handleGetListReplyMessage(msg_ptr message) {
  auto reply = std::dynamic_pointer_cast<proto::MsgGetListReply>(message);

  if (reply) {
    if (nc_->output() && detailed_debug_info_) {
      *nc_->output() << QString("NCWrapper: received MsgGetListReply "
          "(%1 object(s)).").arg(reply->objs->size()) << " qid = "
          << reply->qid << endl;
    }

    const auto promise_iter = promises_.find(reply->qid);
    if (promise_iter != promises_.end()  && promise_iter->second) {
      handleGetChildrenListReply(reply, promise_iter->second);
    } else {
      auto chunk_data_item_query_iter = chunk_data_item_queries_.find(
          reply->qid);
      if(chunk_data_item_query_iter != chunk_data_item_queries_.end()
          && chunk_data_item_query_iter->second->promise != nullptr) {
        handleGetChunkDataItemsReply(reply,
            chunk_data_item_query_iter->second);
      }
    }
  } else {
    wrongMessageType("get_list_reply", "MsgGetListReply");
  }
}

void NCWrapper::handleGetChildrenListReply(
      std::shared_ptr<proto::MsgGetListReply> reply,
      QPointer<dbif::InfoPromise> promise) {
  auto iter_subscriptions = subscriptions_.find(reply->qid);

  auto children_map_iter = children_maps_.find(reply->qid);
  QSharedPointer<ChildrenMap> children_map;
  if (children_map_iter == children_maps_.end()) {
    children_map = QSharedPointer<ChildrenMap>::create();
    if (iter_subscriptions != subscriptions_.end()) {
      children_maps_[reply->qid] = children_map;
    }
  } else {
    children_map = children_map_iter->second;
  }

  for (auto child : *reply->objs) {
    (*children_map)[*child->id] = NCObjectHandle(
        this, *child->id, typeFromTags(child->tags));
  }

  for (auto child_gone : *reply->gone) {
    children_map->erase(*child_gone);
  }

  std::vector<dbif::ObjectHandle> objects;

  for (auto child : *children_map) {
    objects.push_back(QSharedPointer<NCObjectHandle>::create(
        child.second));
  }

  emit promise->gotInfo(
      QSharedPointer<dbif::ChildrenRequest::ReplyType>::create(objects));
}

void NCWrapper::handleGetChunkDataItemsReply(
      std::shared_ptr<proto::MsgGetListReply> reply,
      std::shared_ptr<ChunkDataItemQuery> chunk_data_item_query) {

  if (chunk_data_item_query->promise) {
    updateChildrenDataItems(*chunk_data_item_query, reply->objs, reply->gone);

    if (chunk_data_item_query->ready()) {
      std::vector<data::ChunkDataItem> items = resultDataItems(
          *chunk_data_item_query);
      emit chunk_data_item_query->promise->gotInfo(
          QSharedPointer<dbif::ChunkDataReply>::create(items));

      if (!chunk_data_item_query->sub) {
        uint64_t qid1 = chunk_data_item_query->children_qid;
        uint64_t qid2 = chunk_data_item_query->data_items_qid;
        chunk_data_item_queries_.erase(qid1);
        chunk_data_item_queries_.erase(qid2);
      }
    }
  }
}

void NCWrapper::handleRequestAckMessage(msg_ptr message) {
  auto reply = std::dynamic_pointer_cast<proto::MsgRequestAck>(message);

  if (reply) {
    if (nc_->output() && detailed_debug_info_) {
      *nc_->output() << "NCWrapper: received MsgRequestAck. rid = "
          << reply->rid << endl;
    }

    const auto promise_iter = method_promises_.find(reply->rid);
    if (promise_iter != method_promises_.end() && promise_iter->second) {
      auto id_iter = created_objs_waiting_for_ack_.find(reply->rid);
      if(id_iter != created_objs_waiting_for_ack_.end()) {
        if (nc_->output() && detailed_debug_info_) {
          *nc_->output() << QString("NCWrapper: node with id \"%1\""
              " created.").arg(id_iter->second->id().toHexString()) << endl;
        }
        emit promise_iter->second->gotResult(
            QSharedPointer<dbif::CreatedReply>::create(id_iter->second));
        created_objs_waiting_for_ack_.erase(id_iter);
      } else {
        emit promise_iter->second->gotResult(
            QSharedPointer<dbif::NullReply>::create());
      }
    }
  } else {
    wrongMessageType("request_ack", "MsgRequestAck");
  }
}

void getQStringAttr(std::shared_ptr<std::unordered_map<std::string,
    std::shared_ptr<messages::MsgpackObject>>> attr, std::string key,
    QString* val_out) {
  auto iter = attr->find(key);
  if (iter != attr->end()) {
    auto val_ptr = iter->second->getString();
    if (val_ptr) {
      *val_out = QString::fromStdString(*val_ptr);
    }
  }
}

template<class T> void getAttrSPtr(std::shared_ptr<std::unordered_map<
    std::string, std::shared_ptr<messages::MsgpackObject>>> attr,
    std::string key, T& val) {
  auto iter = attr->find(key);
  if (iter != attr->end()) {
    std::shared_ptr<T> val_ptr;
    messages::fromMsgpackObject(iter->second, val_ptr);
    if (val_ptr) {
      val = *val_ptr;
    }
  }
}

template<class T> void getAttr(std::shared_ptr<std::unordered_map<std::string,
    std::shared_ptr<messages::MsgpackObject>>> attr, std::string key, T& val) {
  auto iter = attr->find(key);
  if (iter != attr->end()) {
    messages::fromMsgpackObject(iter->second, val);
  }
}

void NCWrapper::handleGetReplyMessage(msg_ptr message) {
  auto reply = std::dynamic_pointer_cast<proto::MsgGetReply>(message);

  if (reply) {
    const auto promise_iter = promises_.find(reply->qid);
    if (promise_iter != promises_.end() && promise_iter->second) {
      QString name("");
      QString comment("");

      getQStringAttr(reply->obj->attr, "name", &name);
      getQStringAttr(reply->obj->attr, "comment", &comment);

      dbif::ObjectType node_type = typeFromTags(reply->obj->tags);

      if (node_type == dbif::ObjectType::FILE_BLOB
          || node_type == dbif::ObjectType::SUB_BLOB) {

        uint64_t base(0);
        uint64_t size(0);
        uint64_t width(8);

        getAttr<uint64_t>(reply->obj->attr, "base", base);
        getAttr<uint64_t>(reply->obj->attr, "size", size);
        getAttr<uint64_t>(reply->obj->attr, "width", width);

        if (node_type == dbif::ObjectType::FILE_BLOB) {
          QString path;
          getQStringAttr(reply->obj->attr, "path", &path);

          if (nc_->output() && detailed_debug_info_) {
            *nc_->output() << QString("NCWrapper: received MsgGetReply "
                "(file blob) - name: \"%1\" comment: \"%2\";").arg(name)
                .arg(comment) << endl << QString("    base: %1; size: %2;"
                " width: %3; path: \"%4\".").arg(base).arg(size).arg(width)
                .arg(path) << endl;
          }

          emit promise_iter->second->gotInfo(
              QSharedPointer<dbif::FileBlobDescriptionReply>
              ::create(name, comment, base, size, width, path));
        } else {
          auto parent = QSharedPointer<NCObjectHandle>::create(
              this, *reply->obj->parent, dbif::ObjectType::CHUNK); //FIXME

          if (nc_->output() && detailed_debug_info_) {
            *nc_->output() << QString("NCWrapper: received MsgGetReply "
                "(sub blob) - name: \"%1\" comment: \"%2\";").arg(name)
                .arg(comment) << endl << QString("    base: %1; size: %2;"
                " width: %3.").arg(base).arg(size).arg(width) << endl;
          }

          emit promise_iter->second->gotInfo(
              QSharedPointer<dbif::SubBlobDescriptionReply>
              ::create(name, comment, base, size, width, parent));
        }
      } else if (node_type == dbif::ObjectType::CHUNK) {
        data::NodeID blob_id = *data::NodeID::getNilId();
        getAttrSPtr<data::NodeID>(reply->obj->attr, "blob",
            blob_id);
        auto blob = QSharedPointer<NCObjectHandle>::create(
            this, blob_id, dbif::ObjectType::FILE_BLOB); //FIXME
        auto parent = QSharedPointer<NCObjectHandle>::create(
            this, *reply->obj->parent, dbif::ObjectType::CHUNK);
        if (*blob == *parent) {
          parent = QSharedPointer<NCObjectHandle>::create(
              this, *data::NodeID::getNilId(), dbif::ObjectType::CHUNK);
        }
        uint64_t start(0);
        uint64_t end(0);
        QString chunk_type;

        if (reply->obj->pos_start.first) {
          start = reply->obj->pos_start.second;
        }

        if (reply->obj->pos_end.first) {
          end = reply->obj->pos_end.second;
        }

        getQStringAttr(reply->obj->attr, "type", &chunk_type);

        if (nc_->output() && detailed_debug_info_) {
          *nc_->output() << QString("NCWrapper: received MsgGetReply "
              "(chunk) - name: \"%1\" comment: \"%2\";").arg(name)
              .arg(comment) << endl << QString("    start: %1; end: %2;"
              " type: \"%3\".").arg(start).arg(end).arg(chunk_type) << endl;
        }

        emit promise_iter->second->gotInfo(
            QSharedPointer<dbif::ChunkDescriptionReply>
            ::create(name, comment, blob, parent, start, end, chunk_type));
      } else {
        if (nc_->output() && detailed_debug_info_) {
          *nc_->output() << QString("NCWrapper: received MsgGetReply - name: \""
              "%1\" comment: \"%2\".").arg(name).arg(comment)<< endl;
        }

        emit promise_iter->second->gotInfo(
            QSharedPointer<dbif::DescriptionReply>
            ::create(name, comment));
      }

      if (subscriptions_.find(reply->qid) == subscriptions_.end()) {
        promises_.erase(promise_iter);
      }
    }
  } else {
    wrongMessageType("get_reply", "MsgGetReply");
  }
}

void NCWrapper::handleGetBinDataReplyMessage(msg_ptr message) {
  auto reply = std::dynamic_pointer_cast<proto::MsgGetBinDataReply>(message);
  if (reply) {
    if (nc_->output() && detailed_debug_info_) {
      *nc_->output() << "NCWrapper: received MsgGetBinDataReply."
      << endl;
    }

    const auto promise_iter = promises_.find(reply->qid);
    if (promise_iter != promises_.end()  && promise_iter->second) {
      data::BinData bindata(8, reply->data->size(), reply->data->data());

      emit promise_iter->second->gotInfo(
          QSharedPointer<dbif::BlobDataRequest::ReplyType>::create(bindata));

      if (subscriptions_.find(reply->qid) == subscriptions_.end()) {
        promises_.erase(promise_iter);
      }
    }
  } else {
    wrongMessageType("get_bindata_reply", "MsgGetBinDataReply");
  }
}

void NCWrapper::handleGetDataReplyMessage(msg_ptr message) {
  auto reply = std::dynamic_pointer_cast<proto::MsgGetDataReply>(message);
  if (reply) {
    if (nc_->output() && detailed_debug_info_) {
      *nc_->output() << "NCWrapper: received MsgGetDataReply." << endl;
    }

    auto chunk_data_item_query_iter = chunk_data_item_queries_.find(
        reply->qid);
    if (chunk_data_item_query_iter != chunk_data_item_queries_.end()
        && chunk_data_item_query_iter->second->promise != nullptr) {
      ChunkDataItemQuery& chunk_data_item_query
          = *chunk_data_item_query_iter->second;
      if (reply->data.first) {
        auto data = reply->data.second;
        updateDataItems(chunk_data_item_query, data);
      } else {
        chunk_data_item_query.items.clear();
        chunk_data_item_query.data_items_loaded = true;
      }

      if (chunk_data_item_query.ready()) {
        std::vector<data::ChunkDataItem> items = resultDataItems(
            chunk_data_item_query);
        emit chunk_data_item_query.promise->gotInfo(
            QSharedPointer<dbif::ChunkDataReply>::create(items));

        if (!chunk_data_item_query.sub) {
          uint64_t qid1 = chunk_data_item_query.children_qid;
          uint64_t qid2 = chunk_data_item_query.data_items_qid;
          chunk_data_item_queries_.erase(qid1);
          chunk_data_item_queries_.erase(qid2);
        }
      }
    }
  } else {
    wrongMessageType("get_data_reply", "MsgGetDataReply");
  }
}

void NCWrapper::handleQueryErrorMessage(msg_ptr message) {
  auto reply = std::dynamic_pointer_cast<proto::MsgQueryError>(message);
  if (reply) {
    if (nc_->output() && detailed_debug_info_) {
      *nc_->output()
          << "NCWrapper: received MsgQueryError." << endl
          << "    code: " << QString::fromStdString(reply->err->code)
          << "  msg: " << QString::fromStdString(reply->err->msg) << endl;
    }
  } else {
    wrongMessageType("query_error", "MsgQueryError");
  }
}

/*****************************************************************************/
/* NCWrapper - dbif "info" request handlers */
/*****************************************************************************/

dbif::InfoPromise* NCWrapper::handleDescriptionRequest(data::NodeID id,
    bool sub) {
  uint64_t qid = nc_->nextQid();
  if(nc_->connectionStatus() == NetworkClient::ConnectionStatus::Connected) {
    if (nc_->output() && detailed_debug_info_) {
      *nc_->output() << QString("NCWrapper: Sending MsgGet message "
          "for node id \"%1\".").arg(id.toHexString()) << endl;
    }

    auto msg = std::make_shared<proto::MsgGet>(
        qid,
        std::make_shared<data::NodeID>(id),
        sub);
    nc_->sendMessage(msg);
  }

  return addInfoPromise(qid, sub);;
}

dbif::InfoPromise* NCWrapper::handleChildrenRequest(data::NodeID id, bool sub) {
  uint64_t qid = nc_->nextQid();
  if(nc_->connectionStatus() == NetworkClient::ConnectionStatus::Connected) {
    if (nc_->output() && detailed_debug_info_) {
      *nc_->output() << "NCWrapper: Sending MsgGetList message." << endl;
    }
    const auto null_pos = std::pair<bool, int64_t>(false, 0);
    auto msg = std::make_shared<proto::MsgGetList>(
        qid,
        std::make_shared<data::NodeID>(id),
        std::make_shared<std::unordered_set<std::shared_ptr<std::string>>>(),
        std::make_shared<proto::PosFilter>(
            null_pos, null_pos, null_pos, null_pos),
        sub);
    nc_->sendMessage(msg);
  }

  auto promise = addInfoPromise(qid, sub);
  if(sub && id == *data::NodeID::getRootNodeId()) {
    root_children_promises_[qid] = promise;
  }
  return promise;
}

dbif::InfoPromise* NCWrapper::handleParsersListRequest(bool sub) {
  auto promise = new dbif::InfoPromise;
  if (sub) {
    parser_promises_.push_back(promise);
  }
  emit requestReplyForParsersListRequest(QPointer<dbif::InfoPromise>(promise));
  return promise;
}

dbif::InfoPromise* NCWrapper::handleBlobDataRequest(
    data::NodeID id, uint64_t start, uint64_t end, bool sub) {
  uint64_t qid = nc_->nextQid();
  if(nc_->connectionStatus() == NetworkClient::ConnectionStatus::Connected) {
    if (nc_->output() && detailed_debug_info_) {
      *nc_->output() << "NCWrapper: Sending MsgGetBinData message." << endl;
    }
    const auto end_pos = std::pair<bool, int64_t>(true, end);
    auto msg = std::make_shared<proto::MsgGetBinData>(
        qid,
        std::make_shared<data::NodeID>(id),
        std::make_shared<std::string>("data"),
        start,
        end_pos,
        sub);
    nc_->sendMessage(msg);
  }

  return addInfoPromise(qid, sub);
}

dbif::InfoPromise* NCWrapper::handleChunkDataRequest(data::NodeID id, bool sub) {
  uint64_t qid_data = nc_->nextQid();
  uint64_t qid_children = nc_->nextQid();

  if(nc_->connectionStatus() == NetworkClient::ConnectionStatus::Connected) {
    if (nc_->output() && detailed_debug_info_) {
      *nc_->output() << QString("NCWrapper: Sending MsgGetData and MsgGetList"
          "message to get chunk data items of node \"%1\".")
          .arg(id.toHexString()) << endl;
    }

    auto msg_data = std::make_shared<proto::MsgGetData>(
        qid_data,
        std::make_shared<data::NodeID>(id),
        std::make_shared<std::string>("data_items"),
        sub);
    nc_->sendMessage(msg_data);

    const auto null_pos = std::pair<bool, int64_t>(false, 0);

    auto msg_children = std::make_shared<proto::MsgGetList>(
        qid_children,
        std::make_shared<data::NodeID>(id),
        std::make_shared<std::unordered_set<std::shared_ptr<std::string>>>(),
        std::make_shared<proto::PosFilter>(
            null_pos, null_pos, null_pos, null_pos),
        sub);
    nc_->sendMessage(msg_children);
  }

  auto query = std::make_shared<ChunkDataItemQuery>(qid_children, qid_data,
      new dbif::InfoPromise, id, sub);
  chunk_data_item_queries_[qid_data] = query;
  chunk_data_item_queries_[qid_children] = query;

  return query->promise;
}

/*****************************************************************************/
/* NCWrapper - dbif "method" request handlers */
/*****************************************************************************/

dbif::MethodResultPromise* NCWrapper::handleRootCreateFileBlobFromDataRequest(
      QSharedPointer<dbif::RootCreateFileBlobFromDataRequest>
      create_file_blob_request) {
  uint64_t qid = nc_->nextQid();

  if(nc_->connectionStatus() == NetworkClient::ConnectionStatus::Connected) {
    if (nc_->output() && detailed_debug_info_) {
      *nc_->output() << "NCWrapper: Sending a request to create a file"
          " blob (MsgTransaction)." << endl;
    }

    auto tags = std::make_shared<std::unordered_set<std::shared_ptr<
        std::string>>>();
    tags->insert(std::make_shared<std::string>("blob"));
    tags->insert(std::make_shared<std::string>("blob.stored"));
    tags->insert(std::make_shared<std::string>("blob.file"));

    auto attr = std::make_shared<std::unordered_map<
        std::string,std::shared_ptr<messages::MsgpackObject>>>();
    attr->insert(std::pair<std::string, std::shared_ptr<
        messages::MsgpackObject>>("path",
        std::make_shared<messages::MsgpackObject>(create_file_blob_request
        ->path.toStdString())));
    attr->insert(std::pair<std::string, std::shared_ptr<
        messages::MsgpackObject>>("width",
        std::make_shared<messages::MsgpackObject>(
        static_cast<uint64_t>(create_file_blob_request->data.width()))));
    attr->insert(std::pair<std::string, std::shared_ptr<
        messages::MsgpackObject>>("base",
        std::make_shared<messages::MsgpackObject>(
        static_cast<uint64_t>(0))));
    attr->insert(std::pair<std::string, std::shared_ptr<
        messages::MsgpackObject>>("size",
        std::make_shared<messages::MsgpackObject>(
        static_cast<uint64_t>(create_file_blob_request->data.size()))));

    auto data = std::make_shared<std::unordered_map<
            std::string,std::shared_ptr<messages::MsgpackObject>>>();

    auto bindata = std::make_shared<std::unordered_map<
        std::string,std::shared_ptr<std::vector<uint8_t>>>>();
    bindata->insert(std::pair<std::string,
        std::shared_ptr<std::vector<uint8_t>>>(
        "data", std::make_shared<std::vector<uint8_t>>(
        create_file_blob_request->data.rawData(),
        create_file_blob_request->data.rawData()
        + create_file_blob_request->data.size())));

    auto triggers = std::make_shared<std::unordered_set<
        std::shared_ptr<std::string>>>();

    auto new_id = std::make_shared<data::NodeID>();

    auto operation = std::make_shared<proto::OperationCreate>(
        new_id,
        data::NodeID::getRootNodeId(),
        std::pair<bool, int64_t>(true, 0),
        std::pair<bool, int64_t>(true, create_file_blob_request->data.size()),
        tags,
        attr,
        data,
        bindata,
        triggers
        );

    auto operations = std::make_shared<std::vector<std::shared_ptr<
        proto::Operation>>>();
    operations->push_back(operation);

    auto msg = std::make_shared<proto::MsgTransaction>(
        qid,
        std::make_shared<std::vector<std::shared_ptr<proto::Check>>>(),
        operations);
    nc_->sendMessage(msg);

    created_objs_waiting_for_ack_[qid] = QSharedPointer<NCObjectHandle>
        ::create(this, *new_id, dbif::ObjectType::FILE_BLOB);
  }

  return addMethodPromise(qid);
}

dbif::MethodResultPromise* NCWrapper::handleChunkCreateRequest(data::NodeID id,
      QSharedPointer<dbif::ChunkCreateRequest> chunk_create_request) {
  uint64_t qid = nc_->nextQid();

  if(nc_->connectionStatus() == NetworkClient::ConnectionStatus::Connected) {
    if (nc_->output() && detailed_debug_info_) {
      *nc_->output() << "NCWrapper: Sending a request to create a "
          "chunk (MsgTransaction)." << endl;
    }
    auto new_id = std::make_shared<data::NodeID>();

    auto parent_id = std::make_shared<data::NodeID>(id);
    if (chunk_create_request->parent_chunk) {
      auto parent_handle =
          chunk_create_request->parent_chunk.dynamicCast<NCObjectHandle>();
      if (parent_handle) {
        *parent_id = parent_handle->id();
      }
    }

    auto tags = std::make_shared<std::unordered_set<std::shared_ptr<
        std::string>>>();
    tags->insert(std::make_shared<std::string>("chunk"));
    tags->insert(std::make_shared<std::string>("chunk.stored"));

    auto attr = std::make_shared<std::unordered_map<
        std::string,std::shared_ptr<messages::MsgpackObject>>>();
    attr->insert(std::pair<std::string, std::shared_ptr<
        messages::MsgpackObject>>("blob",
        messages::toMsgpackObject(
        std::make_shared<data::NodeID>(id))));
    attr->insert(std::pair<std::string, std::shared_ptr<
        messages::MsgpackObject>>("name",
        std::make_shared<messages::MsgpackObject>(chunk_create_request
        ->name.toStdString())));
    attr->insert(std::pair<std::string, std::shared_ptr<
        messages::MsgpackObject>>("type",
        std::make_shared<messages::MsgpackObject>(chunk_create_request
        ->chunk_type.toStdString())));


    auto data = std::make_shared<std::unordered_map<
            std::string,std::shared_ptr<messages::MsgpackObject>>>();

    auto bindata = std::make_shared<std::unordered_map<
        std::string,std::shared_ptr<std::vector<uint8_t>>>>();

    auto triggers = std::make_shared<std::unordered_set<
        std::shared_ptr<std::string>>>();

    auto operation = std::make_shared<proto::OperationCreate>(
        new_id,
        parent_id,
        std::pair<bool, int64_t>(true, chunk_create_request->start),
        std::pair<bool, int64_t>(true, chunk_create_request->end),
        tags,
        attr,
        data,
        bindata,
        triggers
        );

    auto operations = std::make_shared<std::vector<std::shared_ptr<
        proto::Operation>>>();
    operations->push_back(operation);

    auto msg = std::make_shared<proto::MsgTransaction>(
        qid,
        std::make_shared<std::vector<std::shared_ptr<proto::Check>>>(),
        operations);
    nc_->sendMessage(msg);

    created_objs_waiting_for_ack_[qid] = QSharedPointer<NCObjectHandle>
        ::create(this, *new_id, dbif::ObjectType::CHUNK);
  }

  return addMethodPromise(qid);
}

dbif::MethodResultPromise* NCWrapper::handleChunkCreateSubBlobRequest(
    data::NodeID id, QSharedPointer<dbif::ChunkCreateSubBlobRequest>
    chunk_create_subblob_request) {
  uint64_t qid = nc_->nextQid();

  if(nc_->connectionStatus() == NetworkClient::ConnectionStatus::Connected) {
    if (nc_->output() && detailed_debug_info_) {
      *nc_->output() << QString("NCWrapper: Sending "
          "MsgTransaction message to handle ChunkCreateSubBlobRequest. "
          "qid = %1").arg(qid) << endl;
    }

    auto tags = std::make_shared<std::unordered_set<std::shared_ptr<
        std::string>>>();
    tags->insert(std::make_shared<std::string>("blob"));
    tags->insert(std::make_shared<std::string>("blob.stored"));

    auto attr = std::make_shared<std::unordered_map<
        std::string,std::shared_ptr<messages::MsgpackObject>>>();
    attr->insert(std::pair<std::string, std::shared_ptr<
        messages::MsgpackObject>>("width",
        std::make_shared<messages::MsgpackObject>(
        static_cast<uint64_t>(chunk_create_subblob_request->data.width()))));
    attr->insert(std::pair<std::string, std::shared_ptr<
        messages::MsgpackObject>>("base",
        std::make_shared<messages::MsgpackObject>(
        static_cast<uint64_t>(0))));
    attr->insert(std::pair<std::string, std::shared_ptr<
        messages::MsgpackObject>>("size",
        std::make_shared<messages::MsgpackObject>(
        static_cast<uint64_t>(chunk_create_subblob_request->data.size()))));
    attr->insert(std::pair<std::string, std::shared_ptr<
        messages::MsgpackObject>>("name",
        std::make_shared<messages::MsgpackObject>(chunk_create_subblob_request
        ->name.toStdString())));

    auto data = std::make_shared<std::unordered_map<
            std::string,std::shared_ptr<messages::MsgpackObject>>>();

    auto bindata = std::make_shared<std::unordered_map<
        std::string,std::shared_ptr<std::vector<uint8_t>>>>();
    bindata->insert(std::pair<std::string,
        std::shared_ptr<std::vector<uint8_t>>>(
        "data", std::make_shared<std::vector<uint8_t>>(
            chunk_create_subblob_request->data.rawData(),
        chunk_create_subblob_request->data.rawData()
        + chunk_create_subblob_request->data.size())));

    auto triggers = std::make_shared<std::unordered_set<
        std::shared_ptr<std::string>>>();

    auto new_id = std::make_shared<data::NodeID>();

    auto operation = std::make_shared<proto::OperationCreate>(
        new_id,
        std::make_shared<data::NodeID>(id),
        std::pair<bool, int64_t>(true, 0),
        std::pair<bool, int64_t>(true,
            chunk_create_subblob_request->data.size()),
        tags,
        attr,
        data,
        bindata,
        triggers
        );

    auto operations = std::make_shared<std::vector<std::shared_ptr<
        proto::Operation>>>();
    operations->push_back(operation);

    auto msg = std::make_shared<proto::MsgTransaction>(
        qid,
        std::make_shared<std::vector<std::shared_ptr<proto::Check>>>(),
        operations);
    nc_->sendMessage(msg);

    created_objs_waiting_for_ack_[qid] = QSharedPointer<NCObjectHandle>
        ::create(this, *new_id, dbif::ObjectType::SUB_BLOB);
  }

  return addMethodPromise(qid);
}

dbif::MethodResultPromise* NCWrapper::handleDeleteRequest(data::NodeID id) {
  uint64_t qid = nc_->nextQid();
  if(nc_->connectionStatus() == NetworkClient::ConnectionStatus::Connected) {
    if (nc_->output() && detailed_debug_info_) {
      *nc_->output() << "NCWrapper: Sending MsgDelete message." << endl;
    }
    auto msg = std::make_shared<proto::MsgDelete>(
        qid,
        std::make_shared<data::NodeID>(id));
    nc_->sendMessage(msg);
  }

  return addMethodPromise(qid);
}

dbif::MethodResultPromise* NCWrapper::handleSetNameRequest(data::NodeID id,
    std::string name) {
  uint64_t qid = nc_->nextQid();

  if(nc_->connectionStatus() == NetworkClient::ConnectionStatus::Connected) {
    if (nc_->output() && detailed_debug_info_) {
      *nc_->output() << "NCWrapper: Sending a request to set a node's"
          " name (MsgTransaction)." << endl;
    }

    auto operation = std::make_shared<proto::OperationSetAttr>(
        std::make_shared<data::NodeID>(id),
        std::make_shared<std::string>("name"),
        std::pair<bool, std::shared_ptr<messages::MsgpackObject>>(true,
            std::make_shared<messages::MsgpackObject>(name)));

    auto operations = std::make_shared<std::vector<std::shared_ptr<
        proto::Operation>>>();
    operations->push_back(operation);

    auto msg = std::make_shared<proto::MsgTransaction>(
        qid,
        std::make_shared<std::vector<std::shared_ptr<proto::Check>>>(),
        operations);
    nc_->sendMessage(msg);
  }

  return addMethodPromise(qid);
}

dbif::MethodResultPromise* NCWrapper::handleSetCommentRequest(data::NodeID id,
    std::string comment) {
  uint64_t qid = nc_->nextQid();

  if(nc_->connectionStatus() == NetworkClient::ConnectionStatus::Connected) {
    if (nc_->output() && detailed_debug_info_) {
      *nc_->output() << "NCWrapper: Sending a request to set a node's"
          " comment (MsgTransaction)." << endl;
    }

    auto operation = std::make_shared<proto::OperationSetAttr>(
        std::make_shared<data::NodeID>(id),
        std::make_shared<std::string>("comment"),
        std::pair<bool, std::shared_ptr<messages::MsgpackObject>>(true,
            std::make_shared<messages::MsgpackObject>(comment)));

    auto operations = std::make_shared<std::vector<std::shared_ptr<
        proto::Operation>>>();
    operations->push_back(operation);

    auto msg = std::make_shared<proto::MsgTransaction>(
        qid,
        std::make_shared<std::vector<std::shared_ptr<proto::Check>>>(),
        operations);
    nc_->sendMessage(msg);
  }

  return addMethodPromise(qid);
}

dbif::MethodResultPromise* NCWrapper::handleChangeDataRequest(data::NodeID id,
    QSharedPointer<dbif::ChangeDataRequest> change_data_request) {

  uint64_t qid = nc_->nextQid();

  if(nc_->connectionStatus() == NetworkClient::ConnectionStatus::Connected) {
    if (nc_->output() && detailed_debug_info_) {
      *nc_->output() << "NCWrapper: Sending a request to set a node's"
          " comment (MsgTransaction)." << endl;
    }

    auto bindata = std::make_shared<std::vector<uint8_t>>(
        change_data_request->data.rawData(),
        change_data_request->data.rawData()
        + change_data_request->data.size());

    auto operation = std::make_shared<proto::OperationSetBinData>(
        std::make_shared<data::NodeID>(id),
        std::make_shared<std::string>("data"),
        change_data_request->start,
        bindata,
        /*truncate=*/false
        );
    auto operations = std::make_shared<std::vector<std::shared_ptr<
        proto::Operation>>>();

    operations->push_back(operation);

    auto msg = std::make_shared<proto::MsgTransaction>(
        qid,
        std::make_shared<std::vector<std::shared_ptr<proto::Check>>>(),
        operations);

    nc_->sendMessage(msg);

   }

   return addMethodPromise(qid);
}

dbif::MethodResultPromise* NCWrapper::handleSetChunkBoundsRequest(
    data::NodeID id, int64_t pos_start, int64_t pos_end) {
  uint64_t qid = nc_->nextQid();

  if(nc_->connectionStatus() == NetworkClient::ConnectionStatus::Connected) {
    if (nc_->output() && detailed_debug_info_) {
      *nc_->output() << "NCWrapper: Sending a request to set chunk's"
          "bounds (MsgTransaction)." << endl;
    }

    auto operation = std::make_shared<proto::OperationSetPos>(
        std::make_shared<data::NodeID>(id),
        std::pair<bool, int64_t>(true, pos_start),
        std::pair<bool, int64_t>(true, pos_end));

    auto operations = std::make_shared<std::vector<std::shared_ptr<
        proto::Operation>>>();
    operations->push_back(operation);

    auto msg = std::make_shared<proto::MsgTransaction>(
        qid,
        std::make_shared<std::vector<std::shared_ptr<proto::Check>>>(),
        operations);
    nc_->sendMessage(msg);
  }

  return addMethodPromise(qid);
}

dbif::MethodResultPromise* NCWrapper::handleSetChunkParseRequest(
    data::NodeID id,
    QSharedPointer<dbif::SetChunkParseRequest> chunk_parse_request) {
  uint64_t qid = nc_->nextQid();

  if(nc_->connectionStatus() == NetworkClient::ConnectionStatus::Connected) {
    if (nc_->output() && detailed_debug_info_) {
      *nc_->output() << "NCWrapper: Sending a request to set chunks's "
          "data items (MsgTransaction). qid = " << qid << endl;
    }

    uint64_t qid_bounds = nc_->nextQid();

    auto operation = std::make_shared<proto::OperationSetPos>(
        std::make_shared<data::NodeID>(id),
        std::pair<bool, int64_t>(true, chunk_parse_request->start),
        std::pair<bool, int64_t>(true, chunk_parse_request->end));

    auto operations = std::make_shared<std::vector<std::shared_ptr<
        proto::Operation>>>();
    operations->push_back(operation);

    auto msg_bounds = std::make_shared<proto::MsgTransaction>(
        qid_bounds,
        std::make_shared<std::vector<std::shared_ptr<proto::Check>>>(),
        operations);
    nc_->sendMessage(msg_bounds);

    auto data_items = std::make_shared<
        std::vector<std::shared_ptr<messages::MsgpackObject>>>();

    for(auto item : chunk_parse_request->items) {
      data_items->push_back(chunkDataItemToMsgpack(item));
    }

    auto msg = std::make_shared<proto::MsgSetData>(
        qid,
        std::make_shared<data::NodeID>(id),
        std::make_shared<std::string>("data_items"),
        std::pair<bool, std::shared_ptr<messages::MsgpackObject>>(
            true, std::make_shared<messages::MsgpackObject>(data_items)));
    nc_->sendMessage(msg);
  }

  return addMethodPromise(qid);
}

dbif::MethodResultPromise* NCWrapper::handleBlobParseRequest(
    data::NodeID id,
    QSharedPointer<dbif::BlobParseRequest> blob_parse_request) {
  auto promise = new dbif::MethodResultPromise;
  auto runner = new db::MethodRunner;

  QObject::connect(runner, &db::MethodRunner::gotResult,
      promise, &db::MethodResultPromise::gotResult);
  QObject::connect(runner, &db::MethodRunner::gotError,
      promise, &db::MethodResultPromise::gotError);

  emit parse(QSharedPointer<NCObjectHandle>::create(
      this, id, dbif::ObjectType::FILE_BLOB), runner,
      blob_parse_request->parser_id, blob_parse_request->start,
      blob_parse_request->parent_chunk);

  return promise;
}

std::shared_ptr<messages::MsgpackObject> NCWrapper::chunkDataItemToMsgpack(
    const data::ChunkDataItem& item) {
  auto attr_item = std::make_shared<std::map<std::string,
      std::shared_ptr<messages::MsgpackObject>>>();
  attr_item->insert(std::pair<std::string, std::shared_ptr<
      messages::MsgpackObject>>("type", std::make_shared<
      messages::MsgpackObject>(uint64_t(item.type))));
  attr_item->insert(std::pair<std::string, std::shared_ptr<
      messages::MsgpackObject>>("start", std::make_shared<
      messages::MsgpackObject>(uint64_t(item.start))));
  attr_item->insert(std::pair<std::string, std::shared_ptr<
      messages::MsgpackObject>>("end", std::make_shared<
      messages::MsgpackObject>(uint64_t(item.end))));
  attr_item->insert(std::pair<std::string, std::shared_ptr<
      messages::MsgpackObject>>("num_elements", std::make_shared<
      messages::MsgpackObject>(uint64_t(item.num_elements))));
  attr_item->insert(std::pair<std::string, std::shared_ptr<
      messages::MsgpackObject>>("name", std::make_shared<
      messages::MsgpackObject>(item.name.toStdString())));
  auto bindata = std::make_shared<data::BinData>();
  *bindata = item.raw_value;
  attr_item->insert(std::pair<std::string, std::shared_ptr<
      messages::MsgpackObject>>("raw_value",
      messages::toMsgpackObject(bindata)));
  auto repacker = std::make_shared<data::Repacker>();
  *repacker = item.repack;
  attr_item->insert(std::pair<std::string, std::shared_ptr<
      messages::MsgpackObject>>("repack",
      messages::toMsgpackObject(repacker)));

  attr_item->insert(std::pair<std::string, std::shared_ptr<
      messages::MsgpackObject>>("field_type_float_complex", std::make_shared<
      messages::MsgpackObject>(item.high_type.float_complex)));
  attr_item->insert(std::pair<std::string, std::shared_ptr<
      messages::MsgpackObject>>("field_type_float_mode", std::make_shared<
      messages::MsgpackObject>(uint64_t(item.high_type.float_mode))));
  attr_item->insert(std::pair<std::string, std::shared_ptr<
      messages::MsgpackObject>>("field_type_mode", std::make_shared<
      messages::MsgpackObject>(uint64_t(item.high_type.mode))));
  attr_item->insert(std::pair<std::string, std::shared_ptr<
      messages::MsgpackObject>>("field_type_shift", std::make_shared<
      messages::MsgpackObject>(int64_t(item.high_type.shift))));
  attr_item->insert(std::pair<std::string, std::shared_ptr<
      messages::MsgpackObject>>("field_type_sign_mode", std::make_shared<
      messages::MsgpackObject>(uint64_t(item.high_type.sign_mode))));
  attr_item->insert(std::pair<std::string, std::shared_ptr<
      messages::MsgpackObject>>("field_type_string_encoding", std::make_shared<
      messages::MsgpackObject>(uint64_t(item.high_type.string_encoding))));
  attr_item->insert(std::pair<std::string, std::shared_ptr<
      messages::MsgpackObject>>("field_type_string_mode", std::make_shared<
      messages::MsgpackObject>(uint64_t(item.high_type.string_mode))));
  attr_item->insert(std::pair<std::string, std::shared_ptr<
      messages::MsgpackObject>>("field_type_name", std::make_shared<
      messages::MsgpackObject>(std::make_shared<std::string>(
          item.high_type.type_name.toStdString()))));

  auto refs_ptr = std::make_shared<std::vector<std::shared_ptr<
      messages::MsgpackObject>>>();
  for (auto ref : item.ref) {
    auto handle = ref.dynamicCast<NCObjectHandle>();
    if (handle) {
      refs_ptr->push_back(messages::toMsgpackObject(
          std::make_shared<data::NodeID>(handle->id())));
    }
  }
  attr_item->insert(std::pair<std::string, std::shared_ptr<
      messages::MsgpackObject>>("refs", std::make_shared<
      messages::MsgpackObject>(refs_ptr)));

  return std::make_shared<messages::MsgpackObject>(attr_item);
}

template<class T> void getFieldFromMap(std::shared_ptr<std::map<std::string,
    std::shared_ptr<messages::MsgpackObject>>> fields, std::string key, T& val) {
  auto iter = fields->find(key);
  if (iter != fields->end()) {
    messages::fromMsgpackObject(iter->second, val);
  }
}

data::ChunkDataItem NCWrapper::msgpackToChunkDataItem(
    std::shared_ptr<messages::MsgpackObject> msgo) {
  data::ChunkDataItem item;
  auto attr_map = msgo->getMap();
  if (attr_map) {
    uint64_t type_uint(data::ChunkDataItem::NONE);
    getFieldFromMap(attr_map, "type", type_uint);
    item.type = data::ChunkDataItem::ChunkDataItemType(type_uint);

    uint64_t start(0);
    getFieldFromMap(attr_map, "start", start);
    item.start = start;

    uint64_t end(0);
    getFieldFromMap(attr_map, "end", end);
    item.end = end;

    uint64_t num_elements(0);
    getFieldFromMap(attr_map, "num_elements", num_elements);
    item.num_elements = num_elements;

    auto name = std::make_shared<std::string>("[not set]");
    getFieldFromMap(attr_map, "name", name);
    item.name = QString::fromStdString(*name);

    auto bindata = std::make_shared<data::BinData>();
    getFieldFromMap(attr_map, "raw_value", bindata);
    item.raw_value = *bindata;

    auto repacker = std::make_shared<data::Repacker>();
    getFieldFromMap(attr_map, "repack", repacker);
    item.repack = *repacker;

    bool float_complex = false;
    getFieldFromMap(attr_map, "float_complex", float_complex);
    item.high_type.float_complex = float_complex;

    uint64_t float_mode(data::FieldHighType::IEEE754_SINGLE);
    getFieldFromMap(attr_map, "float_mode", float_mode);
    item.high_type.float_mode = data::FieldHighType::FieldFloatMode(float_mode);

    uint64_t mode(data::FieldHighType::NONE);
    getFieldFromMap(attr_map, "mode", mode);
    item.high_type.mode = data::FieldHighType::FieldHighMode(mode);

    int64_t shift(0);
    getFieldFromMap(attr_map, "shift", shift);
    item.high_type.shift = shift;

    uint64_t sign_mode(data::FieldHighType::SIGNED);
    getFieldFromMap(attr_map, "sign_mode", sign_mode);
    item.high_type.sign_mode = data::FieldHighType::FieldSignMode(sign_mode);

    uint64_t string_encoding(data::FieldHighType::ENC_RAW);
    getFieldFromMap(attr_map, "string_encoding", string_encoding);
    item.high_type.string_encoding = data::FieldHighType::FieldStringEncoding(
        string_encoding);

    uint64_t string_mode(data::FieldHighType::STRING_RAW);
    getFieldFromMap(attr_map, "string_mode", string_mode);
    item.high_type.string_mode = data::FieldHighType::FieldStringMode(
        string_mode);

    auto type_name = std::make_shared<std::string>("[unknown]");
    getFieldFromMap(attr_map, "type_name", type_name);
    item.high_type.type_name = QString::fromStdString(*type_name);

    std::shared_ptr<std::vector<std::shared_ptr<messages::MsgpackObject>>>
        refs;
    getFieldFromMap(attr_map, "refs", refs);
    if (refs) {
      for (auto ref_ptr : *refs) {
        std::shared_ptr<data::NodeID> id_ptr;
        fromMsgpackObject(ref_ptr, id_ptr);
        if (id_ptr) {
          item.ref.push_back(QSharedPointer<NCObjectHandle>::create(
              this, *id_ptr, item.type == data::ChunkDataItem::SUBBLOB
              ? dbif::ObjectType::SUB_BLOB : dbif::ObjectType::CHUNK));
        }
      }
    }
  }

  return item;
}

bool NCWrapper::nodeToChunkDataItem(
      std::shared_ptr<proto::Node> node,
      data::ChunkDataItem& out_chunk_data_item) {
  QString name("[not set]");
  getQStringAttr(node->attr, "name", &name);
  dbif::ObjectType type = typeFromTags(node->tags);
  if (type == dbif::ObjectType::CHUNK) {
    out_chunk_data_item = data::ChunkDataItem::subchunk(
        node->pos_start.second, node->pos_end.second, name,
        QSharedPointer<NCObjectHandle>::create(
        this, *node->id, dbif::ObjectType::CHUNK));
    return true;
  } else if (type == dbif::ObjectType::SUB_BLOB) {
    out_chunk_data_item = data::ChunkDataItem::subblob(
        name,
        QSharedPointer<NCObjectHandle>::create(
        this, *node->id, dbif::ObjectType::SUB_BLOB));
    return true;
  }

  return false;
}

void NCWrapper::updateChildrenDataItems(ChunkDataItemQuery& query,
    std::shared_ptr<std::vector<std::shared_ptr<proto::Node>>> children,
    std::shared_ptr<std::vector<std::shared_ptr<veles::data::NodeID>>> gone)
    {
  for (auto child : *children) {
    query.children_map[*child->id] = child;
  }

  for (auto child_gone : *gone) {
    query.children_map.erase(*child_gone);
  }

  query.children_loaded = true;
}

void NCWrapper::updateDataItems(ChunkDataItemQuery& query,
    std::shared_ptr<messages::MsgpackObject> data_items) {
  query.items.clear();

  std::shared_ptr<std::vector<std::shared_ptr<messages::MsgpackObject>>>
      data_items_vector;
  fromMsgpackObject(data_items, data_items_vector);
  for (auto item_ptr : *data_items_vector) {
    auto chunk_data_item = msgpackToChunkDataItem(item_ptr);

    query.items.push_back(chunk_data_item);
  }

  query.data_items_loaded = true;
}

std::vector<data::ChunkDataItem> NCWrapper::resultDataItems(
    ChunkDataItemQuery& query) {
  std::vector<data::ChunkDataItem> chunk_data_items;

  for (auto child : query.children_map) {
    data::ChunkDataItem child_data_item;
    if (NCWrapper::nodeToChunkDataItem(child.second, child_data_item)) {
      chunk_data_items.push_back(child_data_item);
    }
  }

  for (auto item : query.items) {
    if ((item.type == data::ChunkDataItem::SUBBLOB
        || item.type == data::ChunkDataItem::SUBCHUNK)
        && item.ref.size() > 0) {
      auto handle = item.ref[0].dynamicCast<NCObjectHandle>();
      if(handle && query.children_map.find(handle->id())
          != query.children_map.end()) {
        continue;
      }
    }

    chunk_data_items.push_back(item);
  }

  return chunk_data_items;
}

void NCWrapper::updateConnectionStatus(client::NetworkClient::ConnectionStatus
    connection_status) {
  if(connection_status ==
      client::NetworkClient::ConnectionStatus::Connected) {
    subscriptions_.clear();
    promises_.clear();
    method_promises_.clear();
    created_objs_waiting_for_ack_.clear();
    children_maps_.clear();

    const auto null_pos = std::pair<bool, int64_t>(false, 0);

    for (auto entry : root_children_promises_) {
      auto msg = std::make_shared<proto::MsgGetList>(
          entry.first,
          data::NodeID::getRootNodeId(),
          std::make_shared<std::unordered_set<std::shared_ptr<std::string>>>(),
          std::make_shared<proto::PosFilter>(
              null_pos, null_pos, null_pos, null_pos),
          true);
      nc_->sendMessage(msg);
      if (nc_->output() && detailed_debug_info_) {
        *nc_->output() << "NCWrapper: Sending MsgGetList message for"
            " the root node." << endl;
      }
      subscriptions_.insert(entry.first);
      promises_[entry.first] = entry.second;
    }
  } else if(connection_status ==
      client::NetworkClient::ConnectionStatus::NotConnected) {
    for (auto entry : root_children_promises_) {
      if (entry.second) {
        std::vector<dbif::ObjectHandle> objects;
        emit entry.second->gotInfo(
            QSharedPointer<dbif::ChildrenRequest::ReplyType>::create(objects));
      }
    }
  }
}

void NCWrapper::messageReceived(msg_ptr message) {
  auto handler_iter = message_handlers_.find(message->object_type);
  if(handler_iter != message_handlers_.end()) {
    MessageHandler handler = handler_iter->second;
    (this->*handler)(message);
  }
}

void NCWrapper::newParser(QString id) {
  parser_ids_.push_back(id);

  for (auto promise : parser_promises_) {
    replyForParsersListRequest(promise);
  }
}

void NCWrapper::replyForParsersListRequest(
    QPointer<dbif::InfoPromise> promise) {
  if (promise) {
    emit promise->gotInfo(QSharedPointer<dbif::ParsersListRequest::ReplyType>
        ::create(parser_ids_));
  }
}

dbif::InfoPromise* NCWrapper::addInfoPromise(uint64_t qid, bool sub) {
  auto promise = new dbif::InfoPromise;
  promises_[qid] = promise;
  if (sub) {
    subscriptions_.insert(qid);
  }

  return promise;
}

dbif::MethodResultPromise* NCWrapper::addMethodPromise(uint64_t qid) {
  auto promise = new dbif::MethodResultPromise;
  method_promises_[qid] = promise;

  return promise;
}

void NCWrapper::wrongMessageType(QString name, QString expected_type) {
  if (nc_->output()) {
    *nc_->output() << QString("NCWrapper: error - declared message type is "
        "\"%1\", but it's actually not a %2.").arg(name).arg(expected_type)
        << endl;
  }
}

} // namespace client
} // namespace veles
