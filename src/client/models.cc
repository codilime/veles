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
/* NodeTreeModel */
/*****************************************************************************/

NodeTreeModel::NodeTreeModel(NodeTree* node_tree, data::NodeID root,
    QObject* parent)
    : QAbstractItemModel(parent), node_tree_(node_tree), root_(root) {
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

bool NodeTreeModel::NodeTreeModel::hasChildren(const QModelIndex &parent) const {
  if (parent.isValid()) {
    Node* node = nodeFromIndex(parent);
    if (node && !node->childrenVect().empty()) {
      return true;
    }
  }

  return false;
}

QVariant NodeTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
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

QModelIndex NodeTreeModel::index(int row, int column, const QModelIndex &parent) const {
  Node* node = nodeFromIndex(parent);

  if (node) {
    if (node->childrenVect().size() > (unsigned)row) {
      return createIndex(row, column, node->childrenVect()[row]);
    }
  }
  return QModelIndex();
}

QModelIndex NodeTreeModel::parent(const QModelIndex &index) const {
  if (index.isValid()) {
    Node* node = parentNodeFromIndex(index);
    if (node) {
      return createIndex(node->index(), 0, node);
    }
  }

  return QModelIndex();
}

int NodeTreeModel::rowCount(const QModelIndex &parent) const {
  Node* node = nodeFromIndex(parent);

  if (node) {
    return node->childrenVect().size();
  }

  return 0;
}

QModelIndex NodeTreeModel::indexFromId(data::NodeID id) const {
  Node* node = node_tree_->node(id);
  if (node) {
    return createIndex(node->index(), 0, node_tree_->node(root_));
  } else {
    return QModelIndex();
  }
}

void NodeTreeModel::addChunk(QString name, QString type, QString comment,
    uint64_t start, uint64_t end, const QModelIndex& index) {
  // TODO
}

void NodeTreeModel::parse(QString parser, qint64 offset,
    const QModelIndex& parent) {
  // TODO
  //dbif::ObjectHandle parent_chunk;
  //if (parent.isValid()) {
  //  parent_chunk = itemFromIndex(parent)->objectHandle();
  //}
  //fileBlob_->asyncRunMethod<dbif::BlobParseRequest>(this, parser, offset,
  //                                                  parent_chunk);
}

QIcon NodeTreeModel::icon(QModelIndex index) const {
  return QIcon();
}

QColor NodeTreeModel::color(int colorIndex) const {
  return util::settings::theme::chunkBackground(colorIndex);
}

QVariant NodeTreeModel::positionColumnData(Node* node, int role) const {
  if (role == Qt::DisplayRole) {
    return zeroPaddedHexNumber(node->start()) + ":" + zeroPaddedHexNumber(node->end());
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
    // TODO
    //return item->value();
    return QString("TODO value");
  }

  return QVariant();
}

Node* NodeTreeModel::parentNodeFromIndex(const QModelIndex &index) const {
  Node* node = nodeFromIndex(index);
  return node ? node->parent() : nullptr;
}

Node* NodeTreeModel::nodeFromIndex(const QModelIndex &index) const {
  if (index.isValid()) {
    return reinterpret_cast<Node*>(index.internalPointer());
  } else {
    return nullptr;
  }
}

} // namespace client
} // namespace veles
