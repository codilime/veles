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
#ifndef HEXEDITTAB_H
#define HEXEDITTAB_H

#include <QGroupBox>
#include <QLabel>
#include <QSplitter>
#include <QTreeView>
#include <QWidget>
#include <QToolBar>

#include "visualisation/base.h"

#include "ui/fileblobmodel.h"
#include "ui/hexedit.h"
#include "ui/searchdialog.h"

#include "dbif/info.h"
#include "dbif/types.h"

namespace veles {
namespace ui {

class VelesMainWindow;

class HexEditTab : public QWidget {
  Q_OBJECT

 public:
  explicit HexEditTab(VelesMainWindow *mainWindow, FileBlobModel *dataModel);
  void reapplySettings();

 private slots:
  void findNext();
  void showSearchDialog();
  void uploadChanges();
  bool saveAs();
  void updateLineEditWithAddress(qint64 address);
  void showVisualisation();
  void parse();

 private:
  bool saveFile(const QString &fileName);

  void addDummySlices(dbif::ObjectHandle);
  void addChunk(QString name, QString type, QString comment, uint64_t start,
                uint64_t end, const QModelIndex &index = QModelIndex());
  void setupTreeViewHandlers();
  void setupDataModelHandlers();

  void createActions();
  void createToolBars();
  void createSliceCreatorWidget();

  void registerLineEdit(QLineEdit *lineEdit);
  bool getRangeValues(qint64 *begin, qint64 *end);

  VelesMainWindow *mainWindow;

  QString curFile;
  QString curFilePath;
  QFile file;
  bool isUntitled;

  QVBoxLayout *layout;

  QWidget *toolBarWrapper;
  QHBoxLayout *toolBarLayout;
  QToolBar *fileToolBar;
  QToolBar *editToolBar;
  QToolBar *visualisationToolBar;
  QToolBar *parserToolBar;

  QAction *uploadAct;
  QAction *saveAsAct;
  QAction *saveReadable;
  QAction *undoAct;
  QAction *redoAct;
  QAction *findAct;
  QAction *findNextAct;
  QAction *visualisationAct;
  QAction *parserAct;

  SearchDialog *searchDialog;

  QSplitter *splitter;
  QTreeView *treeView;

  HexEdit *hexEdit;

  FileBlobModel *dataModel;

  QLineEdit *registeredLineEdit;
};

}  // namespace ui
}  // namespace veles

#endif
