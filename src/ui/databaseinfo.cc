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
#include "ui_databaseinfo.h"

#include "dbif/info.h"
#include "dbif/promise.h"
#include "dbif/universe.h"

namespace veles {
namespace ui {

DatabaseInfo::DatabaseInfo(dbif::ObjectHandle database, QWidget* parent)
    : QWidget(parent),
      ui_(new Ui::DatabaseInfo),
      database_(database),
      childrenPromise_(nullptr) {
  ui_->setupUi(this);

  model_ = new QStandardItemModel(this);
  ui_->resourcesListView->setModel(model_);

  connect(ui_->goButton, SIGNAL(clicked()), this, SLOT(goClicked()));
  connect(ui_->newButton, SIGNAL(clicked()), this, SLOT(newClicked()));

  connect(ui_->resourcesListView, &QAbstractItemView::clicked, [this]() {
    ui_->goButton->setEnabled(true);
  });

  connect(ui_->resourcesListView, &QAbstractItemView::doubleClicked, this , &DatabaseInfo::goClicked);

  ui_->goButton->setEnabled(false);

  subscribeChildren();
}

void DatabaseInfo::subscribeChildren() {
  childrenPromise_ = database_->asyncSubInfo<dbif::ChildrenRequest>(this);
  connect(childrenPromise_, SIGNAL(gotInfo(veles::dbif::PInfoReply)), this,
          SLOT(gotChildrenResponse(veles::dbif::PInfoReply)));
}

DatabaseInfo::~DatabaseInfo() { delete ui_; }

void DatabaseInfo::goClicked() {
  auto selectedIndex = ui_->resourcesListView->currentIndex();
  if (selectedIndex.isValid()) {
    emit goFile(index_to_object_[selectedIndex.row()],
                selectedIndex.data().toString());
  }
}

void DatabaseInfo::newClicked() { emit newFile(); }

void DatabaseInfo::gotChildrenResponse(veles::dbif::PInfoReply reply) {
  object_to_index_.clear();
  index_to_object_.clear();
  model_->clear();
  ui_->goButton->setEnabled(false);

  auto objects =
      reply.dynamicCast<dbif::ChildrenRequest::ReplyType>()->objects;
  int next_index = 0;
  for (const auto& object : objects) {
    if (object->type() != dbif::FILE_BLOB || object_to_index_.contains(object)) {
      continue;
    }

    auto item = new QStandardItem("loading...");
    item->setEditable(false);
    model_->appendRow(item);
    auto index = model_->index(next_index, 0);
    object_to_index_[object] = index;
    index_to_object_.append(object);
    next_index++;

    auto description_promise =
        object->asyncSubInfo<dbif::DescriptionRequest>(this);

    connect(description_promise, &dbif::InfoPromise::gotInfo,
        [this, object](veles::dbif::PInfoReply reply) {
          if (auto file_blob_description =
                reply.dynamicCast<dbif::FileBlobDescriptionReply>()) {
            if (object_to_index_.contains(object)) {
              model_->setData(object_to_index_[object],
                              file_blob_description->path);
            }
          }
        }
    );
  }
}

}  // namespace ui
}  // namespace veles
