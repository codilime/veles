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

#include <QToolButton>
#include <QtWidgets/QWidgetAction>
#include "ui/disasm/tab.h"

namespace veles {
namespace ui {
namespace disasm {

Tab::Tab() : IconAwareView("", "") {
  //  auto* dock_address_column = new DockWidget;
  //  dock_address_column->setWidget(&widget);
  //  addDockWidget(Qt::LeftDockWidgetArea, dock_address_column);
  setCentralWidget(&widget);
  createToolbars();
}

void Tab::createToolbars() {
  tool_bar = new QToolBar("Main Disasm Toolbar"); // TODO(malpunek): translation

  column_display_menu_= new QMenu();
  column_display_menu_ -> clear();
  column_display_menu_ -> addAction("auto");
  column_display_menu_ -> addSeparator();
  column_display_menu_ -> addAction(QString("Ala ma kota")) -> setCheckable(true);
  column_display_menu_ -> addAction(QString("Ala ma kota 2")) -> setCheckable(true);

  auto column_toggle = new QToolButton();
  column_toggle->setPopupMode(QToolButton::InstantPopup);

  column_toggle->setIcon(QIcon(":/images/parse.png"));
  column_toggle -> setMenu(column_display_menu_);

  auto widget_action = new QWidgetAction(tool_bar);
  widget_action -> setDefaultWidget(column_toggle);
  tool_bar -> addAction(widget_action);
  addToolBar(tool_bar);



}

}  // namespace disasm
}  // namespace ui
}  // namespace veles
