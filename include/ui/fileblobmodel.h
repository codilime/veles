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
#ifndef FILEBLOBMODEL_H
#define FILEBLOBMODEL_H

#include <QAbstractItemModel>
#include <QBuffer>
#include <QByteArray>
#include <QStringList>
#include <QString>
#include <QObject>

#include "dbif/types.h"
#include "ui/fileblobitem.h"
#include "data/bindata.h"

namespace veles {
namespace ui {

class FileBlobModel : public QAbstractItemModel {
  Q_OBJECT

 public:
  explicit FileBlobModel(dbif::ObjectHandle fileBlob_, const QStringList &path = {}, QObject *parent = 0);

  QModelIndex index(int row, int column,
                    const QModelIndex &parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex &child) const override;
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index,
                int role = Qt::DisplayRole) const override;
  Qt::ItemFlags flags(const QModelIndex &index) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role) const override;
  bool setData(const QModelIndex &index, const QVariant &value,
               int role = Qt::EditRole) override;
  bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

  void addChunk(QString name, QString type, QString comment, uint64_t start,
                uint64_t end, const QModelIndex &index = QModelIndex());

  QModelIndex indexFromPos(uint64_t pos,
                           const QModelIndex &parent = QModelIndex());

  const data::BinData& binData() {return binData_;}
  bool isRemovable(const QModelIndex &index = QModelIndex());
  void uploadNewData(const QByteArray &buf);

  dbif::ObjectHandle blob(const QModelIndex &index = QModelIndex());
  QStringList path() {return path_;};

  static const Qt::ItemDataRole ROLE_BEGIN = Qt::UserRole;
  static const Qt::ItemDataRole ROLE_END = (Qt::ItemDataRole)(Qt::UserRole + 1);

  static const int COLUMN_INDEX_MAIN = 0;
  static const int COLUMN_INDEX_VALUE = 1;
  static const int COLUMN_INDEX_COMMENT = 2;
  static const int COLUMN_INDEX_POS = 3;

 signals:
  void newBinData();

 private:
  FileBlobItem *item_;
  dbif::ObjectHandle fileBlob_;
  dbif::InfoPromise *bytesPromise_;
  size_t bytesCount_;
  QStringList path_;

  data::BinData binData_;

  QColor color(int colorIndex) const;
  FileBlobItem *itemFromIndex(const QModelIndex &index) const;
  QModelIndex indexFromItem(FileBlobItem *item) const;
  void emitDataChanged(FileBlobItem *item);
  QVariant positionColumnData(FileBlobItem *item, int role) const;
  QVariant valueColumnData(FileBlobItem *item, int role) const;

 private slots:
  void gotDescriptionResponse(veles::dbif::PInfoReply reply);
  void gotBytesResponse(veles::dbif::PInfoReply reply);
};

}  // namespace ui
}  // namespace veles

#endif  // FILEBLOBMODEL_H
