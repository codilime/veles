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

#include <QMap>
#include <QStandardItemModel>
#include <QVector>
#include <QWidget>
#include "data/types.h"
#include "client/models.h"

namespace Ui {
class DatabaseInfo;
}

namespace veles {
namespace ui {

class DatabaseInfo : public QWidget {
  Q_OBJECT

 public:
  explicit DatabaseInfo(
      QSharedPointer<client::TopLevelResourcesModel> model,
      QWidget *parent = 0);
  ~DatabaseInfo();

 private:
  Ui::DatabaseInfo *ui_;
  QSharedPointer<client::TopLevelResourcesModel> model_;

 signals:
  void goFile(QString id, QString file_name);
  void newFile();

 private slots:
  void goClicked();
  void newClicked();
};

}  // namespace ui
}  // namespace veles
