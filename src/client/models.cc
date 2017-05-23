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

#include <QFont>

#include "dbif/universe.h"
#include "client/models.h"
#include "client/node.h"
#include "client/nodetree.h"
#include "client/networkclient.h"
#include "util/settings/theme.h"

namespace veles {
namespace client {

QString zeroPaddedHexNumber(uint64_t number) {
  auto num = QString::number(number, 16);
  while (num.length() < 4) {
    num = "0" + num;
  }
  return num;
}

/*****************************************************************************/
/* NodeTreeModelBase */
/*****************************************************************************/

NodeTreeModelBase::NodeTreeModelBase(NodeTree* node_tree, data::NodeID root,
    QObject* parent)
    : QAbstractItemModel(parent), node_tree_(node_tree), root_(root),
      node_reset_(*data::NodeID::getNilId()) {
  connect(node_tree, &NodeTree::startNodeDataModification,
      this, &NodeTreeModelBase::startNodeDataModificationSlot,
      Qt::UniqueConnection);
  connect(node_tree, &NodeTree::endNodeDataModification,
      this, &NodeTreeModelBase::endNodeDataModificationSlot,
      Qt::UniqueConnection);
  connect(node_tree, &NodeTree::startChildrenModification,
      this, &NodeTreeModelBase::startChildrenModificationSlot,
      Qt::UniqueConnection);
  connect(node_tree, &NodeTree::endChildrenModification,
      this, &NodeTreeModelBase::endChildrenModificationSlot,
      Qt::UniqueConnection);
  connect(node_tree, &NodeTree::beginReset,
        this, &NodeTreeModelBase::beginResetModel,
        Qt::UniqueConnection);
  connect(node_tree, &NodeTree::endReset,
        this, &NodeTreeModelBase::endResetModel,
        Qt::UniqueConnection);
}

NodeTreeModelBase::~NodeTreeModelBase() {
}

bool NodeTreeModelBase::hasChildren(const QModelIndex &parent) const {
  if (parent.isValid()) {
    Node* node = nodeFromIndex(parent);
    if (node && !node->childrenVect().empty()) {
      return true;
    }
  }

  return false;
}

QModelIndex NodeTreeModelBase::index(int row, int column,
    const QModelIndex &parent) const {
  Node* node = nodeFromIndex(parent);

  if (node) {
    if (node->childrenVect().size() > (unsigned)row) {
      return createIndex(row, column, node->childrenVect()[row]);
    }
  }
  return QModelIndex();
}

QModelIndex NodeTreeModelBase::parent(const QModelIndex &index) const {
  if (index.isValid()) {
    Node* node = parentNodeFromIndex(index);
    if (node) {
      return createIndex(node->index(), 0, node);
    }
  }

  return QModelIndex();
}

int NodeTreeModelBase::rowCount(const QModelIndex &parent) const {
  Node* node = nodeFromIndex(parent);

  if (node && node->id() != node_reset_) {
    return node->childrenVect().size();
  }

  return 0;
}

QModelIndex NodeTreeModelBase::indexFromId(data::NodeID id) const {
  Node* node = node_tree_->node(id);
  if (node) {
    return createIndex(node->index(), 0, node_tree_->node(id));
  } else {
    return QModelIndex();
  }
}

data::NodeID NodeTreeModelBase::idFromIndex(const QModelIndex &index) const {
  Node* node = nodeFromIndex(index);
  if (node) {
    return node->id();
  } else {
    return *data::NodeID::getNilId();
  }
}

QSharedPointer<client::NCObjectHandle> NodeTreeModelBase::oldStyleHandleFromId(
      data::NodeID id) {
  client::Node* node = nodeTree()->node(id);
  if (node) {
    return QSharedPointer<client::NCObjectHandle>::create(
        NCWrapper::instance(), id, client::NCWrapper::typeFromTags(node->tags()));
  }

  return QSharedPointer<client::NCObjectHandle>::create(
      nullptr, id, dbif::ObjectType::FILE_BLOB);
}

template<class T> void getFieldFromMap(std::shared_ptr<std::map<std::string,
    std::shared_ptr<messages::MsgpackObject>>> fields, std::string key, T& val) {
  auto iter = fields->find(key);
  if (iter != fields->end()) {
    messages::fromMsgpackObject(iter->second, val);
  }
}

data::ChunkDataItem NodeTreeModelBase::msgpackToChunkDataItem(
    std::shared_ptr<messages::MsgpackObject> msgo) {
  data::ChunkDataItem item;
  auto attr_map = msgo->getMap();
  if (attr_map) {
    uint64_t type_uint(data::ChunkDataItem::NONE);
    getFieldFromMap(attr_map, "type", type_uint);
    item.type = data::ChunkDataItem::ChunkDataItemType(type_uint);

    uint64_t start(0);
    getFieldFromMap(attr_map, "start", start);
    item.start = start;

    uint64_t end(0);
    getFieldFromMap(attr_map, "end", end);
    item.end = end;

    uint64_t num_elements(0);
    getFieldFromMap(attr_map, "num_elements", num_elements);
    item.num_elements = num_elements;

    auto name = std::make_shared<std::string>("[not set]");
    getFieldFromMap(attr_map, "name", name);
    item.name = QString::fromStdString(*name);

    auto bindata = std::make_shared<data::BinData>();
    getFieldFromMap(attr_map, "raw_value", bindata);
    item.raw_value = *bindata;

    auto repacker = std::make_shared<data::Repacker>();
    getFieldFromMap(attr_map, "repack", repacker);
    item.repack = *repacker;

    bool float_complex = false;
    getFieldFromMap(attr_map, "float_complex", float_complex);
    item.high_type.float_complex = float_complex;

    uint64_t float_mode(data::FieldHighType::IEEE754_SINGLE);
    getFieldFromMap(attr_map, "float_mode", float_mode);
    item.high_type.float_mode = data::FieldHighType::FieldFloatMode(float_mode);

    uint64_t mode(data::FieldHighType::NONE);
    getFieldFromMap(attr_map, "mode", mode);
    item.high_type.mode = data::FieldHighType::FieldHighMode(mode);

    int64_t shift(0);
    getFieldFromMap(attr_map, "shift", shift);
    item.high_type.shift = shift;

    uint64_t sign_mode(data::FieldHighType::SIGNED);
    getFieldFromMap(attr_map, "sign_mode", sign_mode);
    item.high_type.sign_mode = data::FieldHighType::FieldSignMode(sign_mode);

    uint64_t string_encoding(data::FieldHighType::ENC_RAW);
    getFieldFromMap(attr_map, "string_encoding", string_encoding);
    item.high_type.string_encoding = data::FieldHighType::FieldStringEncoding(
        string_encoding);

    uint64_t string_mode(data::FieldHighType::STRING_RAW);
    getFieldFromMap(attr_map, "string_mode", string_mode);
    item.high_type.string_mode = data::FieldHighType::FieldStringMode(
        string_mode);

    auto type_name = std::make_shared<std::string>("[unknown]");
    getFieldFromMap(attr_map, "type_name", type_name);
    item.high_type.type_name = QString::fromStdString(*type_name);
  }

  return item;
}

std::shared_ptr<std::vector<std::shared_ptr<messages::MsgpackObject>>>
    NodeTreeModelBase::chunkDataItemsVector(client::Node* node) {
  std::shared_ptr<veles::messages::MsgpackObject> data_items
      = node->data("data_items");

  if (data_items) {
    std::shared_ptr<std::vector<std::shared_ptr<messages::MsgpackObject>>>
        data_items_vector;
    fromMsgpackObject(data_items, data_items_vector);
    return data_items_vector;
  }

  return nullptr;
}

unsigned NodeTreeModelBase::chunkDataItemsCount(client::Node* node) {
  std::shared_ptr<std::vector<std::shared_ptr<messages::MsgpackObject>>>
      data_items = chunkDataItemsVector(node);

  if (data_items) {
    return data_items->size();
  } else {
    return 0;
  }
}

bool NodeTreeModelBase::chunkDataItem(client::Node* node, unsigned index,
    data::ChunkDataItem& data_item) {
  std::shared_ptr<std::vector<std::shared_ptr<messages::MsgpackObject>>>
      data_items = chunkDataItemsVector(node);
  if (data_items && index < data_items->size()) {
    data_item = msgpackToChunkDataItem(data_items->at(index));
    return true;
  }
  return false;
}

void NodeTreeModelBase::startNodeDataModificationSlot(QString id) {
}

void NodeTreeModelBase::endNodeDataModificationSlot(QString id) {
  auto index = indexFromId(*data::NodeID::fromHexString(id));
  emit dataChanged(index, index);
}

void NodeTreeModelBase::startChildrenModificationSlot(QString id) {
  data::NodeID node_id = *data::NodeID::fromHexString(id);
  QModelIndex index = indexFromId(node_id);
  Node* node = nodeTree()->node(node_id);
  if (node && node->childrenVect().size() > 0) {
    beginRemoveRows(index, 0, node->childrenVect().size() - 1);
    node_reset_ = node_id;
    endRemoveRows();

  }
}

void NodeTreeModelBase::endChildrenModificationSlot(QString id) {
  data::NodeID node_id = *data::NodeID::fromHexString(id);
  QModelIndex index = indexFromId(node_id);
  Node* node = nodeTree()->node(node_id);
  if (node && node->childrenVect().size() > 0) {
    beginInsertRows(index, 0, node->childrenVect().size() - 1);
    node_reset_ = *data::NodeID::getNilId();
    endInsertRows();
  }
}

Node* NodeTreeModelBase::parentNodeFromIndex(const QModelIndex &index) const {
  Node* node = nodeFromIndex(index);
  return node ? node->parent() : nullptr;
}

Node* NodeTreeModelBase::nodeFromIndex(const QModelIndex &index) const {
  if (index.isValid()) {
    return reinterpret_cast<Node*>(index.internalPointer());
  } else {
    return nullptr;
  }
}

/*****************************************************************************/
/* NodeTreeModel */
/*****************************************************************************/

NodeTreeModel::NodeTreeModel(NodeTree* node_tree, data::NodeID root,
    QObject* parent)
    : NodeTreeModelBase(node_tree, root, parent) {
}

NodeTreeModel::~NodeTreeModel() {
}

int NodeTreeModel::columnCount(const QModelIndex &parent) const {
  return 4;
}

QVariant NodeTreeModel::data(const QModelIndex& index, int role) const {
  if (role == Qt::SizeHintRole) {
    return QSize(50, 20);
  }

  auto node = nodeFromIndex(index);

  if (role == Qt::DecorationRole) {
    if (index.isValid() && index.column() == COLUMN_INDEX_MAIN) {
      if (node != nullptr && !icon(index).isNull()) {
        return icon(index);
      } else {
        return color(index.row());
      }
    }
  }

  if (node == nullptr) return QVariant();
  QString name("[no name]");
  QString comment("");
  node->getQStringAttr("name", name);
  node->getQStringAttr("comment", comment);

  if (index.column() == COLUMN_INDEX_COMMENT &&
      (role == Qt::DisplayRole || role == Qt::EditRole)) {
    return comment;
  }

  if (index.column() == COLUMN_INDEX_POS) {
    return positionColumnData(node, role);
  }

  if (index.column() == COLUMN_INDEX_VALUE) {
    return valueColumnData(node, role);
  }

  if (index.column() != COLUMN_INDEX_MAIN) {
    return QVariant();
  }

  if (role == Qt::DisplayRole) {
    return name;
  }
  return QVariant();
}

QVariant NodeTreeModel::headerData(int section, Qt::Orientation orientation,
    int role) const {
  if (orientation != Qt::Orientation::Horizontal) {
    return QVariant();
  }

  if (role != Qt::DisplayRole) {
    return QVariant();
  }

  if (section == COLUMN_INDEX_MAIN) {
    return "Name";
  }

  if (section == COLUMN_INDEX_VALUE) {
    return "Value";
  }

  if (section == COLUMN_INDEX_COMMENT) {
    return "Comment";
  }

  if (section == COLUMN_INDEX_POS) {
    return "Position";
  }

  return QVariant();
}

void NodeTreeModel::addChunk(QString name, QString type, QString comment,
    int64_t start, int64_t end, const QModelIndex& index) {
  Node* parent_node = nodeFromIndex(index);
  if (parent_node) {
    node_tree_->addChunk(parent_node->id(), name, type, comment, start, end);
  }
}

void NodeTreeModel::parse(data::NodeID root, QString parser, qint64 offset,
    const QModelIndex& parent) {
  auto file_blob = oldStyleHandleFromId(root);
  dbif::ObjectHandle parent_chunk;
  if (parent.isValid()) {
    parent_chunk = oldStyleHandleFromId(idFromIndex(parent));
  }
  file_blob->asyncRunMethod<dbif::BlobParseRequest>(this, parser, offset,
      parent_chunk);
}

std::shared_ptr<data::BinData> NodeTreeModel::binData(data::NodeID id) {
  Node* node = node_tree_->node(id);
  if (node) {
    auto bin_data = node->binData("data");
    if (bin_data) {
      return bin_data;
    }
  }

  return std::make_shared<data::BinData>(8, 0, nullptr);
}

QModelIndex NodeTreeModel::indexFromPos(int64_t pos,
    const QModelIndex &parent) {
  auto node = nodeFromIndex(parent);

  if (node == nullptr) {
    return QModelIndex();
  }

  for (unsigned child_index = 0;
      child_index < node->childrenVect().size();
      child_index++) {
    auto child = node->childrenVect()[child_index];
    int64_t begin = child->start();
    int64_t end = child->end();
    if (pos >= begin && pos < end) {
      return indexFromId(child->id());
    }
  }

  return QModelIndex();
}

bool NodeTreeModel::setData(const QModelIndex& index, const QVariant& value,
    int role) {
  if (role == Qt::EditRole && index.column() == COLUMN_INDEX_COMMENT) {
    Node* node = nodeFromIndex(index);
    if (node) {
      node->setComment(value.toString());
      return true;
    }
  }

  return false;
}

Qt::ItemFlags NodeTreeModel::flags(const QModelIndex& index) const {
  Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

  if (index.column() == COLUMN_INDEX_COMMENT) {
    flags |= Qt::ItemIsEditable;
  }

  return flags;
}

bool NodeTreeModel::isRemovable(const QModelIndex &index) {
  return true;
}

bool NodeTreeModel::removeRows(int row, int count, const QModelIndex& parent) {
  Node* node = nodeFromIndex(parent);
  if (node) {
    auto children = node->childrenVect();
    if (unsigned(row + count) <= children.size()) {
      for (int i = row; i < row + count; ++i) {
          children[i]->deleteNode();
      }

      return true;
    }
  }

  return false;
}

QIcon NodeTreeModel::icon(QModelIndex index) const {
  return QIcon();
}

QColor NodeTreeModel::color(int colorIndex) const {
  return util::settings::theme::chunkBackground(colorIndex);
}

QVariant NodeTreeModel::positionColumnData(Node* node, int role) const {
  if (role == Qt::DisplayRole) {
    return zeroPaddedHexNumber(node->start()) + ":"
        + zeroPaddedHexNumber(node->end());
  } else if (role == Qt::FontRole) {
#ifdef Q_OS_WIN32
    return QFont("Courier", 10);
#else
    return QFont("Monospace", 10);
#endif
  } else if (role == ROLE_BEGIN) {
    return QString::number(node->start());
  } else if (role == ROLE_END) {
    return QString::number(node->end());
  }
  return QVariant();
}

QVariant NodeTreeModel::valueColumnData(Node* node, int role) const {
  if (role == Qt::DisplayRole) {
    // TODO chunk data item's value
    return QString("");
  }

  return QVariant();
}

/*****************************************************************************/
/* TopLevelResourcesModel */
/*****************************************************************************/

TopLevelResourcesModel::TopLevelResourcesModel(NodeTree* node_tree,
    data::NodeID root, QObject* parent)
    : NodeTreeModelBase(node_tree, root, parent) {
}

TopLevelResourcesModel::~TopLevelResourcesModel() {
}

int TopLevelResourcesModel::columnCount(const QModelIndex &parent) const {
  return 2;
}

QVariant TopLevelResourcesModel::data(const QModelIndex& index, int role) const {
  auto node = nodeFromIndex(index);

  if (node == nullptr || role != Qt::DisplayRole) {
    return QVariant();
  }

  if (index.column() == 0) {
    QString path("[no path available]");
    node->getQStringAttr("path", path);
    return QVariant(path);
  } else if (index.column() == 1) {
    return QVariant(node->id().toHexString());
  }

  return QVariant();
}

bool TopLevelResourcesModel::hasChildren(const QModelIndex &parent) const {
  if (parent.isValid()) {
    Node* node = nodeFromIndex(parent);
    if (node
        && node->id() == *data::NodeID::getRootNodeId()
        && !node->childrenVect().empty()) {
      return true;
    }
  }

  return false;
}

QVariant TopLevelResourcesModel::headerData(int section,
    Qt::Orientation orientation, int role) const {
  if (orientation != Qt::Orientation::Horizontal
      || role != Qt::DisplayRole) {
    return QVariant();
  }

  if (section == 0) {
    return QString("Path");
  } else if (section == 1) {
    return QString("ID");
  }

  return QVariant();
}

int TopLevelResourcesModel::rowCount(const QModelIndex &parent) const {
  Node* node = nodeFromIndex(parent);

  if (node && node->id() == *data::NodeID::getRootNodeId()) {
    return node->childrenVect().size();
  }

  return 0;
}

} // namespace client
} // namespace veles
