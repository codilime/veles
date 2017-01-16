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

#include "ui/veles_mainwindow.h"
#include "visualisation/panel.h"

#include "dbif/info.h"
#include "dbif/types.h"
#include "dbif/universe.h"
#include "ui/hexedittab.h"

#include "util/settings/hexedit.h"

namespace veles {
namespace ui {

/*****************************************************************************/
/* Public methods */
/*****************************************************************************/

HexEditTab::HexEditTab(VelesMainWindow *mainWindow, FileBlobModel *dataModel)
    : mainWindow(mainWindow), dataModel(dataModel) {
  layout = new QVBoxLayout;
  layout->setContentsMargins(2, 2, 2, 2);

  splitter = new QSplitter(this);

  auto selectionModel = new QItemSelectionModel(dataModel, this);
  treeView = new QTreeView();
  treeView->setModel(dataModel);
  treeView->setSelectionModel(selectionModel);
  hexEdit = new HexEdit(dataModel, selectionModel, this);

  splitter->addWidget(treeView);
  splitter->addWidget(hexEdit);
  splitter->setSizes({380, 520});

  searchDialog = new SearchDialog(hexEdit, this);

  createActions();
  createToolBars();

  layout->addWidget(splitter, 1);

  setLayout(layout);
  dataModel->setParent(this);
  setupDataModelHandlers();

  treeView->setColumnWidth(0, 190);
  treeView->setColumnWidth(1, 140);
  treeView->header()->setStretchLastSection(true);

  setupTreeViewHandlers();

  registeredLineEdit = nullptr;

  reapplySettings();
  setWindowTitle(dataModel->path().join(" : ") + " - Hex");
}

void HexEditTab::reapplySettings() {
  hexEdit->setBytesPerRow(util::settings::hexedit::columnsNumber(), util::settings::hexedit::resizeColumnsToWindowWidth());
}

/*****************************************************************************/
/* Private UI methods */
/*****************************************************************************/

void HexEditTab::createActions() {

  uploadAct = new QAction(QIcon(":/images/upload-32.ico"), tr("&Upload"), this);
  uploadAct->setShortcuts(QKeySequence::Save);
  uploadAct->setStatusTip(tr("Upload changed to database"));
  connect(uploadAct, SIGNAL(triggered()), this, SLOT(uploadChanges()));
  uploadAct->setEnabled(false);

  saveAsAct = new QAction(QIcon(":/images/save.png"), tr("Save &As..."), this);
  saveAsAct->setShortcuts(QKeySequence::SaveAs);
  saveAsAct->setStatusTip(tr("Save the document under a new name"));
  connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));
  saveAsAct->setEnabled(false);

  saveReadable = new QAction(tr("Save &Readable..."), this);
  saveReadable->setStatusTip(tr("Save document in readable form"));
  // connect(saveReadable, SIGNAL(triggered()), this,
  // SLOT(saveToReadableFile()));

  undoAct = new QAction(QIcon(":/images/undo.png"), tr("&Undo"), this);
  undoAct->setShortcuts(QKeySequence::Undo);
  undoAct->setEnabled(false);

  redoAct = new QAction(QIcon(":/images/redo.png"), tr("&Redo"), this);
  redoAct->setShortcuts(QKeySequence::Redo);
  redoAct->setEnabled(false);

  findAct = new QAction(QIcon(":/images/find.png"), tr("&Find/Replace"), this);
  findAct->setShortcuts(QKeySequence::Find);
  findAct->setStatusTip(tr("Show the Dialog for finding and replacing"));
  connect(findAct, SIGNAL(triggered()), this, SLOT(showSearchDialog()));

  findNextAct = new QAction(QIcon(":/images/find.png"), tr("Find &next"), this);
  findNextAct->setShortcuts(QKeySequence::FindNext);
  findNextAct->setStatusTip(tr("Find next occurrence of the searched pattern"));
  connect(findNextAct, SIGNAL(triggered()), this, SLOT(findNext()));

  visualisationAct =
      new QAction(QIcon(":/images/nginx3d_32.png"), tr("&Visualisation"), this);
  visualisationAct->setToolTip(tr("Visualisation"));
  visualisationAct->setEnabled(dataModel->binData().size() > 0);
  connect(visualisationAct, SIGNAL(triggered()), this,
          SLOT(showVisualisation()));

  parserAct = new QAction(QIcon(":/images/parse.png"), tr("&Parse"), this);
  parserAct->setToolTip(tr("Parser"));
  parserAct->setEnabled(true);
  connect(parserAct, SIGNAL(triggered()), this, SLOT(parse()));
}

void HexEditTab::createToolBars() {
  toolBarWrapper = new QWidget;
  toolBarLayout = new QHBoxLayout;
  toolBarLayout->setContentsMargins(2, 2, 2, 2);

  fileToolBar = new QToolBar(tr("File"));
  fileToolBar->addAction(uploadAct);
  fileToolBar->addAction(saveAsAct);
  toolBarLayout->addWidget(fileToolBar);

  editToolBar = new QToolBar(tr("Edit"));
  editToolBar->addAction(undoAct);
  editToolBar->addAction(redoAct);
  editToolBar->addAction(findAct);
  editToolBar->addAction(findNextAct);
  toolBarLayout->addWidget(editToolBar);

  visualisationToolBar = new QToolBar(tr("Visualisation"));
  visualisationToolBar->addAction(visualisationAct);
  toolBarLayout->addWidget(visualisationToolBar);

  parserToolBar = new QToolBar(tr("parser"));
  parserToolBar->addAction(parserAct);
  toolBarLayout->addWidget(parserToolBar);

  toolBarLayout->addStretch();
  toolBarWrapper->setLayout(toolBarLayout);
  layout->addWidget(toolBarWrapper, 0);
}

void HexEditTab::addChunk(QString name, QString type, QString comment,
                          uint64_t start, uint64_t end,
                          const QModelIndex &index) {
  dataModel->addChunk(name, type, comment, start, end, index);
}

void HexEditTab::setupTreeViewHandlers() {

  auto context_menu = new QMenu(treeView);
  treeView->setContextMenuPolicy(Qt::ActionsContextMenu);
  auto remove_action = new QAction("remove chunk", context_menu);
  treeView->addAction(remove_action);
  remove_action->setEnabled(false);

  connect(treeView->selectionModel(), &QItemSelectionModel::currentChanged,
          [this, remove_action](const QModelIndex &currentIndex) {
            remove_action->setEnabled(dataModel->isRemovable(currentIndex));
          });

  connect(remove_action, &QAction::triggered, [this]() {
    auto selectedChunk = treeView->selectionModel()->currentIndex();

    auto result = QMessageBox::warning(
        this, tr("remove chunk"),
        tr("Remove chunk %1 ?").arg(selectedChunk.data().toString()),
        QMessageBox::Yes | QMessageBox::No);
    if (result != QMessageBox::Yes) {
      return;
    }

    dataModel->removeRow(selectedChunk.row(), selectedChunk.parent());
  });

  connect(treeView, &QAbstractItemView::doubleClicked,
          [this](const QModelIndex &index) {
            auto mainIndex = dataModel->index(
                index.row(), FileBlobModel::COLUMN_INDEX_MAIN, index.parent());
            dbif::ObjectHandle newRoot = dataModel->blob(mainIndex);
            if (newRoot) {
              auto newPath = dataModel->path();
              newPath.append(mainIndex.data().toString());
              auto newModel = new FileBlobModel(newRoot, newPath);
              HexEditTab *hexTab = new HexEditTab(mainWindow, newModel);
              mainWindow->addTab(hexTab, hexTab->windowTitle());
            }
          });
}

void HexEditTab::setupDataModelHandlers() {
  connect(dataModel, &FileBlobModel::newBinData, [this]() {
    visualisationAct->setEnabled(dataModel->binData().size() > 0);
  });
}

/*****************************************************************************/
/* Other private methods */
/*****************************************************************************/

bool HexEditTab::saveFile(const QString &fileName) {
  QString tmpFileName = fileName + ".~tmp";

  QFile file(tmpFileName);
  file.open(QIODevice::WriteOnly);
  bool ok = file.write(QByteArray((const char *)dataModel->binData().rawData(),
                                  dataModel->binData().size())) != -1;
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

void HexEditTab::findNext() { searchDialog->findNext(); }

void HexEditTab::showSearchDialog() { searchDialog->show(); }

void HexEditTab::uploadChanges() {
}

bool HexEditTab::saveAs() {
  QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"), curFile);
  if (fileName.isEmpty()) return false;

  return saveFile(fileName);
}

void HexEditTab::showVisualisation() {
  auto *panel = new visualisation::VisualisationPanel;
  panel->setData(QByteArray((const char *)dataModel->binData().rawData(),
                            dataModel->binData().size()));
  panel->setWindowTitle(curFilePath);
  panel->setAttribute(Qt::WA_DeleteOnClose);

  mainWindow->addTab(panel, dataModel->path().join(" : ") + " - Visualisation");
}

void HexEditTab::parse() {
  auto fileBlob = dataModel->blob();
  fileBlob->asyncRunMethod<dbif::BlobParseRequest>(this);
}

void HexEditTab::registerLineEdit(QLineEdit *lineEdit) {
  registeredLineEdit = lineEdit;
}

void HexEditTab::updateLineEditWithAddress(qint64 address) {
  if (registeredLineEdit) {
    registeredLineEdit->setText(QString::number(address));
  }
  registeredLineEdit = nullptr;
}

}  // namespace ui
}  // namespace veles
