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
    id_(id), parent_(nullptr), start_(0), end_(0), last_get_qid_(0),
    expected_get_qid_(0), last_get_list_qid_(0), expected_get_list_qid_(0) {
}

Node::~Node() {
  for (auto child : children_) {
    child->setParent(nullptr);
    delete child;
  }

  node_tree_->unregisterNode(id());
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

const std::unordered_set<Node*>& Node::children() const {
  return children_;
}

void Node::addChild(Node* child) {
  children_.insert(child);
}

void Node::removeChild(Node* child) {
  children_.erase(child);
}

int64_t Node::start() {
  return start_;
}

int64_t Node::end() {
  return end_;
}

const Tags& Node::tags() const {
  return tags_;
}

bool Node::hasTag(std::string tag) {
  return tags_.find(tag) != tags_.end();
}

uint64_t Node::get(bool sub) {
  return node_tree_->get(id(), sub);
}

uint64_t Node::getList(bool sub) {
  return node_tree_->getList(id(), sub);
}

uint64_t Node::deleteNode() {
  return node_tree_->deleteNode(id());
}

uint64_t Node::lastGetQid() {
  return last_get_qid_;
}

void Node::setLastGetQid(uint64_t qid) {
  last_get_qid_ = qid;
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

} // client
} // veles
