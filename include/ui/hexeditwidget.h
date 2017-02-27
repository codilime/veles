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
#include <QSplitter>
#include <QTreeView>
#include <QWidget>
#include <QStringList>
#include <QToolBar>
#include <QToolButton>
#include <QMainWindow>
#include <QSharedPointer>
#include "visualization/base.h"

#include "ui/fileblobmodel.h"
#include "ui/hexedit.h"
#include "ui/searchdialog.h"
#include "ui/dockwidget.h"

#include "dbif/info.h"
#include "dbif/types.h"

namespace veles {
namespace ui {

class HexEditWidget : public View {
  Q_OBJECT

 public:
  explicit HexEditWidget(MainWindowWithDetachableDockWidgets *main_window,
      QSharedPointer<FileBlobModel>& data_model,
      QSharedPointer<QItemSelectionModel>& selection_model);
  void reapplySettings() override;
  void setParserIds(QStringList ids);
  QString addressAsText(qint64 addr);
  QAction* findAction() {return find_act_;};
  QAction* findNextAction() {return find_next_act_;};

 signals:
  void showNodeTree(bool show);
  void showMinimap(bool show);

 public slots:
  void nodeTreeVisibilityChanged(bool visibility);
  void minimapVisibilityChanged(bool visibility);

 private slots:
  void parse(QAction *action);
  void findNext();
  void showSearchDialog();
  void uploadChanges();
  bool saveAs();
  void showVisualization();
  void showHexEditor();
  void newBinData();
  void enableFindNext(bool enable);
  void selectionChanged(qint64 start_addr, qint64 selection_size);

 private:
  bool saveFile(const QString &file_name);

  void addDummySlices(dbif::ObjectHandle);
  void addChunk(QString name, QString type, QString comment, uint64_t start,
                uint64_t end, const QModelIndex &index = QModelIndex());
  void setupDataModelHandlers();

  void createActions();
  void createToolBars();
  void createSliceCreatorWidget();
  void initParsersMenu();
  void createSelectionInfo();

  bool getRangeValues(qint64 *begin, qint64 *end);

  MainWindowWithDetachableDockWidgets *main_window_;

  QString cur_file_;
  QString cur_file_path_;
  QFile file_;
  bool is_untitled_;

  QToolBar *file_tool_bar_;
  QToolBar *edit_tool_bar_;
  QToolBar *tools_tool_bar_;

  QAction *upload_act_;
  QAction *save_as_act_;
  QAction *save_readable_;
  QAction *undo_act_;
  QAction *redo_act_;
  QAction *find_act_;
  QAction *find_next_act_;
  QAction *visualization_act_;
  QAction *show_node_tree_act_;
  QAction *show_minimap_act_;
  QAction *show_hex_edit_act_;

  SearchDialog *search_dialog_;
  HexEdit *hex_edit_;

  QSharedPointer<FileBlobModel> data_model_;
  QSharedPointer<QItemSelectionModel> selection_model_;

  QStringList parsers_ids_;
  QMenu parsers_menu_;
  QLabel* selection_label_;
};

}  // namespace ui
}  // namespace veles
