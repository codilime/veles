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

#include <QObject>
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

class Node : public QObject {
  Q_OBJECT

 public:
  Node(NodeTree* node_tree, data::NodeID id);

 public:
  data::NodeID id() const;
  Node* parent() const;
  data::NodeID parentId() const;
  void setParent(Node* parent);
  void setComment(QString comment);
  const std::unordered_set<Node*>& children() const;
  const std::vector<Node*>& childrenVect() const;
  int64_t start() const;
  int64_t end() const;
  const Tags& tags() const;
  bool hasTag(std::string tag) const;

  bool getQStringAttr(std::string key, QString& val_out) {
    if (attributes_) {
      auto iter = attributes_->find(key);
      if (iter != attributes_->end()) {
        auto val_ptr = iter->second->getString();
        if (val_ptr) {
          val_out = QString::fromStdString(*val_ptr);
          return true;
        }
      }
    }

    return false;
  }

  template<class T> bool getAttrSPtr(std::string key, T& val) {
    if (attributes_) {
      auto iter = attributes_->find(key);
      if (iter != attributes_->end()) {
        std::shared_ptr<T> val_ptr;
        messages::fromMsgpackObject(iter->second, val_ptr);
        if (val_ptr) {
          val = *val_ptr;
          return true;
        }
      }
    }

    return false;
  }

  template<class T> bool getAttr(std::string key, T& val) {
    if (attributes_) {
      auto iter = attributes_->find(key);
      if (iter != attributes_->end()) {
        messages::fromMsgpackObject(iter->second, val);
        return true;
      }
    }

    return false;
  }

  uint64_t get(bool sub);
  uint64_t getList(bool sub);
  uint64_t getBinData(QString name, bool sub);
  uint64_t getData(QString name, bool sub);
  uint64_t deleteNode();

  uint64_t lastGetListQid();
  void setLastGetListQid(uint64_t qid);

  uint64_t expectedGetQid();
  void setExpectedGetQid(uint64_t qid);
  uint64_t expectedGetListQid();
  void setExpectedGetListQid(uint64_t qid);

  bool operator<(const Node& other_node) const;
  bool operator==(const Node& other_node) const;

  uint64_t index();

  std::shared_ptr<data::BinData> binData(std::string name);
  std::shared_ptr<veles::messages::MsgpackObject> data(std::string name);
  QString nodePath();

  static bool nodeIsSmaller(Node* first, Node* second) {
    return *first < *second;
  }

 signals:
  void binDataUpdated(QString name);
  void dataUpdated(QString name);

 private:
  virtual ~Node();
  void addChildLocally(Node* child);
  void removeChildLocally(Node* child);
  void updateChildrenVect();

  NodeTree* node_tree_;
  data::NodeID id_;
  Node* parent_;
  std::unordered_set<Node*> children_;
  std::vector<Node*> children_vect_;
  int64_t start_;
  int64_t end_;
  Tags tags_;
  Attributes attributes_;
  DataNamesSet data_names_;
  BindataNamesSet bindata_names_;
  std::unordered_map<std::string, std::shared_ptr<veles::messages::MsgpackObject>> data_;
  std::unordered_map<std::string, std::shared_ptr<data::BinData>> bin_data_;

  uint64_t expected_get_qid_;
  uint64_t last_get_list_qid_;
  uint64_t expected_get_list_qid_;

  uint64_t index_;

  friend class NodeTree;
};

} // client
} // veles
