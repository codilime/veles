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
#include "ui/databaseinfo.h"
#include "ui_databaseinfo.h"

#include "dbif/info.h"
#include "dbif/promise.h"
#include "dbif/universe.h"

namespace veles {
namespace ui {

DatabaseInfo::DatabaseInfo(
    QSharedPointer<client::TopLevelResourcesModel> model,
    QWidget *parent)
    : QWidget(parent), ui_(new Ui::DatabaseInfo) {
  ui_->setupUi(this);
  ui_->resourcesListView->setModel(model.data());
  ui_->resourcesListView->setRootIndex(model->indexFromId(
      *data::NodeID::getRootNodeId()));

  connect(ui_->goButton, SIGNAL(clicked()), this, SLOT(goClicked()));
  connect(ui_->newButton, SIGNAL(clicked()), this, SLOT(newClicked()));

  connect(ui_->resourcesListView, &QAbstractItemView::clicked, [this]() {
    ui_->goButton->setEnabled(true);
  });

  connect(model.data(), &QAbstractItemModel::modelReset, [this, model]() {
    ui_->resourcesListView->setRootIndex(model->indexFromId(
        *data::NodeID::getRootNodeId()));
  });

  connect(ui_->resourcesListView, &QAbstractItemView::doubleClicked,
      this , &DatabaseInfo::goClicked);

  ui_->goButton->setEnabled(false);
}

DatabaseInfo::~DatabaseInfo() { delete ui_; }

void DatabaseInfo::goClicked() {
  auto selectedIndex = ui_->resourcesListView->currentIndex();
  if (selectedIndex.isValid()) {
    emit goFile(
        selectedIndex.sibling(selectedIndex.row(), 1).data().toString(),
        selectedIndex.data().toString());
  }
}

void DatabaseInfo::newClicked() {
  emit newFile();
}

}  // namespace ui
}  // namespace veles
