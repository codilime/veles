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
#include <QSet>
#include <QThread>

#include "client/networkclient.h"
#include "data/nodeid.h"
#include "db/universe.h"
#include "dbif/universe.h"
#include "network/msgpackwrapper.h"

namespace veles {
namespace client {

class NCWrapper;

/*****************************************************************************/
/* NCObjectHandle */
/*****************************************************************************/

class NCObjectHandle : public dbif::ObjectHandleBase {
 public:
  explicit NCObjectHandle(NCWrapper* nc = nullptr,
                          const data::NodeID& id = *data::NodeID::getNilId(),
                          dbif::ObjectType type = dbif::ObjectType::CHUNK);
  data::NodeID id();

  dbif::InfoPromise* getInfo(const dbif::PInfoRequest& req) override;
  dbif::InfoPromise* subInfo(const dbif::PInfoRequest& req) override;
  dbif::MethodResultPromise* runMethod(
      const dbif::PMethodRequest& req) override;
  dbif::ObjectType type() const override;
  bool operator==(const NCObjectHandle& other);

 private:
  NCWrapper* nc_;
  data::NodeID id_;
  dbif::ObjectType type_;
};

/*****************************************************************************/
/* ChunkDataItemQuery */
/*****************************************************************************/

struct ChunkDataItemQuery {
  ChunkDataItemQuery(uint64_t children_qid, uint64_t data_items_qid,
                     const QPointer<dbif::InfoPromise>& promise,
                     const data::NodeID& id, bool sub);
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
  typedef std::map<data::NodeID, NCObjectHandle> ChildrenMap;
  typedef void (NCWrapper::*MessageHandler)(const msg_ptr&);

  explicit NCWrapper(NetworkClient* network_client, QObject* parent = nullptr);
  static dbif::ObjectType typeFromTags(
      const std::shared_ptr<std::unordered_set<std::shared_ptr<std::string>>>&
          tags);

  virtual dbif::InfoPromise* getInfo(const dbif::PInfoRequest& req,
                                     const data::NodeID& id);
  virtual dbif::InfoPromise* subInfo(const dbif::PInfoRequest& req,
                                     const data::NodeID& id);
  virtual dbif::InfoPromise* info(const dbif::PInfoRequest& req,
                                  const data::NodeID& id, bool sub);
  virtual dbif::MethodResultPromise* runMethod(const dbif::PMethodRequest& req,
                                               const data::NodeID& id);

  void handleGetListReplyMessage(const msg_ptr& message);
  void handleGetChildrenListReply(
      const std::shared_ptr<proto::MsgGetListReply>& reply,
      const QPointer<dbif::InfoPromise>& promise);
  void handleGetChunkDataItemsReply(
      const std::shared_ptr<proto::MsgGetListReply>& reply,
      const std::shared_ptr<ChunkDataItemQuery>& chunk_data_item_query);

  void handleRequestAckMessage(const msg_ptr& message);
  void handleGetReplyMessage(const msg_ptr& message);
  void handleGetBinDataReplyMessage(const msg_ptr& message);
  void handleGetDataReplyMessage(const msg_ptr& message);
  void handleQueryErrorMessage(const msg_ptr& message);

  dbif::InfoPromise* handleDescriptionRequest(const data::NodeID& id, bool sub);
  dbif::InfoPromise* handleChildrenRequest(const data::NodeID& id, bool sub);
  dbif::InfoPromise* handleParsersListRequest(bool sub);
  dbif::InfoPromise* handleBlobDataRequest(const data::NodeID& id,
                                           uint64_t start, uint64_t end,
                                           bool sub);
  dbif::InfoPromise* handleChunkDataRequest(const data::NodeID& id, bool sub);

  dbif::MethodResultPromise* handleRootCreateFileBlobFromDataRequest(
      const QSharedPointer<dbif::RootCreateFileBlobFromDataRequest>&
          create_file_blob_request);
  dbif::MethodResultPromise* handleChunkCreateRequest(
      const data::NodeID& id,
      const QSharedPointer<dbif::ChunkCreateRequest>& chunk_create_request);
  dbif::MethodResultPromise* handleChunkCreateSubBlobRequest(
      const data::NodeID& id,
      const QSharedPointer<dbif::ChunkCreateSubBlobRequest>&
          chunk_create_subblob_request);
  dbif::MethodResultPromise* handleDeleteRequest(const data::NodeID& id);
  dbif::MethodResultPromise* handleSetNameRequest(const data::NodeID& id,
                                                  const std::string& name);
  dbif::MethodResultPromise* handleSetCommentRequest(
      const data::NodeID& id, const std::string& comment);
  dbif::MethodResultPromise* handleChangeDataRequest(
      const data::NodeID& id,
      const QSharedPointer<dbif::ChangeDataRequest>& change_data_request);
  dbif::MethodResultPromise* handleSetChunkBoundsRequest(const data::NodeID& id,
                                                         int64_t pos_start,
                                                         int64_t pos_end);
  dbif::MethodResultPromise* handleSetChunkParseRequest(
      const data::NodeID& id,
      const QSharedPointer<dbif::SetChunkParseRequest>& chunk_parse_request);
  dbif::MethodResultPromise* handleBlobParseRequest(
      const data::NodeID& id,
      const QSharedPointer<dbif::BlobParseRequest>& blob_parse_request);

  std::shared_ptr<messages::MsgpackObject> chunkDataItemToMsgpack(
      const data::ChunkDataItem& item);
  data::ChunkDataItem msgpackToChunkDataItem(
      const std::shared_ptr<messages::MsgpackObject>& msgo);
  bool nodeToChunkDataItem(const std::shared_ptr<proto::Node>& node,
                           data::ChunkDataItem* out_chunk_data_item);

  void updateChildrenDataItems(
      ChunkDataItemQuery* query,
      const std::shared_ptr<std::vector<std::shared_ptr<proto::Node>>>&
          children,
      const std::shared_ptr<std::vector<std::shared_ptr<veles::data::NodeID>>>&
          gone);
  void updateDataItems(
      ChunkDataItemQuery* query,
      const std::shared_ptr<messages::MsgpackObject>& data_items);
  std::vector<data::ChunkDataItem> resultDataItems(
      const ChunkDataItemQuery& query);

 public slots:
  void updateConnectionStatus(
      NetworkClient::ConnectionStatus connection_status);
  void messageReceived(const msg_ptr& message);
  void newParser(const QString& id);
  void replyForParsersListRequest(const QPointer<dbif::InfoPromise>& promise);

 signals:
  void requestReplyForParsersListRequest(
      const QPointer<dbif::InfoPromise>& promise);
  void parse(const dbif::ObjectHandle& blob, db::MethodRunner* runner,
             const QString& parser_id, quint64 start = 0,
             const dbif::ObjectHandle& parent_chunk = dbif::ObjectHandle());

 private:
  dbif::InfoPromise* addInfoPromise(uint64_t qid, bool sub);
  dbif::MethodResultPromise* addMethodPromise(uint64_t qid);
  void wrongMessageType(const QString& name, const QString& expected_type);

  NetworkClient* nc_;

  std::unordered_map<std::string, MessageHandler> message_handlers_;
  std::unordered_map<uint64_t, QPointer<dbif::InfoPromise>> promises_;
  std::unordered_map<uint64_t, QPointer<dbif::MethodResultPromise>>
      method_promises_;
  std::unordered_set<uint64_t> subscriptions_;
  std::unordered_map<uint64_t, QPointer<dbif::InfoPromise>>
      root_children_promises_;
  std::unordered_map<uint64_t, QSharedPointer<NCObjectHandle>>
      created_objs_waiting_for_ack_;
  std::unordered_map<uint64_t, QSharedPointer<ChildrenMap>> children_maps_;

  bool detailed_debug_info_;

  QStringList parser_ids_;
  QList<QPointer<dbif::InfoPromise>> parser_promises_;
  std::map<uint64_t, std::shared_ptr<ChunkDataItemQuery>>
      chunk_data_item_queries_;
};

}  // namespace client
}  // namespace veles
