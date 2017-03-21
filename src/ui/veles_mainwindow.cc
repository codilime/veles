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
#include <QApplication>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QUrl>
#include <QMenu>

#include "db/db.h"
#include "dbif/method.h"
#include "dbif/promise.h"
#include "dbif/types.h"
#include "dbif/universe.h"
#include "util/version.h"
#include "ui/databaseinfo.h"
#include "ui/veles_mainwindow.h"
#include "ui/hexeditwidget.h"
#include "ui/nodetreewidget.h"
#include "ui/optionsdialog.h"
#include "ui/logwidget.h"
#include "ui/nodewidget.h"

namespace veles {
namespace ui {

/*****************************************************************************/
/* ConnectionManager */
/*****************************************************************************/

ConnectionManager::ConnectionManager(QWidget* parent)
    : QObject(parent), server_process_(nullptr) {
  connection_dialog_ = new ConnectionDialog(parent);
  show_connection_dialog_action_ = new QAction("Connect...", this);
  disconnect_action_ = new QAction("Disconnect", this);
  kill_locally_created_server_action_ = new QAction(
      "Kill locally created server", this);
  kill_locally_created_server_action_->setEnabled(false);
  disconnect_action_->setEnabled(false);

  connect(show_connection_dialog_action_, &QAction::triggered,
      connection_dialog_, &QDialog::show);
}

ConnectionManager::~ConnectionManager() {
  connection_dialog_->deleteLater();
}

QAction* ConnectionManager::showConnectionDialogAction() {
  return show_connection_dialog_action_;
}

QAction* ConnectionManager::disconnectAction() {
  return disconnect_action_;
}

QAction* ConnectionManager::killLocallyCreatedServerAction() {
  return kill_locally_created_server_action_;
}

void ConnectionManager::locallyCreatedServerStarted() {
  kill_locally_created_server_action_->setEnabled(true);
}

void ConnectionManager::locallyCreatedServerFinished(int exit_code,
    QProcess::ExitStatus exit_status) {
  kill_locally_created_server_action_->setEnabled(false);
}

void ConnectionManager::startLocalServer() {
  server_process_ = new QProcess(this);
  connect(server_process_, &QProcess::started,
      this, &ConnectionManager::locallyCreatedServerStarted);
  connect(server_process_, static_cast<void(QProcess::*)
      (int, QProcess::ExitStatus)>(&QProcess::finished),
      this, &ConnectionManager::locallyCreatedServerFinished);
}

void ConnectionManager::killLocalServer() {
  if(server_process_) {
    server_process_->kill();
    server_process_->deleteLater();
    server_process_ = nullptr;
  }
}

/*****************************************************************************/
/* VelesMainWindow - Public methods */
/*****************************************************************************/

VelesMainWindow::VelesMainWindow() : MainWindowWithDetachableDockWidgets(),
    database_dock_widget_(0), log_dock_widget_(0) {
  setAcceptDrops(true);
  resize(1024, 768);
  init();
}

void VelesMainWindow::addFile(QString path) { createFileBlob(path); }

/*****************************************************************************/
/* VelesMainWindow - Protected methods */
/*****************************************************************************/

void VelesMainWindow::dropEvent(QDropEvent *ev) {
  QList<QUrl> urls = ev->mimeData()->urls();
  for (auto url : urls) {
    if (!url.isLocalFile()) {
      continue;
    }
    addFile(url.toLocalFile());
  }
}

void VelesMainWindow::dragEnterEvent(QDragEnterEvent *ev) { ev->accept(); }

/*****************************************************************************/
/* VelesMainWindow - Private Slots */
/*****************************************************************************/

void VelesMainWindow::open() {
  QString fileName = QFileDialog::getOpenFileName(this);
  if (!fileName.isEmpty()) {
    createFileBlob(fileName);
  }
}

void VelesMainWindow::newFile() { createFileBlob(""); }

void VelesMainWindow::about() {
  QMessageBox::about(
      this, tr("About Veles"),
      tr("This is Veles, a binary analysis tool and editor.\n"
         "Version: %1\n"
         "\n"
         "Report bugs to contact@veles.io\n"
         "https://veles.io/\n"
         "\n"
         "Copyright 2017 CodiLime\n"
         "Licensed under the Apache License, Version 2.0\n"
      ).arg(util::version::string));
}

/*****************************************************************************/
/* VelesMainWindow - Private methods */
/*****************************************************************************/

void VelesMainWindow::init() {
  connection_manager_ = new ConnectionManager(this);
  createActions();
  createMenus();
  createLogWindow();
  createDb();

  options_dialog_ = new OptionsDialog(this);

  connect(options_dialog_, &QDialog::accepted, [this]() {
    for(auto main_window : MainWindowWithDetachableDockWidgets
        ::getMainWindows()) {
      QList<View*> views = main_window->findChildren<View*>();
      for(auto view : views) {
        view->reapplySettings();
      }
    }
  });

  connection_manager_->showConnectionDialogAction()->trigger();
}

void VelesMainWindow::createActions() {
  new_file_act_ = new QAction(tr("&New..."), this);
  new_file_act_->setShortcuts(QKeySequence::New);
  new_file_act_->setStatusTip(tr("Open a new file"));
  connect(new_file_act_, SIGNAL(triggered()), this, SLOT(newFile()));

  open_act_ = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
  open_act_->setShortcuts(QKeySequence::Open);
  open_act_->setStatusTip(tr("Open an existing file"));
  connect(open_act_, SIGNAL(triggered()), this, SLOT(open()));

  exit_act_ = new QAction(tr("E&xit"), this);
  exit_act_->setShortcuts(QKeySequence::Quit);
  exit_act_->setStatusTip(tr("Exit the application"));
  connect(exit_act_, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));

  show_database_act_ = new QAction(tr("Show database view"), this);
  show_database_act_->setStatusTip(tr("Show database view"));
  connect(show_database_act_, SIGNAL(triggered()), this, SLOT(showDatabase()));

  show_log_act_ = new QAction(tr("Show log"), this);
  show_log_act_->setStatusTip(tr("Show log"));
  connect(show_log_act_, SIGNAL(triggered()), this, SLOT(showLog()));

  about_act_ = new QAction(tr("&About"), this);
  about_act_->setStatusTip(tr("Show the application's About box"));
  connect(about_act_, SIGNAL(triggered()), this, SLOT(about()));

  about_qt_act_ = new QAction(tr("About &Qt"), this);
  about_qt_act_->setStatusTip(tr("Show the Qt library's About box"));
  connect(about_qt_act_, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

  options_act_ = new QAction(tr("&Options"), this);
  options_act_->setStatusTip(
      tr("Show the Dialog to select applications options"));
  connect(options_act_, &QAction::triggered, this,
          [this]() { options_dialog_->show(); });
}

void VelesMainWindow::createMenus() {
  file_menu_ = menuBar()->addMenu(tr("&File"));

  //Not implemented yet.
  //fileMenu->addAction(newFileAct);

  file_menu_->addAction(open_act_);
  file_menu_->addSeparator();
  file_menu_->addAction(options_act_);
  file_menu_->addSeparator();
  file_menu_->addAction(exit_act_);

  view_menu_ = menuBar()->addMenu(tr("&View"));
  view_menu_->addAction(show_database_act_);
  view_menu_->addAction(show_log_act_);

  QMenu* connection_menu = menuBar()->addMenu(tr("Connection"));
  connection_menu->addAction(
      connection_manager_->showConnectionDialogAction());
  connection_menu->addAction(connection_manager_->disconnectAction());
  connection_menu->addAction(
      connection_manager_->killLocallyCreatedServerAction());

  help_menu_ = menuBar()->addMenu(tr("&Help"));
  help_menu_->addAction(about_act_);
  help_menu_->addAction(about_qt_act_);
}

void VelesMainWindow::updateParsers(dbif::PInfoReply reply) {
  parsers_list_ = reply.dynamicCast<dbif::ParsersListRequest::ReplyType>()->parserIds;
  QList<QDockWidget*> dock_widgets = findChildren<QDockWidget*>();
  for(auto dock : dock_widgets) {
    if(auto hex_tab = dynamic_cast<HexEditWidget *>(dock->widget())) {
      hex_tab->setParserIds(parsers_list_);
    } else if(auto node_tab = dynamic_cast<NodeTreeWidget *>(dock->widget())) {
      node_tab->setParserIds(parsers_list_);
    }
  }
}

void VelesMainWindow::showDatabase() {
  if (database_dock_widget_ == nullptr) {
    createDb();
  }

  database_dock_widget_->raise();

  if (database_dock_widget_->window()->isMinimized()) {
    database_dock_widget_->window()->showNormal();
  }

  database_dock_widget_->window()->raise();
}

void VelesMainWindow::showLog() {
  if (log_dock_widget_ == nullptr) {
    createLogWindow();
  }

  log_dock_widget_->raise();

  if(log_dock_widget_->window()->isMinimized()) {
    log_dock_widget_->window()->showNormal();
  }

  log_dock_widget_->window()->raise();
}

void VelesMainWindow::createDb() {
  if (database_ == nullptr) {
    database_ = db::create_db();
  }
  auto database_info = new DatabaseInfo(database_);
  DockWidget* dock_widget = new DockWidget;
  dock_widget->setAllowedAreas(Qt::AllDockWidgetAreas);
  dock_widget->setWindowTitle("Database");
  dock_widget->setFeatures(
      QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
  dock_widget->setWidget(database_info);
  dock_widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
  auto children = findChildren<DockWidget*>();
  if (!children.empty()) {
    tabifyDockWidget(children.back(), dock_widget);
  } else {
    addDockWidget(Qt::LeftDockWidgetArea, dock_widget);
  }

  connect(database_info, &DatabaseInfo::goFile,
          [this](dbif::ObjectHandle fileBlob, QString fileName) {
            createHexEditTab(fileName, fileBlob);
          });

  connect(database_info, &DatabaseInfo::newFile, [this]() { open(); });

  auto promise = database_->asyncSubInfo<dbif::ParsersListRequest>(this);
  connect(promise, &dbif::InfoPromise::gotInfo, this, &VelesMainWindow::updateParsers);
  database_dock_widget_ = dock_widget;
  QApplication::processEvents();
  updateDocksAndTabs();
}

void VelesMainWindow::createFileBlob(QString fileName) {
  data::BinData data(8, 0);

  if (!fileName.isEmpty()) {
    QFile file(fileName);
    file.setFileName(fileName);
    if(!file.open(QIODevice::ReadOnly)){
      QMessageBox::warning(
          this, tr("Failed to open"),
          QString(tr("Failed to open \"%1\".")).arg(fileName));
      return;
    }
    QByteArray bytes = file.readAll();
    if(bytes.size() == 0 && file.size() != bytes.size()) {
      QMessageBox::warning(
          this, tr("File too large"),
          QString(tr("Failed to open \"%1\" due to current size limitation.")
                  ).arg(fileName));
      return;
    }
    data = data::BinData(8, bytes.size(),
                         reinterpret_cast<uint8_t *>(bytes.data()));
  }
  auto promise =
      database_->asyncRunMethod<dbif::RootCreateFileBlobFromDataRequest>(
          this, data, fileName);
  connect(promise, &dbif::MethodResultPromise::gotResult,
      [this, fileName](dbif::PMethodReply reply) {
    createHexEditTab(
        fileName.isEmpty() ? "untitled" : fileName,
        reply.dynamicCast<dbif::RootCreateFileBlobFromDataRequest::ReplyType>()
            ->object);
  });

  connect(promise, &dbif::MethodResultPromise::gotError,
          [this, fileName](dbif::PError error) {
            QMessageBox::warning(this, tr("Veles"),
                tr("Cannot load file %1.").arg(fileName));
          });
}

void VelesMainWindow::createHexEditTab(QString fileName,
                                     dbif::ObjectHandle fileBlob) {
  QSharedPointer<FileBlobModel> data_model(
      new FileBlobModel(fileBlob, {QFileInfo(fileName).fileName()}));
  QSharedPointer<QItemSelectionModel> selection_model(
      new QItemSelectionModel(data_model.data()));

  NodeWidget* node_widget = new NodeWidget(this, data_model, selection_model);
  addTab(node_widget, data_model->path().join(" : "), nullptr);
}

void VelesMainWindow::createLogWindow() {
  DockWidget* dock_widget = new DockWidget;
  dock_widget->setAllowedAreas(Qt::AllDockWidgetAreas);
  dock_widget->setWindowTitle("Log");
  dock_widget->setFeatures(
      QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
  dock_widget->setWidget(new LogWidget);
  dock_widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

  auto children = findChildren<DockWidget*>();
  if (!children.empty()) {
    tabifyDockWidget(children.back(), dock_widget);
  } else {
    addDockWidget(Qt::LeftDockWidgetArea, dock_widget);
  }

  log_dock_widget_ = dock_widget;
  QApplication::processEvents();
  updateDocksAndTabs();
}

}  // namespace ui
}  // namespace veles
