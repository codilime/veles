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
#include <QApplication>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QTabWidget>
#include <QUrl>
#include <QMenu>
#include <QCursor>
#include <QDesktopWidget>

#include "db/db.h"
#include "dbif/method.h"
#include "dbif/promise.h"
#include "dbif/types.h"
#include "dbif/universe.h"
#include "util/version.h"
#include "ui/databaseinfo.h"
#include "ui/veles_mainwindow.h"
#include "ui/hexedittab.h"
#include "ui/optionsdialog.h"

namespace veles {
namespace ui {

/*****************************************************************************/
/* DockWidget */
/*****************************************************************************/

DockWidget::DockWidget() : QDockWidget(), timer_id_(0), ticks_(0),
    context_menu_(nullptr) {
  setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);
  setContextMenuPolicy(Qt::CustomContextMenu);
  auto first_main_window =
      MainWindowWithDetachableDockWidgets::getFirstMainWindow();
  if (first_main_window) {
    connect(this, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
        first_main_window, SLOT(dockLocationChanged(Qt::DockWidgetArea)));
  }
  connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
      this, SLOT(displayContextMenu(const QPoint&)));
}

DockWidget::~DockWidget() {
  if (timer_id_) {
    killTimer(timer_id_);
  }
}

void DockWidget::displayContextMenu(const QPoint& pos) {
  if(context_menu_) {
    context_menu_->clear();
  } else {
    context_menu_ = new QMenu(this);
  }

  context_menu_->addMenu(createMoveToDesktopMenu());
  context_menu_->addMenu(createMoveToWindowMenu());
  context_menu_->popup(mapToGlobal(pos));
}

void DockWidget::moveToDesktop() {
  auto action = dynamic_cast<QAction*>(sender());
  if (action) {
    bool ok;
    int screen = action->data().toInt(&ok);
    if (ok) {
      MainWindowWithDetachableDockWidgets
      ::getOrCreateWindowForScreen(screen)->moveDockWidgetToWindow(this);
    }
  }
}

void DockWidget::moveToWindow() {
  auto action = dynamic_cast<QAction*>(sender());
  if (action) {
    MainWindowWithDetachableDockWidgets* window =
        reinterpret_cast<MainWindowWithDetachableDockWidgets*>(
        qvariant_cast<quintptr>(action->data()));

    window->moveDockWidgetToWindow(this);
  }
}

void DockWidget::moveEvent(QMoveEvent *event) {
  ticks_ = 0;

  if(timer_id_ == 0) {
    timer_id_ = startTimer(step_msec_);
  }
}

void DockWidget::timerEvent(QTimerEvent* event) {
  if (isFloating()) {
    ++ticks_;
  }

  if (QApplication::mouseButtons() != Qt::NoButton) {
    ticks_ = 0;
  }

  if (ticks_ > max_ticks_ && isFloating()) {
    if (MainWindowWithDetachableDockWidgets
        ::intersectsWithAnyMainWindow(this)) {
      auto candidate = MainWindowWithDetachableDockWidgets
          ::getParentCandidateForDockWidget(this);
      if(candidate) {
        candidate->moveDockWidgetToWindow(this);
      }
    } else {
      MainWindowWithDetachableDockWidgets* main_window =
          new MainWindowWithDetachableDockWidgets;
      main_window->setGeometry(QDockWidget::geometry());
      main_window->addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, this);
      main_window->show();
    }
  }

  if (timer_id_ && (ticks_ > max_ticks_ || !isFloating())) {
    killTimer(timer_id_);
    timer_id_ = 0;
  }
}

QMenu* DockWidget::createMoveToDesktopMenu() {
  QMenu* menu_move_to_screen = new QMenu(tr("Move to desktop"), context_menu_);
  int screen_count = QApplication::desktop()->screenCount();
  for (int screen = 0; screen < screen_count; ++screen) {
    bool already_there = false;

    if (QApplication::desktop()->screenNumber(this) == screen) {
      already_there = true;
    }

    auto action = new QAction(
        already_there ? QString("Desktop %1 (it's already there)").arg(screen + 1)
        : QString("Desktop %1").arg(screen + 1),
        menu_move_to_screen);
    action->setData(screen);
    action->setEnabled(!already_there);
    connect(action, SIGNAL(triggered()), this, SLOT(moveToDesktop()));
    menu_move_to_screen->addAction(action);
  }

  return menu_move_to_screen;
}

QMenu* DockWidget::createMoveToWindowMenu() {
  QMenu* menu_move_to_window = new QMenu(tr("Move to window"), context_menu_);
  for (auto window : MainWindowWithDetachableDockWidgets::getMainWindows()) {
    bool already_there = false;

    if(!isFloating() && window->findChildren<DockWidget*>().contains(this)) {
      already_there = true;
    }

    auto action = new QAction(
        already_there ? window->windowTitle() + " (it's already there)"
        : window->windowTitle(), menu_move_to_window);
    action->setData(quintptr(window));
    action->setEnabled(!already_there);
    connect(action, SIGNAL(triggered()), this, SLOT(moveToWindow()));
    menu_move_to_window->addAction(action);
  }

  return menu_move_to_window;
}

/*****************************************************************************/
/* MainWindowWithDetachableDockWidgets */
/*****************************************************************************/

std::set<MainWindowWithDetachableDockWidgets*>
    MainWindowWithDetachableDockWidgets::main_windows_;
MainWindowWithDetachableDockWidgets* MainWindowWithDetachableDockWidgets
    ::first_main_window_ = nullptr;
int MainWindowWithDetachableDockWidgets::last_created_window_id_ = 0;

MainWindowWithDetachableDockWidgets::MainWindowWithDetachableDockWidgets(
    QWidget* parent) : QMainWindow(parent) {
  setAttribute(Qt::WA_DeleteOnClose, true);
  setCentralWidget(nullptr);
  setDockNestingEnabled(true);
  setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::TabPosition::North);
  main_windows_.insert(this);
  ++last_created_window_id_;
  if (main_windows_.size() == 1) {
    first_main_window_ = this;
    setAttribute(Qt::WA_QuitOnClose, true);
    setWindowTitle("Veles");
  } else {
    setAttribute(Qt::WA_QuitOnClose, false);
    setWindowTitle(QString("Veles - window #%1").arg(last_created_window_id_));
  }
}

MainWindowWithDetachableDockWidgets::~MainWindowWithDetachableDockWidgets() {
  main_windows_.erase(this);
  if(this == first_main_window_) {
    first_main_window_ = 0;
  }
}

void MainWindowWithDetachableDockWidgets::addTab(QWidget *widget,
    const QString &title) {
  DockWidget* dock_widget = new DockWidget;
  dock_widget->setAllowedAreas(Qt::AllDockWidgetAreas);
  dock_widget->setWindowTitle(title);
  dock_widget->setFloating(false);
  dock_widget->setFeatures(QDockWidget::DockWidgetMovable |
      QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable);
  dock_widget->setWidget(widget);

  QList<QDockWidget*> dock_widgets = findChildren<QDockWidget*>();
  if (dock_widgets.size() > 0) {
    tabifyDockWidget(dock_widgets.back(), dock_widget);
  } else {
    addDockWidget(Qt::RightDockWidgetArea, dock_widget);
  }

  QApplication::processEvents();
  updateCloseButtonsOnTabBars();
  bringDockWidgetToFront(dock_widget);
}

void MainWindowWithDetachableDockWidgets::bringDockWidgetToFront(
    QDockWidget* dock_widget) {
  QList<QTabBar*> tab_bars = findChildren<QTabBar*>();
  for (auto tab_bar : tab_bars) {
    for (int i = 0; i < tab_bar->count(); ++i) {
      if (dock_widget == tabToDockWidget(tab_bar, i)) {
        tab_bar->setCurrentIndex(i);
        return;
      }
    }
  }
}

void MainWindowWithDetachableDockWidgets::moveDockWidgetToWindow(
    DockWidget* dock_widget) {
  DockWidget* dock_widget_to_tabify = 0;
  auto children = findChildren<DockWidget*>();

  if (dock_widget->isFloating()) {
    if(children.contains(dock_widget)) {
      dock_widget->setFloating(false);
      return;
    }

    dock_widget->setParent(nullptr);

    for (auto candidate : findChildren<DockWidget*>()) {
      if (candidate->geometry().intersects(dock_widget->geometry())
          && !candidate->isFloating()) {
        dock_widget_to_tabify = candidate;
        break;
      }
    }
  }

  if (dock_widget_to_tabify) {
    tabifyDockWidget(dock_widget_to_tabify, dock_widget);
  } else {
    addDockWidget(Qt::LeftDockWidgetArea, dock_widget);
  }

  QApplication::processEvents();
  bringDockWidgetToFront(dock_widget);
}

bool MainWindowWithDetachableDockWidgets::intersectsWithAnyMainWindow(
    DockWidget* dock_widget) {
  if (dock_widget == nullptr) {
    return false;
  }

  for(auto main_window : main_windows_) {
    if(main_window->frameGeometry().intersects(dock_widget->frameGeometry())) {
      return true;
    }
  }

  return false;
}

MainWindowWithDetachableDockWidgets* MainWindowWithDetachableDockWidgets
    ::getParentCandidateForDockWidget(DockWidget* dock_widget) {
  if (!dock_widget || !dock_widget->isFloating()) {
    return nullptr;
  }

  for (auto window : main_windows_) {
    if (window->geometry().intersects(dock_widget->geometry())
        && !window->isAncestorOf(dock_widget)) {
      if (window->findChildren<QDockWidget*>().contains(dock_widget)) {
        return nullptr;
      }

      return window;
    }
  }

  return nullptr;
}

const std::set<MainWindowWithDetachableDockWidgets*>&
    MainWindowWithDetachableDockWidgets::getMainWindows(){
  return main_windows_;
}

MainWindowWithDetachableDockWidgets*
    MainWindowWithDetachableDockWidgets::getFirstMainWindow() {
  return first_main_window_;
}

MainWindowWithDetachableDockWidgets* MainWindowWithDetachableDockWidgets::
    getOrCreateWindowForScreen(int screen) {
  if(screen >= QApplication::desktop()->screenCount()) {
    return nullptr;
  }

  for(auto window : MainWindowWithDetachableDockWidgets::getMainWindows()) {
    int wnd_screen = QApplication::desktop()->screenNumber(window);
    if(wnd_screen == screen) {
      return window;
    }
  }

  auto new_window = new MainWindowWithDetachableDockWidgets;
  QRect target_geometry = QApplication::desktop()->availableGeometry(screen);
  new_window->move(target_geometry.topLeft());
  new_window->resize(1000, 700);
  new_window->showMaximized();

  return new_window;
}

MainWindowWithDetachableDockWidgets* MainWindowWithDetachableDockWidgets
    ::getOwnerOfDockWidget(DockWidget* dock_widget) {
  for (auto window : main_windows_) {
    if (window->findChildren<QDockWidget*>().contains(dock_widget)) {
      return window;
    }
  }

  return nullptr;
}

void MainWindowWithDetachableDockWidgets::dockLocationChanged(
    Qt::DockWidgetArea area) {
  QWidget* dock_widget = dynamic_cast<QWidget*>(sender());
  if(dock_widget) {
    auto main_window = dynamic_cast<MainWindowWithDetachableDockWidgets*>(
        dock_widget->window());
    if(main_window) {
      main_window->updateCloseButtonsOnTabBars();
    }
  }
}

void MainWindowWithDetachableDockWidgets::tabCloseRequested(int index) {
  QTabBar* tab_bar = dynamic_cast<QTabBar*>(sender());
  if(tab_bar) {
    QDockWidget* dock_widget = tabToDockWidget(tab_bar, index);
    if(dock_widget) {
      dock_widget->deleteLater();
    }
  }
}

bool MainWindowWithDetachableDockWidgets::event(QEvent* event) {
  bool has_children = findChildren<QDockWidget*>().size() != 0;

  if (has_children) {
    if (event->type() == QEvent::ChildAdded
        || event->type() == QEvent::ChildRemoved) {
      updateCloseButtonsOnTabBars();
    }
  } else {
    if (event->type() == QEvent::ChildRemoved && this != first_main_window_) {
      this->deleteLater();
    }
  }

  return QMainWindow::event(event);
}

QDockWidget* MainWindowWithDetachableDockWidgets::tabToDockWidget(
    QTabBar* tab_bar, int index) {
  if(tab_bar) {
    // Based on undocumented feature (tested with Qt 5.7).
    // QTabBars that control visibility of QDockWidgets grouped in
    // QDockWidgetGroupWindows hold ids of respective QDockAreaLayoutInfos
    // in tabData. Conveniently those ids are just pointers to
    // QDockWidgets (stored as quintptr) and can be retrieved through public
    // interface.
    QDockWidget* dock =
        reinterpret_cast<QDockWidget*>(
        qvariant_cast<quintptr>(tab_bar->tabData(index)));

    return dock;
  }

  return 0;
}

void MainWindowWithDetachableDockWidgets::updateCloseButtonsOnTabBars() {
  QList<QTabBar*> tab_bars = findChildren<QTabBar*>();
  for (auto tab_bar : tab_bars) {
    tab_bar->setTabsClosable(true);
    connect(tab_bar, SIGNAL(tabCloseRequested(int)), this,
        SLOT(tabCloseRequested(int)));

    for (int i = 0; i < tab_bar->count(); ++i) {
      QDockWidget* dock_widget = tabToDockWidget(tab_bar, i);
      if (dock_widget
          && !(dock_widget->features() & QDockWidget::DockWidgetClosable)) {
        // Remove close button of a tab that controls non-closable QDockWidget.
        tab_bar->setTabButton(i, QTabBar::LeftSide, 0);
        tab_bar->setTabButton(i, QTabBar::RightSide, 0);
      }
    }
  }
}

/*****************************************************************************/
/* VelesMainWindow - Public methods */
/*****************************************************************************/

VelesMainWindow::VelesMainWindow() : MainWindowWithDetachableDockWidgets() {
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
         "Report bugs to veles@codisec.com\n"
         "https://codisec.com/veles/\n"
         "\n"
         "Copyright 2016 CodiLime\n"
         "Licensed under the Apache License, Version 2.0\n"
      ).arg(util::version::string));
}

/*****************************************************************************/
/* VelesMainWindow - Private methods */
/*****************************************************************************/

void VelesMainWindow::init() {
  createActions();
  createMenus();
  createDb();

  optionsDialog = new OptionsDialog(this);

  connect(optionsDialog, &QDialog::accepted, [this]() {
    QList<QDockWidget*> dock_widgets = findChildren<QDockWidget*>();
    for(auto dock : dock_widgets) {
      if(auto hexTab = dynamic_cast<HexEditTab *>(dock->widget())) {
        hexTab->reapplySettings();
      }
    }
  });
}

void VelesMainWindow::createActions() {
  newFileAct = new QAction(tr("&New..."), this);
  newFileAct->setShortcuts(QKeySequence::New);
  newFileAct->setStatusTip(tr("Open a new file"));
  connect(newFileAct, SIGNAL(triggered()), this, SLOT(newFile()));

  openAct = new QAction(QIcon(":/images/open.png"), tr("&Open..."), this);
  openAct->setShortcuts(QKeySequence::Open);
  openAct->setStatusTip(tr("Open an existing file"));
  connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

  exitAct = new QAction(tr("E&xit"), this);
  exitAct->setShortcuts(QKeySequence::Quit);
  exitAct->setStatusTip(tr("Exit the application"));
  connect(exitAct, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));

  aboutAct = new QAction(tr("&About"), this);
  aboutAct->setStatusTip(tr("Show the application's About box"));
  connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

  aboutQtAct = new QAction(tr("About &Qt"), this);
  aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
  connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

  optionsAct = new QAction(tr("&Options"), this);
  optionsAct->setStatusTip(
      tr("Show the Dialog to select applications options"));
  connect(optionsAct, &QAction::triggered, this,
          [this]() { optionsDialog->show(); });
}

void VelesMainWindow::createMenus() {
  fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(newFileAct);
  fileMenu->addAction(openAct);
  fileMenu->addSeparator();
  fileMenu->addAction(optionsAct);
  fileMenu->addSeparator();
  fileMenu->addAction(exitAct);

  helpMenu = menuBar()->addMenu(tr("&Help"));
  helpMenu->addAction(aboutAct);
  helpMenu->addAction(aboutQtAct);
}

void VelesMainWindow::createDb() {
  database = db::create_db();
  auto database_info = new DatabaseInfo(database);
  DockWidget* dock_widget = new DockWidget;
  dock_widget->setAllowedAreas(Qt::AllDockWidgetAreas);
  dock_widget->setWindowTitle("Database");
  dock_widget->setFeatures(
      QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
  dock_widget->setWidget(database_info);
  dock_widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
  addDockWidget(Qt::LeftDockWidgetArea, dock_widget);

  connect(database_info, &DatabaseInfo::goFile,
          [this](dbif::ObjectHandle fileBlob, QString fileName) {
            createHexEditTab(fileName, fileBlob);
          });

  connect(database_info, &DatabaseInfo::newFile, [this]() { open(); });
}

void VelesMainWindow::createFileBlob(QString fileName) {
  data::BinData data(8, 0);

  if (!fileName.isEmpty()) {
    QFile file(fileName);
    file.setFileName(fileName);
    file.open(QIODevice::ReadOnly);
    QByteArray bytes = file.readAll();
    data = data::BinData(8, bytes.size(),
                         reinterpret_cast<uint8_t *>(bytes.data()));
  }

  auto promise =
      database->asyncRunMethod<dbif::RootCreateFileBlobFromDataRequest>(
          this, data, fileName);
  connect(promise, &dbif::MethodResultPromise::gotResult, [this, fileName](
                                                              dbif::PMethodReply
                                                                  reply) {
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
  auto dataModel =
      new FileBlobModel(fileBlob, {QFileInfo(fileName).fileName()});
  HexEditTab *hex = new HexEditTab(this, dataModel);
  addTab(hex, dataModel->path().join(" : ") + " - Hex");
}

}  // namespace ui
}  // namespace veles
