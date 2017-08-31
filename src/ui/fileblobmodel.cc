/*
 * Copyright 2016 CodiLime
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
#include <QColor>
#include <QFont>
#include <QSize>
#include "dbif/method.h"
#include "dbif/types.h"
#include "dbif/universe.h"

#include "ui/fileblobmodel.h"
#include "ui/rootfileblobitem.h"

#include "util/settings/theme.h"

namespace veles {
namespace ui {

QColor FileBlobModel::color(int colorIndex) const {
  return util::settings::theme::chunkBackground(colorIndex);
}

FileBlobItem* FileBlobModel::itemFromIndex(const QModelIndex& index) const {
  if (!index.isValid()) {
    return item_;
  }

  auto parentLoader = reinterpret_cast<FileBlobItem*>(index.internalPointer());

  if (parentLoader != nullptr && parentLoader->childrenCount() > index.row()) {
    return parentLoader->child(index.row());
  }

  return nullptr;
}

QModelIndex FileBlobModel::indexFromItem(FileBlobItem* item) const {
  if (item->parent() == this) {
    return {};
  }
  auto* parentLoader = reinterpret_cast<FileBlobItem*>(item->parent());

  int row = parentLoader->childIndex(item);
  if (row < 0) {
    return {};
  }
  return createIndex(row, COLUMN_INDEX_MAIN, parentLoader);
}

void FileBlobModel::emitDataChanged(FileBlobItem* item) {
  auto firstIndex = indexFromItem(item);

  if (!firstIndex.isValid()) {
    return;
  }

  auto lastIndex = createIndex(firstIndex.row(), columnCount(QModelIndex()),
                               firstIndex.internalPointer());

  emit dataChanged(firstIndex, lastIndex);
}

void FileBlobModel::addChunk(const QString& name, const QString& type,
                             const QString& comment, uint64_t start,
                             uint64_t end, const QModelIndex& index) {
  dbif::ObjectHandle parent;
  if (index.isValid()) {
    parent = itemFromIndex(index)->objectHandle();
  }

  auto promise = fileBlob_->asyncRunMethod<dbif::ChunkCreateRequest>(
      this, name, type, parent, start, end);
  connect(
      promise, &dbif::MethodResultPromise::gotResult,
      [this, comment](dbif::PMethodReply reply) {
        auto newChunk =
            reply.dynamicCast<dbif::ChunkCreateRequest::ReplyType>()->object;
        newChunk->asyncRunMethod<dbif::SetCommentRequest>(this, comment);
      });
}

FileBlobModel::FileBlobModel(const dbif::ObjectHandle& fileBlob,
                             const QStringList& path, QObject* parent)
    : QAbstractItemModel(parent),
      fileBlob_(fileBlob),
      bytesPromise_(nullptr),
      bytesCount_(0),
      path_(path) {
  item_ = new RootFileBlobItem(fileBlob, this);

  connect(item_, &FileBlobItem::removingChildren,
          [this](FileBlobItem* item, bool isBefore) {
            if (isBefore) {
              beginRemoveRows(indexFromItem(item), 0, item->childrenCount());
            } else {
              endRemoveRows();
              emitDataChanged(item);
            }
          });

  connect(item_, &FileBlobItem::insertingChildren,
          [this](FileBlobItem* item, bool isBefore, int count) {
            if (isBefore) {
              beginInsertRows(indexFromItem(item), 0, count);
            } else {
              endInsertRows();
              emitDataChanged(item);
            }
          });

  connect(item_, &FileBlobItem::dataUpdated,
          [this](FileBlobItem* item) { emitDataChanged(item); });

  dbif::DescriptionRequest req;
  auto descriptionPromise =
      fileBlob_->asyncSubInfo<dbif::DescriptionRequest>(this, req);
  connect(descriptionPromise, SIGNAL(gotInfo(veles::dbif::PInfoReply)), this,
          SLOT(gotDescriptionResponse(veles::dbif::PInfoReply)));
}

void FileBlobModel::gotBytesResponse(const veles::dbif::PInfoReply& reply) {
  if (auto bytesReply = reply.dynamicCast<dbif::BlobDataRequest::ReplyType>()) {
    binData_ = bytesReply->data;
    emit newBinData();
  }
}

void FileBlobModel::gotDescriptionResponse(
    const veles::dbif::PInfoReply& reply) {
  if (auto description = reply.dynamicCast<dbif::BlobDescriptionReply>()) {
    if (bytesCount_ != description->size) {
      bytesCount_ = description->size;
      delete bytesPromise_;
      bytesPromise_ =
          fileBlob_->asyncSubInfo<dbif::BlobDataRequest>(this, 0, bytesCount_);
      connect(bytesPromise_, SIGNAL(gotInfo(veles::dbif::PInfoReply)), this,
              SLOT(gotBytesResponse(veles::dbif::PInfoReply)));
    }
  }
}

QVariant FileBlobModel::headerData(int section, Qt::Orientation orientation,
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

QModelIndex FileBlobModel::index(int row, int column,
                                 const QModelIndex& parent) const {
  return createIndex(row, column, itemFromIndex(parent));
}

QModelIndex FileBlobModel::parent(const QModelIndex& child_index) const {
  if (!child_index.isValid()) {
    return {};
  }

  auto* loader = reinterpret_cast<FileBlobItem*>(child_index.internalPointer());
  return indexFromItem(loader);
}

int FileBlobModel::rowCount(const QModelIndex& parent) const {
  auto loader = itemFromIndex(parent);

  if (loader == nullptr || parent.column() > COLUMN_INDEX_MAIN) {
    return 0;
  }

  return loader->childrenCount();
}

int FileBlobModel::columnCount(const QModelIndex& /*parent*/) const {
  return 4;
}

QString zeroPaddedHexNumber(uint64_t number) {
  auto num = QString::number(number, 16);
  while (num.length() < 4) {
    num = "0" + num;
  }
  return num;
}

QVariant FileBlobModel::positionColumnData(FileBlobItem* item, int role) const {
  uint64_t begin, end;

  if (!item->range(&begin, &end)) {
    return QVariant();
  }

  if (role == Qt::DisplayRole) {
    return zeroPaddedHexNumber(begin) + ":" + zeroPaddedHexNumber(end);
  }
  if (role == Qt::FontRole) {
#ifdef Q_OS_WIN32
    return QFont("Courier", 10);
#else
    return QFont("Monospace", 10);
#endif
  }
  if (role == ROLE_BEGIN) {
    return QString::number(begin);
  }
  if (role == ROLE_END) {
    return QString::number(end);
  }
  return QVariant();
}

QVariant FileBlobModel::valueColumnData(FileBlobItem* item, int role) const {
  if (role == Qt::DisplayRole) {
    return item->value();
  }

  return QVariant();
}

QVariant FileBlobModel::data(const QModelIndex& index, int role) const {
  if (role == Qt::SizeHintRole) {
    return QSize(50, 20);
  }

  auto item = itemFromIndex(index);

  if (role == Qt::DecorationRole) {
    if (index.isValid() && index.column() == COLUMN_INDEX_MAIN) {
      if (item != nullptr && !item->icon().isNull()) {
        return item->icon();
      }
      return color(index.row());
    }
    return QVariant();
  }

  if (item == nullptr) {
    return QVariant();
  }
  auto name = item->name();
  auto comment = item->comment();

  if (index.column() == COLUMN_INDEX_COMMENT &&
      (role == Qt::DisplayRole || role == Qt::EditRole)) {
    return comment;
  }

  if (index.column() == COLUMN_INDEX_POS) {
    return positionColumnData(item, role);
  }

  if (index.column() == COLUMN_INDEX_VALUE) {
    return valueColumnData(item, role);
  }

  if (index.column() != COLUMN_INDEX_MAIN) {
    return QVariant();
  }

  if (role == Qt::DisplayRole) {
    return name;
  }
  return QVariant();
}

QModelIndex FileBlobModel::indexFromPos(uint64_t pos,
                                        const QModelIndex& parent) {
  auto loader = itemFromIndex(parent);

  if (loader == nullptr) {
    return {};
  }

  for (const auto loader_child : loader->children()) {
    uint64_t begin, end;
    loader_child->range(&begin, &end);
    if (pos >= begin && pos < end) {
      return indexFromItem(loader_child);
    }
  }

  return {};
}

bool FileBlobModel::setData(const QModelIndex& index, const QVariant& value,
                            int role) {
  if (role == Qt::EditRole && index.column() == COLUMN_INDEX_COMMENT) {
    itemFromIndex(index)->setComment(value.toString());
    return true;
  }

  return false;
}

bool FileBlobModel::removeRows(int row, int count, const QModelIndex& parent) {
  QVector<dbif::ObjectHandle> toRemove;
  auto item = itemFromIndex(parent);
  for (auto index = row; index < row + count; ++index) {
    auto child = item->child(index);
    if (child != nullptr && !child->objectHandle().isNull()) {
      toRemove.append(child->objectHandle());
    }
  }

  for (const auto& obj : toRemove) {
    obj->asyncRunMethod<dbif::DeleteRequest>(this);
  }
  return !toRemove.empty();
}

Qt::ItemFlags FileBlobModel::flags(const QModelIndex& index) const {
  Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

  bool isLeaf = !itemFromIndex(index)->objectHandle();

  if (isLeaf) {
    flags |= Qt::ItemNeverHasChildren;
  }

  if (index.column() == COLUMN_INDEX_COMMENT && !isLeaf) {
    flags |= Qt::ItemIsEditable;
  }

  return flags;
}

void FileBlobModel::uploadNewData(const data::BinData& bindata,
                                  uint64_t offset) {
  fileBlob_->asyncRunMethod<dbif::ChangeDataRequest>(this, offset,
                                                     bindata.size(), bindata);
}

void FileBlobModel::parse(const QString& parser, qint64 offset,
                          const QModelIndex& parent) {
  dbif::ObjectHandle parent_chunk;
  if (parent.isValid()) {
    parent_chunk = itemFromIndex(parent)->objectHandle();
  }
  fileBlob_->asyncRunMethod<dbif::BlobParseRequest>(this, parser, offset,
                                                    parent_chunk);
}

bool FileBlobModel::isRemovable(const QModelIndex& index) {
  auto item = itemFromIndex(index);
  return index.isValid() && item != nullptr && item->isRemovable();
}

dbif::ObjectHandle FileBlobModel::blob(const QModelIndex& index) {
  if (!index.isValid()) {
    return fileBlob_;
  }
  auto item = itemFromIndex(index);
  if (item == nullptr) {
    return dbif::ObjectHandle();
  }

  return item->newRoot();
}

}  // namespace ui
}  // namespace veles
