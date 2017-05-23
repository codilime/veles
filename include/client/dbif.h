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
#include <QThread>
#include <QSet>

#include "network/msgpackwrapper.h"
#include "client/networkclient.h"
#include "db/universe.h"
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
/* ChunkDataItemQuery */
/*****************************************************************************/

struct ChunkDataItemQuery{
  ChunkDataItemQuery(uint64_t children_qid, uint64_t data_items_qid,
      QPointer<dbif::InfoPromise> promise, data::NodeID id, bool sub);
  typedef std::map<data::NodeID, std::shared_ptr<proto::Node>> ChildrenMap;

  bool ready();

  uint64_t children_qid;
  uint64_t data_items_qid;
  bool children_loaded;
  bool data_items_loaded;
  QPointer<dbif::InfoPromise> promise;
  data::NodeID id;
  bool sub;

  ChildrenMap children_map;
  std::vector<data::ChunkDataItem> items;
};

/*****************************************************************************/
/* NCWrapper */
/*****************************************************************************/

class NCWrapper : public QObject {
  Q_OBJECT

 public:
  static NCWrapper* instance() {return ncw_;}
  typedef std::map<data::NodeID, NCObjectHandle> ChildrenMap;
  typedef void (NCWrapper::*MessageHandler)(msg_ptr);

  NCWrapper(NetworkClient* network_client, QObject* parent = nullptr);
  static dbif::ObjectType typeFromTags(
      std::shared_ptr<std::unordered_set<std::shared_ptr<std::string>>> tags);
  static dbif::ObjectType typeFromTags(
      const std::unordered_set<std::string>& tags);

  virtual dbif::InfoPromise* getInfo(dbif::PInfoRequest req, data::NodeID id);
  virtual dbif::InfoPromise* subInfo(dbif::PInfoRequest req, data::NodeID id);
  virtual dbif::InfoPromise* info(dbif::PInfoRequest req, data::NodeID id,
      bool sub);
  virtual dbif::MethodResultPromise *runMethod(dbif::PMethodRequest req,
      data::NodeID id);

  void handleGetListReplyMessage(msg_ptr message);
  void handleGetChildrenListReply(
      std::shared_ptr<proto::MsgGetListReply> reply,
      QPointer<dbif::InfoPromise> promise);
  void handleGetChunkDataItemsReply(
      std::shared_ptr<proto::MsgGetListReply> reply,
      std::shared_ptr<ChunkDataItemQuery> chunk_data_item_query);

  void handleRequestAckMessage(msg_ptr message);
  void handleGetReplyMessage(msg_ptr message);
  void handleGetBinDataReplyMessage(msg_ptr message);
  void handleGetDataReplyMessage(msg_ptr message);
  void handleQueryErrorMessage(msg_ptr message);

  dbif::InfoPromise* handleDescriptionRequest(data::NodeID id, bool sub);
  dbif::InfoPromise* handleChildrenRequest(data::NodeID id, bool sub);
  dbif::InfoPromise* handleParsersListRequest(bool sub);
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
  dbif::MethodResultPromise* handleSetChunkParseRequest(data::NodeID id,
      QSharedPointer<dbif::SetChunkParseRequest> chunk_parse_request);
  dbif::MethodResultPromise* handleBlobParseRequest(data::NodeID id,
      QSharedPointer<dbif::BlobParseRequest> blob_parse_request);

  std::shared_ptr<messages::MsgpackObject> chunkDataItemToMsgpack(
      const data::ChunkDataItem& item);
  data::ChunkDataItem msgpackToChunkDataItem(
      std::shared_ptr<messages::MsgpackObject> msgo);
  bool nodeToChunkDataItem(
      std::shared_ptr<proto::Node> node,
      data::ChunkDataItem& out_chunk_data_item);

  void updateChildrenDataItems(ChunkDataItemQuery& query,
      std::shared_ptr<std::vector<std::shared_ptr<proto::Node>>> children,
      std::shared_ptr<std::vector<std::shared_ptr<veles::data::NodeID>>> gone);
  void updateDataItems(ChunkDataItemQuery& query,
      std::shared_ptr<messages::MsgpackObject> data_items);
  std::vector<data::ChunkDataItem> resultDataItems(ChunkDataItemQuery& query);

 public slots:
  void updateConnectionStatus(NetworkClient::ConnectionStatus
      connection_status);
  void messageReceived(msg_ptr message);
  void newParser(QString id);
  void replyForParsersListRequest(QPointer<dbif::InfoPromise> promise);

 signals:
  void requestReplyForParsersListRequest(QPointer<dbif::InfoPromise> promise);
  void parse( dbif::ObjectHandle blob, db::MethodRunner* runner,
      QString parser_id, quint64 start = 0,
      dbif::ObjectHandle parent_chunk = dbif::ObjectHandle());

 private:
  dbif::InfoPromise* addInfoPromise(uint64_t qid, bool sub);
  dbif::MethodResultPromise* addMethodPromise(uint64_t qid);
  void wrongMessageType(QString name, QString expected_type);
  NetworkClient* nc_;

  std::unordered_map<std::string, MessageHandler> message_handlers_;
  std::unordered_map<uint64_t, QPointer<dbif::InfoPromise>> promises_;
  std::unordered_map<uint64_t, QPointer<dbif::MethodResultPromise>>
      method_promises_;
  std::unordered_set<uint64_t> subscriptions_;
  std::unordered_map<uint64_t, QPointer<dbif::InfoPromise>>
      root_children_promises_;
  std::unordered_map<uint64_t, QSharedPointer<NCObjectHandle>> created_objs_waiting_for_ack_;
  std::unordered_map<uint64_t, QSharedPointer<ChildrenMap>> children_maps_;

  bool detailed_debug_info_;

  QStringList parser_ids_;
  QList<QPointer<dbif::InfoPromise>> parser_promises_;
  std::map<uint64_t, std::shared_ptr<ChunkDataItemQuery>>
      chunk_data_item_queries_;

  static NCWrapper* ncw_;
};

} // namespace client
} // namespace veles
