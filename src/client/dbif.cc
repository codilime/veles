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

#include "data/types.h"
#include "network/msgpackwrapper.h"
#include "client/dbif.h"

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
/* NCWrapper */
/*****************************************************************************/

NCWrapper::NCWrapper(NetworkClient* network_client, QObject* parent)
    : QObject(parent), nc_(network_client), detailed_debug_info_(true) {
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
    return handleParsersListRequest();
  } else if (auto blob_data_request
      = req.dynamicCast<dbif::BlobDataRequest>()) {
    return handleBlobDataRequest(id, blob_data_request->start,
        blob_data_request->end, sub);
  } else if (req.dynamicCast<dbif::ChunkDataRequest>()) {
    return handleChunkDataRequest(id, sub);
  }

  if (nc_->output()) {
    *nc_->output() << QString("NCWrapper: unknown InfoRequest.") << endl;
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
    return handleChunkCreateRequest(chunk_create_request);
  } else if (auto chunk_create_request
      = req.dynamicCast<dbif::ChunkCreateRequest>()) {
    return handleChunkCreateRequest(chunk_create_request);
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
    return handleChangeDataRequest();
  } else if (auto set_chunk_bounds_request
      = req.dynamicCast<dbif::SetChunkBoundsRequest>()) {
    return handleSetChunkBoundsRequest(
        id, set_chunk_bounds_request->start, set_chunk_bounds_request->end);
  } else if (auto set_chunk_parse_request
      = req.dynamicCast<dbif::SetChunkParseRequest>()) {
    return handleSetChunkParseRequest();
  } else if (auto blob_parse_request
      = req.dynamicCast<dbif::BlobParseRequest>()) {
    return handleBlobParseRequest();
  }

  if (nc_->output()) {
    *nc_->output() << QString("NCWrapper: unknown MethodRequest.") << endl;
  }

  return new dbif::MethodResultPromise;
}

/*****************************************************************************/
/* NCWrapper - NetworkClient message handlers */
/*****************************************************************************/

void NCWrapper::handleGetListReplyMessage(
    std::shared_ptr<proto::MsgpackMsg> message) {
  auto reply = std::dynamic_pointer_cast<proto::MsgGetListReply>(message);
  if (reply) {
    if (nc_->output() && detailed_debug_info_) {
      *nc_->output() << QString("NCWrapper: received MsgGetListReply "
          "(%1 object(s)).").arg(reply->objs->size()) << endl;
    }

    const auto promise_iter = promises_.find(reply->qid);
    if (promise_iter != promises_.end()) {
      std::vector<dbif::ObjectHandle> objects;
      for (auto child : *reply->objs) {
        objects.push_back(QSharedPointer<NCObjectHandle>::create(
            this, *child->id, typeFromTags(child->tags)));
      }
      emit promise_iter->second->gotInfo(
          QSharedPointer<dbif::ChildrenRequest::ReplyType>::create(objects));
    }
  }
}

void NCWrapper::handleRequestAckMessage(
    std::shared_ptr<proto::MsgpackMsg> message) {
  auto reply = std::dynamic_pointer_cast<proto::MsgRequestAck>(message);
  if (reply) {
    if (nc_->output() && detailed_debug_info_) {
      *nc_->output() << QString("NCWrapper: received MsgRequestAck.") << endl;
    }

    const auto promise_iter = method_promises_.find(reply->rid);
    if (promise_iter != method_promises_.end()) {
      auto id_iter = created_objs_waiting_for_ack_.find(reply->rid);
      if(id_iter != created_objs_waiting_for_ack_.end()) {
        emit promise_iter->second->gotResult(
            QSharedPointer<dbif::RootCreateFileBlobFromDataRequest::ReplyType>
            ::create(id_iter->second));
        created_objs_waiting_for_ack_.erase(id_iter);
      }
    }

    // TODO Do we have other cases when we need to do something special when
    // MsgRequestAck is received?
  }
}

void getQStringAttr(std::shared_ptr<std::unordered_map<std::string,
    std::shared_ptr<messages::MsgpackObject>>> attr, std::string key,
    QString& val) {
  auto iter = attr->find(key);
  if (iter != attr->end()) {
    std::shared_ptr<std::string> val_ptr;
    messages::fromMsgpackObject(iter->second, val_ptr);
    if (val_ptr) {
      val = QString::fromStdString(*val_ptr);
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

void NCWrapper::handleGetReplyMessage(
    std::shared_ptr<proto::MsgpackMsg> message) {
  auto reply = std::dynamic_pointer_cast<proto::MsgGetReply>(message);
  if (reply) {
    const auto promise_iter = promises_.find(reply->qid);
    if (promise_iter != promises_.end()) {
      QString name("");
      QString comment("");

      getQStringAttr(reply->obj->attr, std::string("name"), name);
      getQStringAttr(reply->obj->attr, std::string("comment"), comment);

      dbif::ObjectType node_type = typeFromTags(reply->obj->tags);

      if (nc_->output() && detailed_debug_info_) {
        *nc_->output() << QString("NCWrapper: received MsgGetReply - name: \""
            "%1\" comment: \"%2\".").arg(name).arg(comment)<< endl;
      }

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
          getQStringAttr(reply->obj->attr, std::string("path"), path);

          emit promise_iter->second->gotInfo(
              QSharedPointer<dbif::FileBlobDescriptionReply>
              ::create(name, comment, base, size, width, path));
        } else {
          auto parent = QSharedPointer<NCObjectHandle>::create(
              this, *reply->obj->parent, dbif::ObjectType::CHUNK); //FIXME

          emit promise_iter->second->gotInfo(
              QSharedPointer<dbif::SubBlobDescriptionReply>
              ::create(name, comment, base, size, width, parent));
        }
      } else if (node_type == dbif::ObjectType::CHUNK) {
        data::NodeID blob_id = *data::NodeID::getNilId();
        getAttrSPtr<data::NodeID>(reply->obj->attr, std::string("blob"),
            blob_id);
        auto blob = QSharedPointer<NCObjectHandle>::create(
            this, blob_id, dbif::ObjectType::FILE_BLOB);
        auto parent = QSharedPointer<NCObjectHandle>::create(
            this, *reply->obj->parent, dbif::ObjectType::CHUNK);
        if (*blob == *parent) {
          parent = QSharedPointer<NCObjectHandle>::create(
              this, *data::NodeID::getNilId(), dbif::ObjectType::CHUNK);
        }
        uint64_t start(0);
        uint64_t end(0);
        QString chunk_type;

        getAttr<uint64_t>(reply->obj->attr, "start", start);
        getAttr<uint64_t>(reply->obj->attr, "end", end);
        getQStringAttr(reply->obj->attr, "type", chunk_type);

        emit promise_iter->second->gotInfo(
            QSharedPointer<dbif::ChunkDescriptionReply>
            ::create(name, comment, blob, parent, start, end, chunk_type));
      } else {
        emit promise_iter->second->gotInfo(
            QSharedPointer<dbif::DescriptionReply>
            ::create(name, comment));
      }

      if (subscriptions_.find(reply->qid) == subscriptions_.end()) {
        promises_.erase(promise_iter);
      }
    }
  }
}

void NCWrapper::handleGetBinDataReplyMessage(msg_ptr message) {
  auto reply = std::dynamic_pointer_cast<proto::MsgGetBinDataReply>(message);
  if (reply) {
    if (nc_->output() && detailed_debug_info_) {
      *nc_->output() << QString("NCWrapper: received MsgGetBinDataReply.")
      << endl;
    }

    const auto promise_iter = promises_.find(reply->qid);
    if (promise_iter != promises_.end()) {
      data::BinData bindata(8, reply->data->size(), reply->data->data());
      emit promise_iter->second->gotInfo(
          QSharedPointer<dbif::BlobDataRequest::ReplyType>::create(bindata));
    }
  }
}

void NCWrapper::handleGetDataReplyMessage(msg_ptr message) {
  auto reply = std::dynamic_pointer_cast<proto::MsgGetDataReply>(message);
  if (reply) {
    if (nc_->output() && detailed_debug_info_) {
      *nc_->output() << QString("NCWrapper: received MsgGetDataReply.")
      << endl;
    }

    //TODO
  }
}

void NCWrapper::handleQueryErrorMessage(msg_ptr message) {
  auto reply = std::dynamic_pointer_cast<proto::MsgQueryError>(message);
  if (reply) {
    if (nc_->output() && detailed_debug_info_) {
      *nc_->output()
          << QString("NCWrapper: received MsgQueryError.") << endl
          << "    code: " << QString::fromStdString(reply->err->code)
          << "  msg: " << QString::fromStdString(reply->err->msg) << endl;
    }
  }
}

/*****************************************************************************/
/* NCWrapper - dbif "info" request handlers */
/*****************************************************************************/

dbif::InfoPromise* NCWrapper::handleDescriptionRequest(data::NodeID id,
    bool sub) {
  // TODO
  return nullptr;
}

dbif::InfoPromise* NCWrapper::handleChildrenRequest(data::NodeID id, bool sub) {
  // TODO
  return nullptr;
}

dbif::InfoPromise* NCWrapper::handleParsersListRequest() {
  // TODO
  return nullptr;
}

dbif::InfoPromise* NCWrapper::handleBlobDataRequest(
    data::NodeID id, uint64_t start, uint64_t end, bool sub) {
  // TODO
  return nullptr;
}

dbif::InfoPromise* NCWrapper::handleChunkDataRequest(data::NodeID id, bool sub) {
  // TODO
  return nullptr;
}

/*****************************************************************************/
/* NCWrapper - dbif "method" request handlers */
/*****************************************************************************/

dbif::MethodResultPromise* NCWrapper::handleRootCreateFileBlobFromDataRequest(
      QSharedPointer<dbif::RootCreateFileBlobFromDataRequest>
      create_file_blob_request) {
  // TODO
  return nullptr;
}

dbif::MethodResultPromise* NCWrapper::handleChunkCreateRequest(
      QSharedPointer<dbif::ChunkCreateRequest> chunk_create_request) {
  // TODO
  return nullptr;
}

dbif::MethodResultPromise* NCWrapper::handleChunkCreateSubBlobRequest(
    QSharedPointer<dbif::ChunkCreateSubBlobRequest>
    chunk_create_subblob_request) {
  // TODO
  return nullptr;
}

dbif::MethodResultPromise* NCWrapper::handleDeleteRequest(data::NodeID id) {
  // TODO
  return nullptr;
}

dbif::MethodResultPromise* NCWrapper::handleSetNameRequest(data::NodeID id,
    std::string name) {
  // TODO
  return nullptr;
}

dbif::MethodResultPromise* NCWrapper::handleSetCommentRequest(data::NodeID id,
    std::string comment) {
  // TODO
  return nullptr;
}

dbif::MethodResultPromise* NCWrapper::handleChangeDataRequest() {
  // TODO
  return nullptr;
}

dbif::MethodResultPromise* NCWrapper::handleSetChunkBoundsRequest(
    data::NodeID id, int64_t pos_start, int64_t pos_end) {
  // TODO
  return nullptr;
}

dbif::MethodResultPromise* NCWrapper::handleSetChunkParseRequest() {
  // TODO
  return nullptr;
}

dbif::MethodResultPromise* NCWrapper::handleBlobParseRequest() {
  // TODO
  return nullptr;
}

void NCWrapper::updateConnectionStatus(client::NetworkClient::ConnectionStatus
    connection_status) {
  if(connection_status ==
      client::NetworkClient::ConnectionStatus::Connected) {
    subscriptions_.clear();
    promises_.clear();
    method_promises_.clear();
    created_objs_waiting_for_ack_.clear();

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
        *nc_->output() << QString("NCWrapper: Sending MsgGetList message for"
            " the root node.") << endl;
      }
      subscriptions_.insert(entry.first);
      promises_[entry.first] = entry.second;
    }
  } else if(connection_status ==
      client::NetworkClient::ConnectionStatus::NotConnected) {

  }
}

void NCWrapper::messageReceived(msg_ptr message) {
  auto handler_iter = message_handlers_.find(message->object_type);
  if(handler_iter != message_handlers_.end()) {
    MessageHandler handler = handler_iter->second;
    (this->*handler)(message);
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

} // client
} // veles
