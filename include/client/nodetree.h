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

#include <list>

#include <QObject>

#include "data/nodeid.h"
#include "node.h"
#include "networkclient.h"

namespace veles {
namespace client {

typedef std::shared_ptr<proto::MsgpackMsg> msg_ptr;

/*****************************************************************************/
/* NodeTree */
/*****************************************************************************/

class NodeTree : public QObject {
  Q_OBJECT

 public:
  NodeTree(NetworkClient* network_client, QObject* parent = nullptr);
  virtual ~NodeTree();

  typedef void (NodeTree::*MessageHandler)(msg_ptr);

  Node* node(data::NodeID id);
  void applyRemoteMessages();
  bool supportedMessage(msg_ptr msg);

 public slots:
  void reset();
  void messageReceived(msg_ptr msg);
  void handleMessage(msg_ptr msg);
  void updateConnectionStatus(NetworkClient::ConnectionStatus
      connection_status);

  void handleGetListReplyMessage(msg_ptr msg);
  void handleGetReplyMessage(msg_ptr msg);
  void handleGetDataReplyMessage(msg_ptr msg);
  void handleGetBinDataReplyMessage(msg_ptr msg);
  void handleRequestAckMessage(msg_ptr msg);
  void handleQueryErrorMessage(msg_ptr msg);

  void wrongMessageType(QString name, QString expected_type);

 signals:
   void startNodeDataModification(QString id);
   void endNodeDataModification(QString id);
   void startChildrenModification(QString id);
   void endChildrenModification(QString id);
   void beginReset();
   void endReset();
   void nodeDiscovered(QString id);

 public:
  void detachFromParent(data::NodeID id);
  void unregisterNode(data::NodeID id);

  uint64_t get(data::NodeID id, bool sub);
  uint64_t getList(data::NodeID id, bool sub);
  uint64_t getBinData(data::NodeID id, QString name, bool sub);
  uint64_t getData(data::NodeID id, QString name, bool sub);
  uint64_t deleteNode(data::NodeID id);
  uint64_t addChunk(data::NodeID parent_id, QString name, QString type,
      QString comment, int64_t start, int64_t end);
  uint64_t addFileBlob(
      QString path, const data::BinData& file_data, data::NodeID id);
  uint64_t setQStrAttribute(data::NodeID id,
      std::string attribute_name, QString value);

  void updateNode(std::shared_ptr<proto::Node> src_node, Node* dst_node);

  void printTree();
  void printNode(Node* node, QTextStream& out, int level);

 private:
  void removeSubtreeLocally(Node* child);

  NetworkClient* network_client_;
  std::list<msg_ptr> remote_messages_;
  std::unordered_map<std::string, MessageHandler> message_handlers_;
  std::unordered_map<data::NodeID, Node*, data::NodeIDHash> nodes_;
  std::unordered_map<uint64_t, data::NodeID> queries_;
  std::unordered_set<uint64_t> subscriptions_;
};

} // namespace client
} // namespace veles
