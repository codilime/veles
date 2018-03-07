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

#include "ui/disasm/tab.h"
#include <QToolButton>
#include <QtWidgets/QWidgetAction>

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
  // TODO(malpunek): translation
  tool_bar_ = new QToolBar("Main Disasm Toolbar");

  column_display_menu_ = new QMenu();
  column_display_menu_->clear();

  auto toggleCommentsAction =
      column_display_menu_->addAction(QString("Comments"));
  toggleCommentsAction->setCheckable(true);
  toggleCommentsAction->setChecked(true);
  connect(toggleCommentsAction, &QAction::changed,
          [&] { this->widget.toggleColumn(Row::ColumnName::Comments); });
  auto toggleChunksAction = column_display_menu_->addAction(QString("Chunks"));
  toggleChunksAction->setCheckable(true);
  toggleChunksAction->setChecked(true);
  connect(toggleChunksAction, &QAction::changed,
          [&] { this->widget.toggleColumn(Row::ColumnName::Chunks); });
  auto toggleAdressesAction =
      column_display_menu_->addAction(QString("Adresses"));
  toggleAdressesAction->setCheckable(true);
  toggleAdressesAction->setChecked(true);
  connect(toggleAdressesAction, &QAction::changed,
          [&] { this->widget.toggleColumn(Row::ColumnName::Address); });

  auto column_toggle = new QToolButton();
  column_toggle->setPopupMode(QToolButton::InstantPopup);

  // TODO(malpunek): change icon
  column_toggle->setIcon(QIcon(":/images/parse.png"));
  column_toggle->setMenu(column_display_menu_);

  auto widget_action = new QWidgetAction(tool_bar_);
  widget_action->setDefaultWidget(column_toggle);
  tool_bar_->addAction(widget_action);
  addToolBar(tool_bar_);
}

}  // namespace disasm
}  // namespace ui
}  // namespace veles
