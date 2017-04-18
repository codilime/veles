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
/* NCWrapper - NetworkClients message handlers */
/*****************************************************************************/

void NCWrapper::handleGetListReplyMessage(
    std::shared_ptr<proto::MsgpackMsg> message) {
  // TODO
}

void NCWrapper::handleRequestAckMessage(
    std::shared_ptr<proto::MsgpackMsg> message) {
  // TODO
}

void getQStringAttr(std::shared_ptr<std::unordered_map<std::string,
    std::shared_ptr<messages::MsgpackObject>>> attr, std::string key,
    QString& val) {
  // TODO
}

template<class T> void getAttrSPtr(std::shared_ptr<std::unordered_map<
    std::string, std::shared_ptr<messages::MsgpackObject>>> attr,
    std::string key, T& val) {
  // TODO
}

template<class T> void getAttr(std::shared_ptr<std::unordered_map<std::string,
    std::shared_ptr<messages::MsgpackObject>>> attr, std::string key, T& val) {
  // TODO
}

void NCWrapper::handleGetReplyMessage(
    std::shared_ptr<proto::MsgpackMsg> message) {
  // TODO
}

void NCWrapper::handleGetBinDataReplyMessage(msg_ptr message) {
  // TODO
}

void NCWrapper::handleGetDataReplyMessage(msg_ptr message) {
  // TODO
}

void NCWrapper::handleQueryErrorMessage(msg_ptr message) {
  // TODO
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
