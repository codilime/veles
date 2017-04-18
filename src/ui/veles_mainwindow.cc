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
#include "client/dbif.h"
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

void VelesMainWindow::showEvent(QShowEvent* event) {
  emit shown();
}

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

  tool_bar_ = addToolBar("Connection");
  tool_bar_->setContextMenuPolicy(Qt::PreventContextMenu);

  ConnectionsWidget* connections_widget = new ConnectionsWidget(this);
  tool_bar_->addWidget(connections_widget);
  connect(connection_manager_, &ConnectionManager::connectionStatusChanged,
      connections_widget,
      &ConnectionsWidget::updateConnectionStatus);
  connect(connection_manager_, &ConnectionManager::connectionsChanged,
      connections_widget, &ConnectionsWidget::updateConnections);

  connection_notification_widget_ = new ConnectionNotificationWidget(this);
  connect(connection_manager_, &ConnectionManager::connectionStatusChanged,
      connection_notification_widget_,
      &ConnectionNotificationWidget::updateConnectionStatus);
  tool_bar_->addWidget(connection_notification_widget_);

  bringDockWidgetToFront(log_dock_widget_);
  log_dock_widget_->setFocus(Qt::OtherFocusReason);

  connection_manager_->showConnectionDialogAction()->trigger();
  connect(this, &VelesMainWindow::shown,
      connection_manager_, &ConnectionManager::raiseConnectionDialog,
      Qt::QueuedConnection);
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
#if 0
    database_ = db::create_db();
#else
    auto nc = new client::NCWrapper(
        connection_manager_->networkClient(), this);
    database_ = QSharedPointer<client::NCObjectHandle>::create(
        nc, *data::NodeID::getRootNodeId(), dbif::ObjectType::ROOT);
#endif
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
