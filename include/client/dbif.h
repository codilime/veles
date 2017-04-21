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
#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include <QObject>
#include <QPointer>

#include "network/msgpackwrapper.h"
#include "client/networkclient.h"
#include "dbif/universe.h"
#include "data/nodeid.h"

namespace veles {
namespace client {

class NCWrapper;

/*****************************************************************************/
/* NCObjectHandle */
/*****************************************************************************/

class NCObjectHandle : public dbif::ObjectHandleBase {
 public:
  NCObjectHandle(NCWrapper* nc = nullptr,
      data::NodeID id = *data::NodeID::getNilId(),
      dbif::ObjectType type = dbif::ObjectType::CHUNK);
  data::NodeID id();

  virtual dbif::InfoPromise *getInfo(dbif::PInfoRequest req);
  virtual dbif::InfoPromise *subInfo(dbif::PInfoRequest req);
  virtual dbif::MethodResultPromise *runMethod(dbif::PMethodRequest req);
  virtual dbif::ObjectType type() const;
  bool operator==(NCObjectHandle& other);

 private:
  NCWrapper* nc_;
  data::NodeID id_;
  dbif::ObjectType type_;
};

/*****************************************************************************/
/* NCWrapper */
/*****************************************************************************/

class NCWrapper : public QObject {
  Q_OBJECT

 public:
  typedef void (NCWrapper::*MessageHandler)(msg_ptr);

  NCWrapper(NetworkClient* network_client, QObject* parent = nullptr);
  static dbif::ObjectType typeFromTags(
      std::shared_ptr<std::unordered_set<std::shared_ptr<std::string>>> tags);

  virtual dbif::InfoPromise* getInfo(dbif::PInfoRequest req, data::NodeID id);
  virtual dbif::InfoPromise* subInfo(dbif::PInfoRequest req, data::NodeID id);
  virtual dbif::InfoPromise* info(dbif::PInfoRequest req, data::NodeID id,
      bool sub);
  virtual dbif::MethodResultPromise *runMethod(dbif::PMethodRequest req,
      data::NodeID id);

  void handleGetListReplyMessage(msg_ptr message);
  void handleRequestAckMessage(msg_ptr message);
  void handleGetReplyMessage(msg_ptr message);
  void handleGetBinDataReplyMessage(msg_ptr message);
  void handleGetDataReplyMessage(msg_ptr message);
  void handleQueryErrorMessage(msg_ptr message);

  dbif::InfoPromise* handleDescriptionRequest(data::NodeID id, bool sub);
  dbif::InfoPromise* handleChildrenRequest(data::NodeID id, bool sub);
  dbif::InfoPromise* handleParsersListRequest();
  dbif::InfoPromise* handleBlobDataRequest(
      data::NodeID id, uint64_t start, uint64_t end, bool sub);
  dbif::InfoPromise* handleChunkDataRequest(data::NodeID id, bool sub);

  dbif::MethodResultPromise* handleRootCreateFileBlobFromDataRequest(
      QSharedPointer<dbif::RootCreateFileBlobFromDataRequest>
      create_file_blob_request);
  dbif::MethodResultPromise* handleChunkCreateRequest(data::NodeID id,
      QSharedPointer<dbif::ChunkCreateRequest> chunk_create_request);
  dbif::MethodResultPromise* handleChunkCreateSubBlobRequest(data::NodeID id,
      QSharedPointer<dbif::ChunkCreateSubBlobRequest>
      chunk_create_subblob_request);
  dbif::MethodResultPromise* handleDeleteRequest(data::NodeID id);
  dbif::MethodResultPromise* handleSetNameRequest(data::NodeID id,
      std::string name);
  dbif::MethodResultPromise* handleSetCommentRequest(data::NodeID id,
      std::string comment);
  dbif::MethodResultPromise* handleChangeDataRequest();
  dbif::MethodResultPromise* handleSetChunkBoundsRequest(
      data::NodeID id, int64_t pos_start, int64_t pos_end);
  dbif::MethodResultPromise* handleSetChunkParseRequest();
  dbif::MethodResultPromise* handleBlobParseRequest();

 public slots:
  void updateConnectionStatus(NetworkClient::ConnectionStatus
      connection_status);
  void messageReceived(msg_ptr message);

 private:
  dbif::InfoPromise* addInfoPromise(uint64_t qid, bool sub);
  dbif::MethodResultPromise* addMethodPromise(uint64_t qid);
  NetworkClient* nc_;

  std::unordered_map<std::string, MessageHandler> message_handlers_;
  std::unordered_map<uint64_t, QPointer<dbif::InfoPromise>> promises_;
  std::unordered_map<uint64_t, QPointer<dbif::MethodResultPromise>>
      method_promises_;
  std::unordered_set<uint64_t> subscriptions_;
  std::unordered_map<uint64_t, QPointer<dbif::InfoPromise>>
      root_children_promises_;
  std::unordered_map<uint64_t, QSharedPointer<NCObjectHandle>> created_objs_waiting_for_ack_;
  typedef std::map<data::NodeID, NCObjectHandle> ChildrenMap;
  std::unordered_map<uint64_t, QSharedPointer<ChildrenMap>> children_maps_;

  bool detailed_debug_info_;
};

} // client
} // veles
