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

#include <algorithm>
#include <functional>
#include <cstring>
#include <list>

#include "network/msgpackwrapper.h"
#include "network/msgpackobject.h"
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

    reset();

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

void NodeTree::reset() {
  emit beginReset();
  remote_messages_.clear();

  for (auto node : nodes_) {
    delete node.second;
  }

  nodes_.clear();
  Node* root = new Node(this, *data::NodeID::getRootNodeId());
  root->attributes_ = std::make_shared<std::unordered_map<
      std::string,std::shared_ptr<veles::messages::MsgpackObject>>>();
  (*root->attributes_)["name"]
      = std::make_shared<messages::MsgpackObject>("Root");
  nodes_[root->id()] = root;
  queries_.clear();
  subscriptions_.clear();
  emit endReset();
}

void NodeTree::messageReceived(msg_ptr msg) {
  if (supportedMessage(msg)) {
    remote_messages_.push_back(msg);
  }

  // TODO this is temporary and should be eventually invoked by client
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

        std::list<Node*> new_children;

        if (reply->qid != n->lastGetListQid()) {
          n->setLastGetListQid(reply->qid);
          std::list<Node*> children_list;
          for (auto child : n->children()) {
            removeSubtreeLocally(child);
          }
        }

        for (auto new_child : *reply->objs) {
          Node* new_node = nullptr;
          auto node_iter = nodes_.find(*new_child->id);
          if (node_iter == nodes_.end()) {
            new_node = new Node(this, *new_child->id);
            nodes_[new_node->id()] = new_node;
            new_node->setParent(n);
            n->addChildLocally(new_node);
            new_children.push_back(new_node);
          } else {
            // TODO investigate situations that result in entering into this case
            new_node = node_iter->second;
          }

          updateNode(new_child, new_node);
        }

        for (auto removed_child_id : *reply->gone) {
          Node* removed_child = node(*removed_child_id);
          if (removed_child) {
            removeSubtreeLocally(removed_child);
          }
        }

        n->updateChildrenVect();
        emit endChildrenModification(node_id.toHexString());

        for (auto new_node : new_children) {
          // TODO this is temporary
          // It should be eventually called by models after user's request
          new_node->getList(true);
          new_node->getData("data_items", true);

          emit nodeDiscovered(new_node->id().toHexString());
        }

        printTree();
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
        // TODO Sorting the nodes might be required here.
        // In such a case startChildrenModification/endChildrenModification
        // should also be emitted for parent of n.
        emit endNodeDataModification(node_id.toHexString());

        Node* parent = n->parent();
        if (parent && !std::is_sorted(parent->children_vect_.begin(),
            parent->children_vect_.end(), Node::nodeIsSmaller)) {
          emit startChildrenModification(parent->id().toHexString());
          std::sort(parent->children_vect_.begin(), parent->children_vect_.end(), Node::nodeIsSmaller);
          int i = 0;
          for(auto child : parent->children_vect_) {
            child->index_ = i++;
          }
          emit endChildrenModification(parent->id().toHexString());
        }
      }
    }
  } else {
    wrongMessageType("get_reply", "MsgGetReply");
  }
}

void NodeTree::handleGetDataReplyMessage(msg_ptr msg) {
  auto reply = std::dynamic_pointer_cast<proto::MsgGetDataReply>(msg);
  if (reply) {
    auto queries_iter = queries_.find(reply->qid);
    if (queries_iter != queries_.end()) {
      auto node_id = queries_iter->second;
      if (subscriptions_.find(reply->qid) == subscriptions_.end()) {
        queries_.erase(reply->qid);
      }

      Node* n = node(node_id);
      if (n) {
        // TODO handle key properly
        // TODO handle data removal
        n->data_["data_items"] = reply->data.second;
        emit n->dataUpdated("data_items");
      }
    }
  } else {
    wrongMessageType("get_data_reply", "MsgGetDataReply");
  }
}

void NodeTree::handleGetBinDataReplyMessage(msg_ptr msg) {
  auto reply = std::dynamic_pointer_cast<proto::MsgGetBinDataReply>(msg);
  if (reply) {
    auto queries_iter = queries_.find(reply->qid);
    if (queries_iter != queries_.end()) {
      auto node_id = queries_iter->second;
      if (subscriptions_.find(reply->qid) == subscriptions_.end()) {
        queries_.erase(reply->qid);
      }

      Node* n = node(node_id);
      if (n) {
        // TODO handle key properly
        n->bin_data_["data"] = std::make_shared<data::BinData>(
            8, reply->data->size(), reply->data->data());
        emit n->binDataUpdated("data");
      }
    }
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
    if (n->expectedGetQid()) {
      // TODO properly handle sub
      return n->expectedGetQid();
    }

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
    if (n->expectedGetListQid()) {
      // TODO properly handle sub
      return n->expectedGetListQid();
    }

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

uint64_t NodeTree::getBinData(data::NodeID id, QString name, bool sub) {
  Node* n = node(id);
  if (n) {
    uint64_t qid = network_client_->nextQid();
    queries_[qid] = id;

    if (sub) {
      subscriptions_.insert(qid);
    }

    auto msg = std::make_shared<proto::MsgGetBinData>(
        qid,
        std::make_shared<data::NodeID>(id),
        std::make_shared<std::string>(name.toStdString()),
        n->start(),
        std::pair<bool, uint64_t>(false, 0),
        sub);

    network_client_->sendMessage(msg);
    return qid;
  } else {
    return 0;
  }
}

uint64_t NodeTree::getData(data::NodeID id, QString name, bool sub) {
  Node* n = node(id);
  if (n) {
    uint64_t qid = network_client_->nextQid();
    queries_[qid] = id;

    if (sub) {
      subscriptions_.insert(qid);
    }

    auto msg = std::make_shared<proto::MsgGetData>(
        qid,
        std::make_shared<data::NodeID>(id),
        std::make_shared<std::string>(name.toStdString()),
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

uint64_t NodeTree::addChunk(data::NodeID parent_id, QString name, QString type,
    QString comment, int64_t start, int64_t end) {
  uint64_t rid = network_client_->nextQid();
  auto new_id = std::make_shared<data::NodeID>();

  auto tags = std::make_shared<std::unordered_set<std::shared_ptr<
      std::string>>>();
  tags->insert(std::make_shared<std::string>("chunk"));
  tags->insert(std::make_shared<std::string>("chunk.stored"));
  auto attr = std::make_shared<std::unordered_map<
      std::string,std::shared_ptr<messages::MsgpackObject>>>();
  attr->insert(std::pair<std::string, std::shared_ptr<
      messages::MsgpackObject>>("name",
      std::make_shared<messages::MsgpackObject>(name.toStdString())));
  attr->insert(std::pair<std::string, std::shared_ptr<
      messages::MsgpackObject>>("comment",
      std::make_shared<messages::MsgpackObject>(comment.toStdString())));
  attr->insert(std::pair<std::string, std::shared_ptr<
      messages::MsgpackObject>>("type",
      std::make_shared<messages::MsgpackObject>(type.toStdString())));

  auto data = std::make_shared<std::unordered_map<
      std::string,std::shared_ptr<messages::MsgpackObject>>>();

  auto bindata = std::make_shared<std::unordered_map<
      std::string,std::shared_ptr<std::vector<uint8_t>>>>();

  auto triggers = std::make_shared<std::unordered_set<
      std::shared_ptr<std::string>>>();

  auto operation = std::make_shared<proto::OperationCreate>(
      new_id,
      std::make_shared<data::NodeID>(parent_id),
      std::pair<bool, int64_t>(true, start),
      std::pair<bool, int64_t>(true, end),
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
      rid,
      std::make_shared<std::vector<std::shared_ptr<proto::Check>>>(),
      operations);
  network_client_->sendMessage(msg);

  return rid;
}

uint64_t NodeTree::addFileBlob(
    QString path, const data::BinData& file_data, data::NodeID id) {
  auto rid = network_client_->nextQid();
  auto new_id = std::make_shared<data::NodeID>(id);

  auto tags = std::make_shared<std::unordered_set<std::shared_ptr<
      std::string>>>();
  tags->insert(std::make_shared<std::string>("blob"));
  tags->insert(std::make_shared<std::string>("blob.file"));
  tags->insert(std::make_shared<std::string>("blob.stored"));
  auto attr = std::make_shared<std::unordered_map<
      std::string,std::shared_ptr<messages::MsgpackObject>>>();
  attr->insert(std::pair<std::string, std::shared_ptr<
      messages::MsgpackObject>>("name",
      std::make_shared<messages::MsgpackObject>(
      "\"" + path.toStdString() + "\"")));
  attr->insert(std::pair<std::string, std::shared_ptr<
      messages::MsgpackObject>>("path",
      std::make_shared<messages::MsgpackObject>(path.toStdString())));
  attr->insert(std::pair<std::string, std::shared_ptr<
      messages::MsgpackObject>>("size",
      std::make_shared<messages::MsgpackObject>(
      static_cast<uint64_t>(file_data.size()))));

  auto data = std::make_shared<std::unordered_map<
      std::string,std::shared_ptr<messages::MsgpackObject>>>();

  auto bindata = std::make_shared<std::unordered_map<
      std::string,std::shared_ptr<std::vector<uint8_t>>>>();
  auto data_vec = std::make_shared<std::vector<uint8_t>>();
  data_vec->assign(file_data.rawData(),
      file_data.rawData() + file_data.octets());
  bindata->insert(std::pair<std::string, std::shared_ptr<std::vector<uint8_t>>>
      ("data", data_vec));

  auto triggers = std::make_shared<std::unordered_set<
      std::shared_ptr<std::string>>>();

  auto operation = std::make_shared<proto::OperationCreate>(
      new_id,
      data::NodeID::getRootNodeId(),
      std::pair<bool, int64_t>(true, 0),
      std::pair<bool, int64_t>(true, file_data.octets()),
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
      rid,
      std::make_shared<std::vector<std::shared_ptr<proto::Check>>>(),
      operations);
  network_client_->sendMessage(msg);

  return rid;
}

uint64_t NodeTree::setQStrAttribute(data::NodeID id,
    std::string attribute_name, QString value) {
  uint64_t rid = network_client_->nextQid();

  auto operation = std::make_shared<proto::OperationSetAttr>(
      std::make_shared<data::NodeID>(id),
      std::make_shared<std::string>(attribute_name),
      std::pair<bool, std::shared_ptr<messages::MsgpackObject>>(true,
      std::make_shared<messages::MsgpackObject>(value.toStdString())));

  auto operations = std::make_shared<std::vector<std::shared_ptr<
      proto::Operation>>>();
      operations->push_back(operation);

  auto msg = std::make_shared<proto::MsgTransaction>(
      rid,
      std::make_shared<std::vector<std::shared_ptr<proto::Check>>>(),
      operations);
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

  std::unordered_set<Node*> children = child->children();

  for(auto node : children) {
    removeSubtreeLocally(node);
  }

  delete child;
}

} // namespace client
} // namespace veles
