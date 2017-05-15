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

    Node* root = new Node(this, *data::NodeID::getRootNodeId());
    nodes_[root->id()] = root;

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

  // FIXME this is temporary
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
    auto queries_iter = queries_.find(reply->qid);
    if (queries_iter != queries_.end()) {
      auto node_id = queries_iter->second;
      if (subscriptions_.find(reply->qid) == subscriptions_.end()) {
        queries_.erase(reply->qid);
      }

      Node* n = node(node_id);
      if (n && reply->qid == n->expectedGetListQid()) {
        emit startChildrenModification(node_id.toHexString());

        if (reply->qid != n->lastGetListQid()) {
          n->setLastGetListQid(reply->qid);
          std::list<Node*> children_list;
          for (auto child : n->children()) {
            removeSubtreeLocally(child);
          }
        }

        for (auto new_child : *reply->objs) {
          Node* new_node = new Node(this, *new_child->id);
          nodes_[new_node->id()] = new_node;
          new_node->setParent(n);
          n->addChildLocally(new_node);
          updateNode(new_child, new_node);

          // FIXME
          new_node->get(true);
          new_node->getList(true);
          printTree();

          // TODO
        }

        for (auto removed_child_id : *reply->gone) {
          Node* removed_child = node(*removed_child_id);
          if (removed_child) {
            removeSubtreeLocally(removed_child);
          }
        }

        n->updateChildrenVect();
        emit endChildrenModification(node_id.toHexString());
      }
    }
  } else {
    wrongMessageType("get_list_reply", "MsgGetListReply");
  }
}

void NodeTree::handleGetReplyMessage(msg_ptr msg) {
  auto reply = std::dynamic_pointer_cast<proto::MsgGetReply>(msg);
  if (reply) {
    auto queries_iter = queries_.find(reply->qid);
    if (queries_iter != queries_.end()) {
      auto node_id = queries_iter->second;
      if (subscriptions_.find(reply->qid) == subscriptions_.end()) {
        queries_.erase(reply->qid);
      }

      Node* n = node(node_id);
      if (n) {
        emit startNodeDataModification(node_id.toHexString());
        updateNode(reply->obj, n);
        emit endNodeDataModification(node_id.toHexString());
      }
    }
  } else {
    wrongMessageType("get_reply", "MsgGetReply");
  }
}

void NodeTree::handleGetDataReplyMessage(msg_ptr msg) {
  auto reply = std::dynamic_pointer_cast<proto::MsgGetDataReply>(msg);
  if (reply) {

  } else {
    wrongMessageType("get_data_reply", "MsgGetDataReply");
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
    parent = child->parent();
    child->setParent(nullptr);
  }

  if (parent) {
    parent->removeChildLocally(child);
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
    queries_[qid] = id;
    n->setExpectedGetQid(qid);
    if (sub) {
      subscriptions_.insert(qid);
    }

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
  Node* n = node(id);
  if (n) {
    uint64_t qid = network_client_->nextQid();
    queries_[qid] = id;
    n->setExpectedGetListQid(qid);
    if (sub) {
      subscriptions_.insert(qid);
    }

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
  } else {
    return 0;
  }
}

uint64_t NodeTree::deleteNode(data::NodeID id) {
  uint64_t rid = network_client_->nextQid();

  auto msg = std::make_shared<proto::MsgDelete>(
      rid,
      std::make_shared<data::NodeID>(id));
  network_client_->sendMessage(msg);

  return rid;
}

void NodeTree::updateNode(
    std::shared_ptr<proto::Node> src_node, Node* dst_node) {
  dst_node->attributes_ = src_node->attr;

  if (src_node->pos_start.first) {
    dst_node->start_ = src_node->pos_start.second;
  }

  if (src_node->pos_end.first) {
    dst_node->end_ = src_node->pos_end.second;
  }

  dst_node->tags_.clear();
  for (auto tag_ptr : *src_node->tags) {
    dst_node->tags_.insert(*tag_ptr);
  }

  dst_node->data_names_ = src_node->data;
  dst_node->bindata_names_ = src_node->bindata;

  // TODO triggers
  // TODO changing parent
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
  QString name("[no name]");
  node->getQStringAttr("name", name);

  for (int i = 0; i < level; ++i) {
    out << "  ";
  }

  QString tags_str;
  for (auto tag : node->tags_) {
    tags_str += QString("%1, ").arg(QString::fromStdString(tag));
  }

  out << QString("\"%1\" (%2) [%3]").arg(name).arg(node->id().toHexString())
      .arg(tags_str) << endl;
  for (auto child : node->childrenVect()) {
    printNode(child, out, level + 1);
  }
}

void NodeTree::removeSubtreeLocally(Node* child) {
  unregisterNode(child->id());

  for(auto node : child->children()) {
    removeSubtreeLocally(node);
  }

  delete child;
}

} // namespace client
} // namespace veles
