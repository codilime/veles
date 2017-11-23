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
#include "ui/databaseinfo.h"

#include "dbif/info.h"
#include "dbif/promise.h"
#include "dbif/universe.h"
#include "ui_databaseinfo.h"

namespace veles {
namespace ui {

DatabaseInfo::DatabaseInfo(const dbif::ObjectHandle& database, QWidget* parent)
    : QWidget(parent),
      ui_(new Ui::DatabaseInfo),
      database_(database),
      childrenPromise_(nullptr) {
  ui_->setupUi(this);

  model_ = new QStandardItemModel(this);
  ui_->resourcesListView->setModel(model_);
  ui_->resourcesListView->setStyleSheet(
      "QListView::item:hover:!selected {"
      "background-color: palette(alternate-base);"
      "}");

  connect(ui_->goButton, &QPushButton::clicked, this, &DatabaseInfo::goClicked);
  connect(ui_->newButton, &QPushButton::clicked, this,
          &DatabaseInfo::newClicked);

  connect(ui_->resourcesListView, &QAbstractItemView::clicked,
          [this]() { ui_->goButton->setEnabled(true); });

  connect(ui_->resourcesListView, &QAbstractItemView::doubleClicked, this,
          &DatabaseInfo::goClicked);

  ui_->goButton->setEnabled(false);

  historyModel_ = new QStandardItemModel(0, 3);

  subscribeChildren();
}

void DatabaseInfo::subscribeChildren() {
  childrenPromise_ = database_->asyncSubInfo<dbif::ChildrenRequest>(this);
  connect(childrenPromise_, &dbif::InfoPromise::gotInfo, this,
          &DatabaseInfo::gotChildrenResponse);
}

DatabaseInfo::~DatabaseInfo() { delete ui_; }

void DatabaseInfo::goClicked() {
  auto selectedIndex = ui_->resourcesListView->currentIndex();
  if (selectedIndex.isValid()) {
    emit goFile(indexToObject_[selectedIndex.row()],
                selectedIndex.data().toString());
  }
}

void DatabaseInfo::newClicked() { emit newFile(); }

void DatabaseInfo::gotChildrenResponse(const veles::dbif::PInfoReply& reply) {
  objectToIndex_.clear();
  indexToObject_.clear();
  model_->clear();
  ui_->goButton->setEnabled(false);

  auto objects = reply.dynamicCast<dbif::ChildrenRequest::ReplyType>()->objects;
  int nextIndex = 0;
  for (auto& object : objects) {
    if (object->type() != dbif::FILE_BLOB || objectToIndex_.contains(object)) {
      continue;
    }

    auto item = new QStandardItem("loading");
    item->setEditable(false);
    model_->appendRow(item);
    auto index = model_->index(nextIndex, 0);
    objectToIndex_[object] = index;
    indexToObject_.append(object);
    nextIndex++;

    auto descriptionPromise =
        object->asyncSubInfo<dbif::DescriptionRequest>(this);

    connect(descriptionPromise, &dbif::InfoPromise::gotInfo,
            [this, object](veles::dbif::PInfoReply reply) {
              if (auto fileBlobDescription =
                      reply.dynamicCast<dbif::FileBlobDescriptionReply>()) {
                if (objectToIndex_.contains(object)) {
                  model_->setData(objectToIndex_[object],
                                  fileBlobDescription->path);
                }
              }
            });
  }
}

}  // namespace ui
}  // namespace veles
