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
#include "ui/filters/activatedockeventfilter.h"

#include <QEvent>

#include "ui/dockwidget.h"
#include "ui/mainwindowwithdetachabledockwidgets.h"

namespace veles {
namespace ui {

ActivateDockEventFilter::ActivateDockEventFilter(QObject* parent)
    : QObject(parent) {}

bool ActivateDockEventFilter::eventFilter(QObject* watched, QEvent* event) {
  if (event->type() == QEvent::MouseButtonPress) {
    auto parent = DockWidget::getParentDockWidget(watched);
    if (parent != nullptr) {
      MainWindowWithDetachableDockWidgets::setActiveDockWidget(parent);
    }
  }

  return false;
}

}  // namespace ui
}  // namespace veles
