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

#include <QObject>

#include <data/field.h>

#include "dbif/types.h"
#include "ui/fileblobitem.h"

namespace veles {
namespace ui {

class SimpleFileBlobItem : public FileBlobItem {
  Q_OBJECT

 public:
  SimpleFileBlobItem(QString name, QString comment, QObject* parent = nullptr)
      : FileBlobItem(name, "", comment, 0, 0, parent) {}
  bool range(uint64_t* /*start*/, uint64_t* /*end*/) const override {
    return false;
  }
};

}  // namespace ui
}  // namespace veles
