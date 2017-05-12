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

#include <cstdint>
#include <unordered_set>
#include <unordered_map>

#include <QString>

#include "data/nodeid.h"
#include "data/bindata.h"
#include "network/msgpackwrapper.h"

namespace veles {
namespace client {

class NodeTree;
class NetworkClient;

typedef std::unordered_set<std::string> Tags;
typedef std::shared_ptr<std::unordered_map<std::string,std::shared_ptr<
    veles::messages::MsgpackObject>>> Attributes;
typedef std::shared_ptr<std::unordered_set<std::shared_ptr<std::string>>>
    DataNamesSet;
typedef std::shared_ptr<std::unordered_map<std::string,uint64_t>>
    BindataNamesSet;

/*****************************************************************************/
/* Node */
/*****************************************************************************/

class Node {
 public:
  typedef std::unordered_set<Node*>::iterator ChildIterator;
  Node(NodeTree* node_tree, data::NodeID id);
  virtual ~Node();

  data::NodeID id() const;
  Node* parent() const;
  data::NodeID parentId() const;
  void setParent(Node* parent);
  const std::unordered_set<Node*>& children() const;
  void addChild(Node* child);
  void removeChild(Node* child);
  int64_t start();
  int64_t end();
  const Tags& tags() const;
  bool hasTag(std::string tag);

  template<class T> const T attribute(std::string name) {
    // TODO
    return T();
  }

  uint64_t get(bool sub);
  uint64_t getList(bool sub);
  uint64_t deleteNode();

  uint64_t lastGetQid();
  void setLastGetQid(uint64_t qid);
  uint64_t lastGetListQid();
  void setLastGetListQid(uint64_t qid);

  uint64_t expectedGetQid();
  void setExpectedGetQid(uint64_t qid);
  uint64_t expectedGetListQid();
  void setExpectedGetListQid(uint64_t qid);

 private:
  NodeTree* node_tree_;
  data::NodeID id_;
  Node* parent_;
  std::unordered_set<Node*> children_;
  int64_t start_;
  int64_t end_;
  Tags tags_;
  Attributes attributes_;
  DataNamesSet data_names_;
  BindataNamesSet bindata_names_;

  uint64_t last_get_qid_;
  uint64_t expected_get_qid_;
  uint64_t last_get_list_qid_;
  uint64_t expected_get_list_qid_;
};

} // client
} // veles
