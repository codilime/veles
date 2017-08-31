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
#pragma once

#include <QAbstractItemModel>
#include <QBuffer>
#include <QByteArray>
#include <QObject>
#include <QString>
#include <QStringList>

#include "data/bindata.h"
#include "dbif/types.h"
#include "ui/fileblobitem.h"

namespace veles {
namespace ui {

class FileBlobModel : public QAbstractItemModel {
  Q_OBJECT

 public:
  explicit FileBlobModel(const dbif::ObjectHandle& fileBlob,
                         const QStringList& path = {},
                         QObject* parent = nullptr);

  QModelIndex index(int row, int column,
                    const QModelIndex& parent) const override;
  QModelIndex parent(const QModelIndex& child_index) const override;
  int rowCount(const QModelIndex& parent) const override;
  int columnCount(const QModelIndex& parent) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role) const override;
  bool setData(const QModelIndex& index, const QVariant& value,
               int role) override;
  bool removeRows(int row, int count, const QModelIndex& parent) override;

  void addChunk(const QString& name, const QString& type,
                const QString& comment, uint64_t start, uint64_t end,
                const QModelIndex& index = QModelIndex());

  QModelIndex indexFromPos(uint64_t pos,
                           const QModelIndex& parent = QModelIndex());

  const data::BinData& binData() const { return binData_; }
  bool isRemovable(const QModelIndex& index = QModelIndex());
  void uploadNewData(const data::BinData& bindata, uint64_t offset = 0);
  void parse(const QString& parser = "", qint64 offset = 0,
             const QModelIndex& parent = QModelIndex());

  dbif::ObjectHandle blob(const QModelIndex& index = QModelIndex());
  QStringList path() { return path_; }

  static const Qt::ItemDataRole ROLE_BEGIN = Qt::UserRole;
  static const Qt::ItemDataRole ROLE_END =
      static_cast<Qt::ItemDataRole>(Qt::UserRole + 1);

  static const int COLUMN_INDEX_MAIN = 0;
  static const int COLUMN_INDEX_VALUE = 1;
  static const int COLUMN_INDEX_COMMENT = 2;
  static const int COLUMN_INDEX_POS = 3;

 signals:
  void newBinData();

 private:
  FileBlobItem* item_;
  dbif::ObjectHandle fileBlob_;
  dbif::InfoPromise* bytesPromise_;
  size_t bytesCount_;
  QStringList path_;

  data::BinData binData_;

  QColor color(int colorIndex) const;
  FileBlobItem* itemFromIndex(const QModelIndex& index) const;
  QModelIndex indexFromItem(FileBlobItem* item) const;
  void emitDataChanged(FileBlobItem* item);
  QVariant positionColumnData(FileBlobItem* item, int role) const;
  QVariant valueColumnData(FileBlobItem* item, int role) const;

 private slots:
  void gotDescriptionResponse(const veles::dbif::PInfoReply& reply);
  void gotBytesResponse(const veles::dbif::PInfoReply& reply);
};

}  // namespace ui
}  // namespace veles
