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
#ifndef VELES_UI_NODETREEWIDGET_H
#define VELES_UI_NODETREEWIDGET_H

#include <QGroupBox>
#include <QLabel>
#include <QSplitter>
#include <QTreeView>
#include <QWidget>
#include <QToolBar>
#include <QSharedPointer>

#include "visualisation/base.h"

#include "ui/fileblobmodel.h"
#include "ui/hexedit.h"
#include "ui/searchdialog.h"
#include "ui/dockwidget.h"

#include "dbif/info.h"
#include "dbif/types.h"

namespace veles {
namespace ui {

class NodeTreeWidget : public View {
  Q_OBJECT

 public:
  explicit NodeTreeWidget(MainWindowWithDetachableDockWidgets *main_window,
      QSharedPointer<FileBlobModel>& data_model,
      QSharedPointer<QItemSelectionModel>& selection_model);
  void reapplySettings();
  void setParserIds(QStringList ids);

 private slots:
  void uploadChanges();
  bool saveAs();
  void updateLineEditWithAddress(qint64 address);
  void showVisualisation();
  void parse(QAction *action);
  void showHexEditor();
  void removeChunk();
  void currentSelectionChanged(const QModelIndex &currentIndex);
  void newBinData();

 private:
  void initParsersMenu();
  bool saveFile(const QString &file_name);

  void addDummySlices(dbif::ObjectHandle);
  void addChunk(QString name, QString type, QString comment, uint64_t start,
                uint64_t end, const QModelIndex &index = QModelIndex());
  void setupTreeViewHandlers();
  void setupDataModelHandlers();

  void createActions();
  void createToolBars();
  void createSliceCreatorWidget();

  void registerLineEdit(QLineEdit *line_edit);
  bool getRangeValues(qint64 *begin, qint64 *end);

  MainWindowWithDetachableDockWidgets *main_window_;

  QString cur_file_;
  QString cur_file_path_;
  QFile file_;
  bool is_untitled;

  QToolBar *file_tool_bar_;
  QToolBar *edit_tool_bar_;
  QToolBar *tools_tool_bar_;

  QAction *upload_act_;
  QAction *save_as_act_;
  QAction *save_readable_act_;
  QAction *undo_act_;
  QAction *redo_act_;
  QAction *visualisation_act_;
  QAction *parser_act_;
  QAction *show_hex_edit_act_;
  QAction *remove_action_;

  QTreeView *tree_view_;

  QSharedPointer<FileBlobModel> data_model_;
  QSharedPointer<QItemSelectionModel> selection_model_;

  QLineEdit *registered_line_edit_;

  QStringList parsers_ids_;
  QMenu parsers_menu_;

};

}  // namespace ui
}  // namespace veles

#endif // VELES_UI_NODETREEWIDGET_H
