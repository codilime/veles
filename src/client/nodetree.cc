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

#include <functional>
#include <cstring>
#include <list>

#include "client/node.h"
#include "client/nodetree.h"
#include "client/networkclient.h"

namespace veles {
namespace client {

/*****************************************************************************/
/* NodeTree */
/*****************************************************************************/

NodeTree::NodeTree(NetworkClient* network_client, QObject* parent)
    : QObject(parent), network_client_(network_client) {
  if (network_client_) {
    message_handlers_["get_list_reply"]
        = &NodeTree::handleGetListReplyMessage;
    message_handlers_["get_reply"]
        = &NodeTree::handleGetReplyMessage;
    message_handlers_["get_data_reply"]
        = &NodeTree::handleGetDataReplyMessage;
    message_handlers_["get_bindata_reply"]
        = &NodeTree::handleGetBinDataReplyMessage;
    message_handlers_["request_ack"]
        = &NodeTree::handleRequestAckMessage;
    message_handlers_["query_error"]
        = &NodeTree::handleQueryErrorMessage;

    connect(network_client_, &NetworkClient::messageReceived,
        this, &NodeTree::messageReceived);
    connect(network_client_, &NetworkClient::connectionStatusChanged,
        this, &NodeTree::updateConnectionStatus);
  }
}

NodeTree::~NodeTree() {
  for (auto node_iter : nodes_) {
    delete node_iter.second;
  }
}

Node* NodeTree::node(data::NodeID id) {
  auto node_iter = nodes_.find(id);
  if(node_iter != nodes_.end()) {
    return node_iter->second;
  } else {
    return nullptr;
  }
}

void NodeTree::applyRemoteMessages() {
  while (!remote_messages_.empty()) {
    handleMessage(remote_messages_.front());
    remote_messages_.pop_front();
  }
}

bool NodeTree::supportedMessage(msg_ptr msg) {
  auto handler_iter = message_handlers_.find(msg->object_type);
  return handler_iter != message_handlers_.end();
}

void NodeTree::messageReceived(msg_ptr msg) {
  if (supportedMessage(msg)) {
    remote_messages_.push_back(msg);
  }

  // this is temporary
  applyRemoteMessages();
}

void NodeTree::handleMessage(msg_ptr msg) {
  auto handler_iter = message_handlers_.find(msg->object_type);
  if(handler_iter != message_handlers_.end()) {
    MessageHandler handler = handler_iter->second;
    (this->*handler)(msg);
  }
}

void NodeTree::updateConnectionStatus(NetworkClient::ConnectionStatus
    connection_status) {
  if(connection_status ==
      client::NetworkClient::ConnectionStatus::Connected) {
    // TODO
  }
}

void NodeTree::handleGetListReplyMessage(msg_ptr msg) {
  auto reply = std::dynamic_pointer_cast<proto::MsgGetListReply>(msg);
  if (reply) {
    // TODO
  } else {
    wrongMessageType("get_list_reply", "MsgGetListReply");
  }
}

void NodeTree::handleGetReplyMessage(msg_ptr msg) {
  auto reply = std::dynamic_pointer_cast<proto::MsgGetReply>(msg);
  if (reply) {
    // TODO
  } else {
    wrongMessageType("get_reply", "MsgGetReply");
  }
}

void NodeTree::handleGetDataReplyMessage(msg_ptr msg) {
  auto reply = std::dynamic_pointer_cast<proto::MsgGetListReply>(msg);
  if (reply) {

  } else {
    wrongMessageType("get_list_reply", "MsgGetListReply");
  }
}

void NodeTree::handleGetBinDataReplyMessage(msg_ptr msg) {
  auto reply = std::dynamic_pointer_cast<proto::MsgGetBinDataReply>(msg);
  if (reply) {
    // TODO
  } else {
    wrongMessageType("get_bindata_reply", "MsgGetBinDataReply");
  }
}

void NodeTree::handleRequestAckMessage(msg_ptr msg) {
  auto reply = std::dynamic_pointer_cast<proto::MsgRequestAck>(msg);
  if (reply) {
    // TODO
  } else {
    wrongMessageType("request_ack", "MsgRequestAck");
  }
}

void NodeTree::handleQueryErrorMessage(msg_ptr msg) {
  auto reply = std::dynamic_pointer_cast<proto::MsgQueryError>(msg);
  if (reply) {
    // TODO
  } else {
    wrongMessageType("query_error", "MsgQueryError");
  }
}

void NodeTree::wrongMessageType(QString name, QString expected_type) {
  if (network_client_->output()) {
    *network_client_->output() << QString("NodeTree: error - declared message"
        " type is \"%1\", but it's actually not a %2.").arg(name)
        .arg(expected_type) << endl;
  }
}

void NodeTree::detachFromParent(data::NodeID id) {
  Node* child = node(id);
  Node* parent = nullptr;

  if (child) {
    child->setParent(nullptr);
    parent = child->parent();
  }

  if (parent) {
    parent->removeChild(child);
  }
}

void NodeTree::unregisterNode(data::NodeID id) {
  detachFromParent(id);
  nodes_.erase(id);
}

uint64_t NodeTree::get(data::NodeID id, bool sub) {
  Node* n = node(id);
  if (n) {
    uint64_t qid = network_client_->nextQid();

    auto msg = std::make_shared<proto::MsgGet>(
        qid,
        std::make_shared<data::NodeID>(id),
        sub);
    network_client_->sendMessage(msg);

    return qid;
  } else {
    return 0;
  }
}

uint64_t NodeTree::getList(data::NodeID id, bool sub) {
  uint64_t qid = network_client_->nextQid();

  // TODO handle tags and pos filters here
  const auto null_pos = std::pair<bool, int64_t>(false, 0);
  auto msg = std::make_shared<proto::MsgGetList>(
      qid,
      std::make_shared<data::NodeID>(id),
      std::make_shared<std::unordered_set<std::shared_ptr<std::string>>>(),
      std::make_shared<proto::PosFilter>(
          null_pos, null_pos, null_pos, null_pos),
      sub);
  network_client_->sendMessage(msg);

  return qid;
}

uint64_t NodeTree::deleteNode(data::NodeID id) {
  uint64_t rid = network_client_->nextQid();

  auto msg = std::make_shared<proto::MsgDelete>(
      rid,
      std::make_shared<data::NodeID>(id));
  network_client_->sendMessage(msg);

  return rid;
}

void NodeTree::printTree() {
  QTextStream* out_ptr = network_client_->output();
  if (out_ptr) {
    QTextStream& out = *out_ptr;
    out << "NodeTree::printTree" << endl;
    Node* root = node(*data::NodeID::getRootNodeId());
    if (root) {
      printNode(root, out, 1);
    }
  }
}

void NodeTree::printNode(Node* node, QTextStream& out, int level) {
  auto name_ptr = node->attribute<std::shared_ptr<std::string>>("name");
  for (int i = 0; i < level; ++i) {
    out << "  ";
  }
  out << (name_ptr ? QString::fromStdString(*name_ptr) : "[na name]")
      << "(" << node->id().toHexString() << ")" << endl;
  for (auto child : node->children()) {
    printNode(child, out, level + 1);
  }
}

} // veles
} // client
