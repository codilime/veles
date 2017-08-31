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
#include <QAction>
#include <QColorDialog>
#include <QFileDialog>
#include <QGroupBox>
#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardItemModel>
#include <QToolBar>
#include <QTreeView>
#include <QVBoxLayout>
#include <QWidgetAction>

#include "visualization/panel.h"

#include "dbif/info.h"
#include "dbif/types.h"
#include "dbif/universe.h"
#include "ui/hexeditwidget.h"
#include "ui/nodetreewidget.h"
#include "ui/nodewidget.h"
#include "ui/veles_mainwindow.h"

#include "util/icons.h"
#include "util/settings/hexedit.h"
#include "util/settings/shortcuts.h"

namespace veles {
namespace ui {

using util::settings::shortcuts::ShortcutsModel;

/*****************************************************************************/
/* Public methods */
/*****************************************************************************/

NodeTreeWidget::NodeTreeWidget(
    MainWindowWithDetachableDockWidgets* main_window,
    const QSharedPointer<FileBlobModel>& data_model,
    const QSharedPointer<QItemSelectionModel>& selection_model)
    : View("Node tree", ":/images/show_node_tree.png"),
      main_window_(main_window),
      data_model_(data_model),
      selection_model_(selection_model) {
  tree_view_ = new QTreeView();
  tree_view_->setModel(data_model_.data());
  tree_view_->setSelectionModel(selection_model_.data());

  createActions();
  createToolBars();

  this->setCentralWidget(tree_view_);

  setupDataModelHandlers();

  tree_view_->setColumnWidth(0, 190);
  tree_view_->setColumnWidth(1, 140);
  tree_view_->header()->setStretchLastSection(true);

  setupTreeViewHandlers();

  registered_line_edit_ = nullptr;

  NodeTreeWidget::reapplySettings();
  setWindowTitle(data_model_->path().join(" : "));

  // We don't need this now, but it might be useful in the future.
  // connect(&parsers_menu_, &QMenu::triggered, this, &NodeTreeWidget::parse);
  setParserIds(dynamic_cast<VelesMainWindow*>(
                   MainWindowWithDetachableDockWidgets::getFirstMainWindow())
                   ->parsersList());
}

void NodeTreeWidget::setParserIds(const QStringList& ids) {
  parsers_ids_ = ids;

  // We don't need this now, but it might be useful in the future.
  // initParsersMenu();
}

/*****************************************************************************/
/* Private UI methods */
/*****************************************************************************/

void NodeTreeWidget::createActions() {
  //  Not implemented yet.
  //  undo_act_ = new QAction(QIcon(":/images/undo.png"), tr("&Undo"), this);
  //  undo_act_->setShortcuts(QKeySequence::Undo);
  //  undo_act_->setEnabled(false);

  //  redo_act_ = new QAction(QIcon(":/images/redo.png"), tr("&Redo"), this);
  //  redo_act_->setShortcuts(QKeySequence::Redo);
  //  redo_act_->setEnabled(false);
}

void NodeTreeWidget::createToolBars() {
  // Not implemented yet.
  // edit_tool_bar_ = new QToolBar(tr("Edit"));
  // edit_tool_bar_->addAction(undo_act_);
  // edit_tool_bar_->addAction(redo_act_);
  // addToolBar(edit_tool_bar_);

  // tools_tool_bar_ = new QToolBar(tr("Tools"));
  // addToolBar(tools_tool_bar_);
}

void NodeTreeWidget::addChunk(const QString& name, const QString& type,
                              const QString& comment, uint64_t start,
                              uint64_t end, const QModelIndex& index) {
  data_model_->addChunk(name, type, comment, start, end, index);
}

void NodeTreeWidget::setupTreeViewHandlers() {
  tree_view_->setContextMenuPolicy(Qt::ActionsContextMenu);
  remove_action_ = ShortcutsModel::getShortcutsModel()->createQAction(
      util::settings::shortcuts::REMOVE_CHUNK, this,
      Qt::WidgetWithChildrenShortcut);
  tree_view_->addAction(remove_action_);
  remove_action_->setEnabled(false);

  connect(tree_view_->selectionModel(), &QItemSelectionModel::currentChanged,
          this, &NodeTreeWidget::currentSelectionChanged);

  connect(remove_action_, &QAction::triggered, this,
          &NodeTreeWidget::removeChunk);

  connect(tree_view_, &QAbstractItemView::doubleClicked,
          [this](const QModelIndex& index) {
            auto mainIndex = data_model_->index(
                index.row(), FileBlobModel::COLUMN_INDEX_MAIN, index.parent());
            dbif::ObjectHandle new_root = data_model_->blob(mainIndex);
            if (!new_root.isNull()) {
              auto new_path = data_model_->path();
              new_path.append(mainIndex.data().toString());
              QSharedPointer<FileBlobModel> new_model(
                  new FileBlobModel(new_root, new_path));
              QSharedPointer<QItemSelectionModel> new_selection_model(
                  new QItemSelectionModel(new_model.data()));

              auto* node_widget =
                  new NodeWidget(main_window_, new_model, new_selection_model);
              main_window_->addTab(node_widget, new_model->path().join(" : "),
                                   nullptr);
            }
          });
}

void NodeTreeWidget::setupDataModelHandlers() {
  connect(data_model_.data(), &FileBlobModel::newBinData, this,
          &NodeTreeWidget::newBinData);
}

/*****************************************************************************/
/* Other private methods */
/*****************************************************************************/

void NodeTreeWidget::initParsersMenu() {
  parsers_menu_.clear();
  parsers_menu_.addAction("auto");
  parsers_menu_.addSeparator();
  for (const auto& id : parsers_ids_) {
    parsers_menu_.addAction(id);
  }
}

/*****************************************************************************/
/* Private Slots */
/*****************************************************************************/

void NodeTreeWidget::parse(QAction* action) {
  if (action->text() == "auto") {
    data_model_->parse();
  } else {
    data_model_->parse(action->text());
  }
}

void NodeTreeWidget::removeChunk() {
  auto selectedChunk = tree_view_->selectionModel()->currentIndex();

  auto result = QMessageBox::warning(
      this, tr("remove chunk"),
      tr("Remove chunk %1 ?").arg(selectedChunk.data().toString()),
      QMessageBox::Yes | QMessageBox::No);
  if (result != QMessageBox::Yes) {
    return;
  }

  data_model_->removeRow(selectedChunk.row(), selectedChunk.parent());
}

void NodeTreeWidget::currentSelectionChanged(const QModelIndex& currentIndex) {
  remove_action_->setEnabled(data_model_->isRemovable(currentIndex));
}

void NodeTreeWidget::newBinData() {}

void NodeTreeWidget::registerLineEdit(QLineEdit* line_edit) {
  registered_line_edit_ = line_edit;
}

void NodeTreeWidget::updateLineEditWithAddress(qint64 address) {
  if (registered_line_edit_ != nullptr) {
    registered_line_edit_->setText(QString::number(address));
  }
  registered_line_edit_ = nullptr;
}

}  // namespace ui
}  // namespace veles
