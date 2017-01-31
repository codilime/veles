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

#include "visualisation/panel.h"

#include "dbif/info.h"
#include "dbif/types.h"
#include "dbif/universe.h"
#include "ui/hexeditwidget.h"
#include "ui/nodetreewidget.h"
#include "ui/veles_mainwindow.h"

#include "util/settings/hexedit.h"
#include "util/icons.h"

namespace veles {
namespace ui {

/*****************************************************************************/
/* Public methods */
/*****************************************************************************/

NodeTreeWidget::NodeTreeWidget(
    MainWindowWithDetachableDockWidgets *main_window,
    QSharedPointer<FileBlobModel>& data_model,
    QSharedPointer<QItemSelectionModel>& selection_model)
    : View("Node tree", ":/images/show_node_tree.png"),
      main_window_(main_window), data_model_(data_model),
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

  reapplySettings();
  setWindowTitle(data_model_->path().join(" : "));

  connect(&parsers_menu_, &QMenu::triggered, this, &NodeTreeWidget::parse);
  setParserIds(dynamic_cast<VelesMainWindow*>(
        MainWindowWithDetachableDockWidgets::getFirstMainWindow())
        ->parsersList());
}

void NodeTreeWidget::reapplySettings() {
}

void NodeTreeWidget::setParserIds(QStringList ids) {
  parsers_ids_ = ids;
  initParsersMenu();
}

/*****************************************************************************/
/* Private UI methods */
/*****************************************************************************/

void NodeTreeWidget::createActions() {

  upload_act_ = new QAction(QIcon(":/images/upload-32.ico"), tr("&Upload"), this);
  upload_act_->setShortcuts(QKeySequence::Save);
  upload_act_->setStatusTip(tr("Upload changed to database"));
  connect(upload_act_, SIGNAL(triggered()), this, SLOT(uploadChanges()));
  upload_act_->setEnabled(false);

  save_as_act_ = new QAction(QIcon(":/images/save.png"), tr("Save &As..."), this);
  save_as_act_->setShortcuts(QKeySequence::SaveAs);
  save_as_act_->setStatusTip(tr("Save the document under a new name"));
  connect(save_as_act_, SIGNAL(triggered()), this, SLOT(saveAs()));
  save_as_act_->setEnabled(false);

  save_readable_act_ = new QAction(tr("Save &Readable..."), this);
  save_readable_act_->setStatusTip(tr("Save document in readable form"));
  // connect(save_readable_act_, SIGNAL(triggered()), this,
  // SLOT(saveToReadableFile()));

  undo_act_ = new QAction(QIcon(":/images/undo.png"), tr("&Undo"), this);
  undo_act_->setShortcuts(QKeySequence::Undo);
  undo_act_->setEnabled(false);

  redo_act_ = new QAction(QIcon(":/images/redo.png"), tr("&Redo"), this);
  redo_act_->setShortcuts(QKeySequence::Redo);
  redo_act_->setEnabled(false);

  QColor icon_color = palette().color(QPalette::WindowText);
  visualisation_act_ = new QAction(
          util::getColoredIcon(":/images/trigram_icon.png", icon_color),
          tr("&Visualisation"), this);
  visualisation_act_->setToolTip(tr("Visualisation"));
  visualisation_act_->setEnabled(data_model_->binData().size() > 0);
  connect(visualisation_act_, SIGNAL(triggered()), this,
          SLOT(showVisualisation()));

  show_hex_edit_act_ = new QAction(
      QIcon(":/images/show_hex_edit.png"), tr("Show &hex editor"), this);
  show_hex_edit_act_->setToolTip(tr("Hex editor"));
  show_hex_edit_act_->setEnabled(true);
  connect(show_hex_edit_act_, SIGNAL(triggered()), this, SLOT(showHexEditor()));
}

void NodeTreeWidget::createToolBars() {
  //Not implemented yet.
  //file_tool_bar_ = new QToolBar(tr("File"));
  //file_tool_bar_->addAction(upload_act_);
  //file_tool_bar_->addAction(save_as_act_);
  //addToolBar(file_tool_bar_);

  //Not implemented yet.
  //edit_tool_bar_ = new QToolBar(tr("Edit"));
  //edit_tool_bar_->addAction(undo_act_);
  //edit_tool_bar_->addAction(redo_act_);
  //addToolBar(edit_tool_bar_);

  tools_tool_bar_ = new QToolBar(tr("Tools"));
  tools_tool_bar_->addAction(visualisation_act_);

  auto parser_tool_button = new QToolButton();
  parser_tool_button->setMenu(&parsers_menu_);
  parser_tool_button->setPopupMode(QToolButton::InstantPopup);
  parser_tool_button->setIcon(QIcon(":/images/parse.png"));
  parser_tool_button->setText(tr("&Parse"));
  parser_tool_button->setToolTip(tr("Parser"));
  parser_tool_button->setAutoRaise(true);
  auto widget_action = new QWidgetAction(tools_tool_bar_);
  widget_action->setDefaultWidget(parser_tool_button);
  tools_tool_bar_->addAction(widget_action);

  tools_tool_bar_->addAction(show_hex_edit_act_);
  addToolBar(tools_tool_bar_);
}

void NodeTreeWidget::addChunk(QString name, QString type, QString comment,
                          uint64_t start, uint64_t end,
                          const QModelIndex &index) {
  data_model_->addChunk(name, type, comment, start, end, index);
}

void NodeTreeWidget::setupTreeViewHandlers() {

  auto context_menu = new QMenu(tree_view_);
  tree_view_->setContextMenuPolicy(Qt::ActionsContextMenu);
  remove_action_ = new QAction("remove chunk", context_menu);
  tree_view_->addAction(remove_action_);
  remove_action_->setEnabled(false);

  connect(tree_view_->selectionModel(), &QItemSelectionModel::currentChanged,
          this, &NodeTreeWidget::currentSelectionChanged);

  connect(remove_action_, &QAction::triggered,
      this, &NodeTreeWidget::removeChunk);

  connect(tree_view_, &QAbstractItemView::doubleClicked,
      [this](const QModelIndex &index) {
    auto mainIndex = data_model_->index(
        index.row(), FileBlobModel::COLUMN_INDEX_MAIN, index.parent());
    dbif::ObjectHandle new_root = data_model_->blob(mainIndex);
    if (new_root) {
      auto new_path = data_model_->path();
      new_path.append(mainIndex.data().toString());
      QSharedPointer<FileBlobModel> new_model(
          new FileBlobModel(new_root, new_path));
      QSharedPointer<QItemSelectionModel> new_selection_model(
          new QItemSelectionModel(new_model.data()));
      NodeTreeWidget *node_tree = new NodeTreeWidget(main_window_,
          new_model, new_selection_model);
      HexEditWidget *hex_edit = new HexEditWidget(main_window_,
          new_model, new_selection_model);

      DockWidget *sibling1(nullptr), *sibling2(nullptr);
      main_window_->findTwoNonTabifiedDocks(sibling1, sibling2);
      main_window_->addTab(node_tree, node_tree->windowTitle(), sibling1);
      DockWidget* hex_edit_tab = main_window_->addTab(hex_edit,
          new_model->path().join(" : "), sibling2);

      if (sibling1 == sibling2) {
        main_window_->addDockWidget(Qt::RightDockWidgetArea, hex_edit_tab);
      }
    }
  });
}

void NodeTreeWidget::setupDataModelHandlers() {
  connect(data_model_.data(), &FileBlobModel::newBinData, this, &NodeTreeWidget::newBinData);
}

/*****************************************************************************/
/* Other private methods */
/*****************************************************************************/

void NodeTreeWidget::initParsersMenu() {
  parsers_menu_.clear();
  parsers_menu_.addAction("auto");
  parsers_menu_.addSeparator();
  for (auto id : parsers_ids_) {
    parsers_menu_.addAction(id);
  }
}

bool NodeTreeWidget::saveFile(const QString &fileName) {
  QString tmpFileName = fileName + ".~tmp";

  QFile file(tmpFileName);
  file.open(QIODevice::WriteOnly);
  bool ok = file.write(QByteArray((const char *)data_model_->binData().rawData(),
                                  static_cast<int>(data_model_->binData().size()))) != -1;
  if (QFile::exists(fileName)) ok = QFile::remove(fileName);
  if (ok) {
    ok = file.copy(fileName);
    if (ok) ok = QFile::remove(tmpFileName);
  }
  file.close();

  if (!ok) {
    QMessageBox::warning(this, tr("HexEdit"),
                         tr("Cannot write file %1.").arg(fileName));
    return false;
  }
  return true;
}

/*****************************************************************************/
/* Private Slots */
/*****************************************************************************/

void NodeTreeWidget::uploadChanges() {
}

bool NodeTreeWidget::saveAs() {
  QString file_name = QFileDialog::getSaveFileName(
      this, tr("Save As"), cur_file_);
  if (file_name.isEmpty()) return false;

  return saveFile(file_name);
}

void NodeTreeWidget::showVisualisation() {
  auto *panel = new visualisation::VisualisationPanel;
  panel->setData(QByteArray((const char *)data_model_->binData().rawData(),
      static_cast<int>(data_model_->binData().size())));
  panel->setWindowTitle(cur_file_path_);
  panel->setAttribute(Qt::WA_DeleteOnClose);

  main_window_->addTab(panel,
      data_model_->path().join(" : "));
}

void NodeTreeWidget::parse(QAction *action) {
  if (action->text() == "auto") {
    data_model_->parse();
  } else {
    data_model_->parse(action->text());
  }
}

void NodeTreeWidget::showHexEditor() {
  HexEditWidget *hex_edit = new HexEditWidget(main_window_, data_model_,
      selection_model_);
  auto sibling = main_window_->findDockNotTabifiedWith(this);
  auto dock_widget = main_window_->addTab(hex_edit,
      data_model_->path().join(" : "), sibling);
  if (sibling == nullptr) {
    main_window_->addDockWidget(Qt::RightDockWidgetArea, dock_widget);
  }
}

void NodeTreeWidget::removeChunk() {
  auto selectedChunk = tree_view_->selectionModel()->currentIndex();

  auto result = QMessageBox::warning(this, tr("remove chunk"),
      tr("Remove chunk %1 ?").arg(selectedChunk.data().toString()),
      QMessageBox::Yes | QMessageBox::No);
  if (result != QMessageBox::Yes) {
    return;
  }

  data_model_->removeRow(selectedChunk.row(), selectedChunk.parent());
}

void NodeTreeWidget::currentSelectionChanged(const QModelIndex &currentIndex) {
  remove_action_->setEnabled(data_model_->isRemovable(currentIndex));
}

void NodeTreeWidget::newBinData() {
  visualisation_act_->setEnabled(data_model_->binData().size() > 0);
}

void NodeTreeWidget::registerLineEdit(QLineEdit *line_edit) {
  registered_line_edit_ = line_edit;
}

void NodeTreeWidget::updateLineEditWithAddress(qint64 address) {
  if (registered_line_edit_) {
    registered_line_edit_->setText(QString::number(address));
  }
  registered_line_edit_ = nullptr;
}

}  // namespace ui
}  // namespace veles
