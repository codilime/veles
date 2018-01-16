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
#pragma once

#include <QGroupBox>
#include <QLabel>
#include <QMainWindow>
#include <QSharedPointer>
#include <QSplitter>
#include <QStringList>
#include <QToolBar>
#include <QToolButton>
#include <QTreeView>
#include <QWidget>
#include <QWidgetAction>

#include "dbif/info.h"
#include "dbif/types.h"
#include "ui/dockwidget.h"
#include "ui/fileblobmodel.h"
#include "ui/hexedit.h"
#include "ui/searchdialog.h"
#include "visualization/base.h"

namespace veles {
namespace ui {

class HexEditWidget : public IconAwareView {
  Q_OBJECT

 public:
  explicit HexEditWidget(
      MainWindowWithDetachableDockWidgets* main_window,
      const QSharedPointer<FileBlobModel>& data_model,
      const QSharedPointer<QItemSelectionModel>& selection_model);
  void reapplySettings() override;
  void setParserIds(const QStringList& ids);
  QString addressAsText(qint64 addr);
  QAction* uploadAction() const { return upload_act_; }
  QAction* undoAction() const { return undo_act_; }
  QAction* discardAction() const { return discard_act_; }
  QAction* findAction() const { return find_act_; }
  QAction* findNextAction() const { return find_next_act_; }
  QAction* showVisualizationAction() const { return visualization_act_; }
  QAction* showHexEditAction() const { return show_hex_edit_act_; }
  QAction* showNodeTreeAction() const { return show_node_tree_act_; }
  QAction* addColumnAction() const { return add_column_act_; }
  QAction* removeColumnAction() const { return remove_column_act_; }

 signals:
  void showNodeTree(bool show);
  void showMinimap(bool show);

 public slots:
  void nodeTreeVisibilityChanged(bool visibility);
  void minimapVisibilityChanged(bool visibility);

 private slots:
  void parse(QAction* action);
  void findNext();
  void showSearchDialog();
  void saveAs();
  void showVisualization();
  void showDisasmTab();
  void openHexEditor();
  void newBinData();
  void enableFindNext(bool enable);
  void selectionChanged(qint64 start_addr, qint64 selection_size);
  void editStateChanged(bool has_changes, bool has_undo);
  void addColumn();
  void removeColumn();

 private:
  void addChunk(const QString& name, const QString& type,
                const QString& comment, uint64_t start, uint64_t end,
                const QModelIndex& index = QModelIndex());
  void setupDataModelHandlers();

  void createActions();
  void createToolBars();
  void initParsersMenu();
  void createSelectionInfo();

  MainWindowWithDetachableDockWidgets* main_window_;

  QToolBar* file_tool_bar_;
  QToolBar* edit_tool_bar_;
  QToolBar* tools_tool_bar_;
  QToolBar* view_tool_bar_;

  QAction* save_as_act_;
  QAction* upload_act_;
  QAction* undo_act_;
  QAction* discard_act_;
  // QAction* redo_act_;
  QAction* find_act_;
  QAction* find_next_act_;
  QAction* visualization_act_;
  QAction* show_node_tree_act_;
  QAction* show_minimap_act_;
  QAction* show_hex_edit_act_;
  QAction* show_disasm_act_;
  QAction* add_column_act_;
  QAction* remove_column_act_;
  QWidgetAction* auto_resize_act_;
  QCheckBox* auto_resize_checkbox_;
  QAction* change_edit_mode_act_;
  QPushButton* change_edit_mode_button_;

  SearchDialog* search_dialog_;
  HexEdit* hex_edit_;

  QSharedPointer<FileBlobModel> data_model_;
  QSharedPointer<QItemSelectionModel> selection_model_;

  QStringList parsers_ids_;
  QMenu parsers_menu_;
  QLabel* selection_label_;
};

}  // namespace ui
}  // namespace veles
