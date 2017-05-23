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

#include "client/node.h"
#include "client/nodetree.h"
#include "client/networkclient.h"

namespace veles {
namespace client {

/*****************************************************************************/
/* Node */
/*****************************************************************************/

Node::Node(NodeTree* node_tree, data::NodeID id) : node_tree_(node_tree),
    id_(id), parent_(nullptr), start_(0), end_(0), expected_get_qid_(0),
    last_get_list_qid_(0), expected_get_list_qid_(0), index_(0) {
}

data::NodeID Node::id() const {
  return id_;
}

Node* Node::parent() const {
  return parent_;
}

data::NodeID Node::parentId() const {
  if(parent_) {
    return parent_->id();
  } else {
    return *data::NodeID::getNilId();
  }
}

void Node::setParent(Node* parent) {
  parent_ = parent;
}

void Node::setComment(QString comment) {
  node_tree_->setQStrAttribute(id(), "comment", comment);
}

const std::unordered_set<Node*>& Node::children() const {
  return children_;
}

const std::vector<Node*>& Node::childrenVect() const {
  return children_vect_;
}

int64_t Node::start() const {
  return start_;
}

int64_t Node::end() const {
  return end_;
}

const Tags& Node::tags() const {
  return tags_;
}

bool Node::hasTag(std::string tag) const {
  return tags_.find(tag) != tags_.end();
}

uint64_t Node::get(bool sub) {
  return node_tree_->get(id(), sub);
}

uint64_t Node::getList(bool sub) {
  return node_tree_->getList(id(), sub);
}

uint64_t Node::getBinData(QString name, bool sub) {
  return node_tree_->getBinData(id(), name, sub);
}

uint64_t Node::getData(QString name, bool sub) {
  return node_tree_->getData(id(), name, sub);
}

uint64_t Node::deleteNode() {
  return node_tree_->deleteNode(id());
}

uint64_t Node::lastGetListQid() {
  return last_get_list_qid_;
}

void Node::setLastGetListQid(uint64_t qid) {
  last_get_list_qid_ = qid;
}

uint64_t Node::expectedGetQid() {
  return expected_get_qid_;
}

void Node::setExpectedGetQid(uint64_t qid) {
  expected_get_qid_ = qid;
}

uint64_t Node::expectedGetListQid() {
  return expected_get_list_qid_;
}

void Node::setExpectedGetListQid(uint64_t qid) {
  expected_get_list_qid_ = qid;
}

bool Node::operator<(const Node& other_node) const {
  if (start() != other_node.start()) {
    return start() < other_node.start();
  } else if (end() != other_node.end()) {
    return end() < other_node.end();
  }

  return id() < other_node.id();
}

bool Node::operator==(const Node& other_node) const {
  return id() == other_node.id();
}

uint64_t Node::index() {
  return index_;
}

std::shared_ptr<data::BinData> Node::binData(std::string name) {
  auto iter = bin_data_.find(name);
  if (iter == bin_data_.end()) {
    return nullptr;
  }

  return iter->second;
}

std::shared_ptr<veles::messages::MsgpackObject> Node::data(std::string name) {
  auto iter = data_.find(name);
  if (iter == data_.end()) {
    return nullptr;
  }

  return iter->second;
}

QString Node::nodePath() {
  Node* n = this;
  while(!n->hasTag("blob.file") && n->parent() != nullptr) {
    n = n->parent();
  }

  QString path("[no path]");
  n->getQStringAttr("path", path);
  if (n != this) {
    QString name("[no name]");
    getQStringAttr("name", name);
    path.append(QString(":%1").arg(name));
  }

  return path;
}

Node::~Node() {}

void Node::addChildLocally(Node* child) {
  children_.insert(child);
}

void Node::removeChildLocally(Node* child) {
  children_.erase(child);
}

void Node::updateChildrenVect() {
  children_vect_.clear();
  children_vect_.insert(children_vect_.begin(),
      children_.begin(), children_.end());
  std::sort(children_vect_.begin(), children_vect_.end(), nodeIsSmaller);

  uint64_t i = 0;
  for (auto child : children_vect_) {
    child->index_ = i++;
  }
}

} // client
} // veles
