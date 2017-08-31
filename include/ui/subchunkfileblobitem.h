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

#include <QList>
#include <QObject>

#include <data/field.h>

#include "dbif/types.h"
#include "ui/fileblobitem.h"

namespace veles {
namespace ui {

class SubchunkFileBlobItem : public FileBlobItem {
  Q_OBJECT

 public:
  explicit SubchunkFileBlobItem(const dbif::ObjectHandle& obj,
                                QObject* parent = nullptr);

  int childrenCount() override;
  void setComment(const QString& comment) override;
  QString name() override;

 private:
  bool infoSubscribed_;
  void subscribeInfo();

 private slots:
  void gotChunkDataResponse(const veles::dbif::PInfoReply& reply);
  void gotChunkDescriptionResponse(const veles::dbif::PInfoReply& reply);
  void gotError(const veles::dbif::PError& error);
};

}  // namespace ui
}  // namespace veles
