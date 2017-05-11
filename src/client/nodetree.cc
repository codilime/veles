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
  }
}

void NodeTree::handleGetListReplyMessage(msg_ptr msg) {
  auto reply = std::dynamic_pointer_cast<proto::MsgGetListReply>(msg);
  if (reply) {

  } else {
    wrongMessageType("get_list_reply", "MsgGetListReply");
  }
}

void NodeTree::handleGetReplyMessage(msg_ptr msg) {
  auto reply = std::dynamic_pointer_cast<proto::MsgGetReply>(msg);
  if (reply) {

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

  } else {
    wrongMessageType("get_bindata_reply", "MsgGetBinDataReply");
  }
}

void NodeTree::handleRequestAckMessage(msg_ptr msg) {
  auto reply = std::dynamic_pointer_cast<proto::MsgRequestAck>(msg);
  if (reply) {

  } else {
    wrongMessageType("request_ack", "MsgRequestAck");
  }
}

void NodeTree::handleQueryErrorMessage(msg_ptr msg) {
  auto reply = std::dynamic_pointer_cast<proto::MsgQueryError>(msg);
  if (reply) {

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

} // veles
} // client
