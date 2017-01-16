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
#ifndef DATABASEINFO_H
#define DATABASEINFO_H

#include <QMap>
#include <QStandardItemModel>
#include <QVector>
#include <QWidget>
#include "data/types.h"
#include "dbif/promise.h"
#include "dbif/types.h"

namespace Ui {
class DatabaseInfo;
}

namespace veles {
namespace ui {

class DatabaseInfo : public QWidget {
  Q_OBJECT

 public:
  explicit DatabaseInfo(dbif::ObjectHandle database, QWidget *parent = 0);
  ~DatabaseInfo();

 private:
  void subscribeChildren();

  Ui::DatabaseInfo *ui_;
  dbif::ObjectHandle database_;
  QMap<dbif::ObjectHandle, QModelIndex> objectToIndex_;
  QVector<dbif::ObjectHandle> indexToObject_;
  QStandardItemModel *model_;
  QStandardItemModel *historyModel_;
  dbif::InfoPromise *childrenPromise_;

 signals:
  void goFile(dbif::ObjectHandle fileblob, QString fileName);
  void newFile();

 private slots:
  void gotChildrenResponse(veles::dbif::PInfoReply reply);
  void goClicked();
  void newClicked();
};

}  // namespace ui
}  // namespace veles

#endif  // DATABASEINFO_H
