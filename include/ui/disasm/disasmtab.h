/*
* Copyright 2018 CodiLime
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

#include <QDockWidget>
#include <QObject>
#include <QPushButton>

#include "ui/dockwidget.h"

namespace veles {
namespace ui {

class DisasmTab : public IconAwareView {
  Q_OBJECT

  QPushButton tmp;

 public:
  DisasmTab();

  virtual void paintEvent(QPaintEvent* event);
  virtual ~DisasmTab();
};

}  // namespace ui
}  // namespace veles
