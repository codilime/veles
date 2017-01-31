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
#include <QMenuBar>
#include <QTabWidget>
#include <QUrl>
#include <QMenu>
#include <QCursor>
#include <QDesktopWidget>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>
#include <QLayout>

#include "ui/dockwidget.h"
#include "ui/dockwidget_native.h"

namespace veles {
namespace ui {

/*****************************************************************************/
/* QProxyStyleForDockWidgetWithIconOnTitleBar */
/*****************************************************************************/

QProxyStyleForDockWidgetWithIconOnTitleBar
    ::QProxyStyleForDockWidgetWithIconOnTitleBar(QStyle* default_style)
    : QProxyStyle(default_style) {
}

void QProxyStyleForDockWidgetWithIconOnTitleBar::drawControl(
    QStyle::ControlElement element, const QStyleOption *option,
    QPainter *painter, const QWidget *widget) const {
  if (element == QStyle::CE_DockWidgetTitle
      && !widget->windowIcon().isNull()) {
    int title_margin = baseStyle()->pixelMetric(
        QStyle::PM_DockWidgetTitleMargin);
    int icon_size = pixelMetric(QStyle::PM_SmallIconSize);
    QPoint origin(title_margin + option->rect.left(),
        option->rect.center().y() - icon_size / 2);
    painter->drawPixmap(origin,
        widget->windowIcon().pixmap(icon_size, icon_size));
    const_cast<QStyleOption*>(option)->rect
        = option->rect.adjusted(icon_size + 2 * title_margin, 0, 0, 0);
  }

  baseStyle()->drawControl(element, option, painter, widget);
}

/*****************************************************************************/
/* DockWidget */
/*****************************************************************************/

DockWidget::DockWidget() : QDockWidget(), timer_id_(0), ticks_(0),
    context_menu_(nullptr), empty_title_bar_(new QWidget(this)) {
  QStyle* style = new QProxyStyleForDockWidgetWithIconOnTitleBar(this->style());
  setStyle(style);

  setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);
  setContextMenuPolicy(Qt::CustomContextMenu);
  auto first_main_window =
      MainWindowWithDetachableDockWidgets::getFirstMainWindow();
  if (first_main_window) {
    connect(this, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
        first_main_window, SLOT(dockLocationChanged(Qt::DockWidgetArea)));
  }

  maximize_here_action_ = createMoveToNewWindowAndMaximizeAction();
  addAction(maximize_here_action_);
  detach_action_ = createMoveToNewWindowAction();
  createSplitActions();

  connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
        this, SLOT(displayContextMenu(const QPoint&)));
  connect(this, SIGNAL(topLevelChanged(bool)),
      this, SLOT(topLevelChangedNotify(bool)), Qt::QueuedConnection);
}

DockWidget::~DockWidget() {
  if (timer_id_) {
    killTimer(timer_id_);
  }
}

const QAction* DockWidget::maximizeHereAction() {
  return maximize_here_action_;
}

DockWidget* DockWidget::getParentDockWidget(QObject* obj) {
  while (obj) {
    DockWidget* dock = dynamic_cast<DockWidget*>(obj);
    if (dock == nullptr) {
      obj = obj->parent();
    } else {
      return dock;
    }
  }

  return nullptr;
}

void DockWidget::displayContextMenu(const QPoint& pos) {
  if(context_menu_) {
    context_menu_->clear();
  } else {
    context_menu_ = new QMenu(this);
  }

  context_menu_->addMenu(createMoveToDesktopMenu());
  context_menu_->addMenu(createMoveToWindowMenu());
  context_menu_->addAction(detach_action_);
  context_menu_->addAction(maximize_here_action_);

  auto parent = MainWindowWithDetachableDockWidgets::getParentMainWindow(this);
  if (parent && parent->tabifiedDockWidgets(this).size() > 0) {
    context_menu_->addAction(split_horizontally_action_);
    context_menu_->addAction(split_vertically_action_);
  }

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

void DockWidget::detachToNewTopLevelWindow() {
  auto current_main_window =
      MainWindowWithDetachableDockWidgets::getOwnerOfDockWidget(this);
  auto docks = current_main_window->findChildren<DockWidget*>();
  if (docks.size() == 1) {
    return;
  }

  setFloating(true);
  MainWindowWithDetachableDockWidgets* main_window =
      new MainWindowWithDetachableDockWidgets;
  main_window->setGeometry(QDockWidget::geometry());
  main_window->addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, this);
  main_window->show();
}

void DockWidget::detachToNewTopLevelWindowAndMaximize() {
  auto current_main_window =
      MainWindowWithDetachableDockWidgets::getOwnerOfDockWidget(this);
  auto docks = current_main_window->findChildren<DockWidget*>();
  if(docks.size() == 1) {
    current_main_window->showMaximized();
  } else {
    int screen = QApplication::desktop()->screenNumber(this);
    auto new_window = new MainWindowWithDetachableDockWidgets;
    QRect target_geometry = QApplication::desktop()->availableGeometry(screen);
    new_window->move(target_geometry.topLeft());
    new_window->resize(1000, 700);
    new_window->showMaximized();
    new_window->moveDockWidgetToWindow(this);
  }
}

void DockWidget::topLevelChangedNotify(bool top_level) {
  auto parent = MainWindowWithDetachableDockWidgets
      ::getOwnerOfDockWidget(this);

  if (parent) {
    parent->updateDocksAndTabs();
  }
}

void DockWidget::switchTitleBar(bool is_default) {
  if (is_default) {
    if (titleBarWidget() != nullptr) {
      setTitleBarWidget(nullptr);
    }
  } else if (titleBarWidget() != empty_title_bar_) {
    setTitleBarWidget(empty_title_bar_);
  }
}

void DockWidget::splitHorizontally() {
  auto parent = MainWindowWithDetachableDockWidgets::getParentMainWindow(this);

  if (parent != nullptr) {
    auto sibling = parent->findSibling(this);
    if(sibling) {
      parent->splitDockWidget2(sibling, this, Qt::Horizontal);
    }
  }
}

void DockWidget::splitVertically() {
  auto parent = MainWindowWithDetachableDockWidgets::getParentMainWindow(this);

  if (parent != nullptr) {
    auto sibling = parent->findSibling(this);
    if (sibling) {
      parent->splitDockWidget2(sibling, this, Qt::Vertical);
    }
  }
}

void DockWidget::centerTitleBarOnPosition(QPoint pos) {
  int local_pos_x = frameGeometry().width() / 2;
  int local_pos_y = (frameGeometry().height() - geometry().height()) / 2;
  QPoint startPos(local_pos_x, local_pos_y);
  move(pos - startPos);
}

void DockWidget::moveEvent(QMoveEvent *event) {
  ticks_ = 0;

  if(timer_id_ == 0) {
    timer_id_ = startTimer(step_msec_);
  }

  auto candidate = MainWindowWithDetachableDockWidgets
      ::getParentCandidateForDockWidget(this);
  MainWindowWithDetachableDockWidgets::hideAllRubberBands();

  if(candidate) {
    candidate->showRubberBand(true);
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

QAction* DockWidget::createMoveToNewWindowAction() {
  auto action = new QAction(tr("Move to new top level window"), this);
  connect(action, SIGNAL(triggered()),
      this, SLOT(detachToNewTopLevelWindow()));

  return action;
}

QAction* DockWidget::createMoveToNewWindowAndMaximizeAction() {
  auto action = new QAction(tr("Move to new top level window and maximize"), this);
  action->setShortcut(QKeySequence(Qt::Key_F12));
  action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  action->setIcon(QIcon(":/images/maximize.png"));
  connect(action, SIGNAL(triggered()),
      this, SLOT(detachToNewTopLevelWindowAndMaximize()));

  return action;
}

void DockWidget::createSplitActions() {
  split_horizontally_action_ = new QAction(tr("Split horizontally"), this);
  split_horizontally_action_->setShortcutContext(
      Qt::WidgetWithChildrenShortcut);
  split_horizontally_action_->setIcon(
      QIcon(":/images/split_horizontally.png"));
  connect(split_horizontally_action_, SIGNAL(triggered()), this,
      SLOT(splitHorizontally()));

  split_vertically_action_ = new QAction(tr("Split vertically"), this);
  split_vertically_action_->setShortcutContext(
      Qt::WidgetWithChildrenShortcut);
  split_vertically_action_->setIcon(
      QIcon(":/images/split_vertically.png"));
  connect(split_vertically_action_, SIGNAL(triggered()), this,
      SLOT(splitVertically()));
}

/*****************************************************************************/
/* TabBarEventFilter */
/*****************************************************************************/

TabBarEventFilter::TabBarEventFilter(QObject* parent) :
    QObject(parent), dragged_tab_bar_(nullptr), dragged_tab_index_(-1),
    drag_init_pos_(0, 0) {
}

void TabBarEventFilter::tabMoved(int from, int to) {
  if (dragged_tab_bar_) {
    dragged_tab_index_ = dragged_tab_bar_->currentIndex();
  }
}

bool TabBarEventFilter::eventFilter(QObject *watched, QEvent *event) {
  auto main_window = MainWindowWithDetachableDockWidgets
      ::getParentMainWindow(watched);
  if (main_window != nullptr && !main_window->dockWidgetsWithNoTitleBars()) {
    return false;
  }

  QTabBar* tab_bar = dynamic_cast<QTabBar*>(watched);
  if (!tab_bar) {
    return false;
  } else {
    connect(tab_bar, &QTabBar::tabMoved, this,
        &TabBarEventFilter::tabMoved, Qt::UniqueConnection);
  }

  if(event->type() != QEvent::MouseMove
      && event->type() != QEvent::MouseButtonPress
      && event->type() != QEvent::MouseButtonRelease
      && event->type() != QEvent::MouseButtonDblClick) {
    return false;
  }

  QMouseEvent* mouse_event = static_cast<QMouseEvent*>(event);

  if (mouse_event->type() == QEvent::MouseMove) {
    return mouseMove(tab_bar, mouse_event);
  } else if (mouse_event->type() == QEvent::MouseButtonPress) {
    return mouseButtonPress(tab_bar, mouse_event);
  } else if (mouse_event->type() == QEvent::MouseButtonRelease) {
    return mouseButtonRelease(tab_bar, mouse_event);
  } else if(mouse_event->type() == QEvent::MouseButtonDblClick) {
    return mouseButtonDblClick(tab_bar, mouse_event);
  } else {
    return false;
  }
}

bool TabBarEventFilter::mouseMove(QTabBar* tab_bar, QMouseEvent* event) {
  if(dragged_tab_bar_) {
    bool horizontal_tabs =
        dragged_tab_bar_->shape() == QTabBar::RoundedNorth
        || dragged_tab_bar_->shape() == QTabBar::RoundedSouth
        || dragged_tab_bar_->shape() == QTabBar::TriangularNorth
        || dragged_tab_bar_->shape() == QTabBar::TriangularSouth;

    if ((horizontal_tabs ? (event->pos() - drag_init_pos_).y()
        : (event->pos() - drag_init_pos_).x())
        > k_drag_treshold_ * QApplication::startDragDistance()) {
      auto window = dynamic_cast<MainWindowWithDetachableDockWidgets*>(
          tab_bar->window());
      if (window) {
        DockWidget* dock_widget =
            dynamic_cast<DockWidget*>(window->tabToDockWidget(tab_bar,
            dragged_tab_index_));
        if (dock_widget) {
          stopTabBarDragging(dragged_tab_bar_);

          dragged_tab_bar_ = nullptr;
          dragged_tab_index_ = -1;

          dock_widget->switchTitleBar(true);
          dock_widget->setFloating(true);
          dock_widget->centerTitleBarOnPosition(event->globalPos());
          QCursor::setPos(event->globalPos());
          startDockDragging(dock_widget);

          return true;
        }
      }
    }
  }

  return false;
}

bool TabBarEventFilter::mouseButtonPress(
    QTabBar* tab_bar, QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    dragged_tab_index_ = tab_bar->tabAt(event->pos());
    if (dragged_tab_index_ > -1) {
      dragged_tab_bar_ = tab_bar;
      drag_init_pos_ = event->pos();
    } else {
      dragged_tab_bar_ = nullptr;
    }
  }

  return false;
}

bool TabBarEventFilter::mouseButtonRelease(
    QTabBar* tab_bar, QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    dragged_tab_bar_ = nullptr;
    dragged_tab_index_ = -1;
  } else if (event->button() == Qt::RightButton) {
    int tab_index = tab_bar->tabAt(event->pos());
    if (tab_index > -1) {
      auto window = dynamic_cast<MainWindowWithDetachableDockWidgets*>(
          tab_bar->window());
      if (window) {
        DockWidget* dock_widget =
            dynamic_cast<DockWidget*>(window->tabToDockWidget(
            tab_bar, tab_index));
        if (dock_widget) {
          dock_widget->displayContextMenu(
              dock_widget->mapFromGlobal(event->globalPos()));
        }
      }
    }
  }

  return false;
}

bool TabBarEventFilter::mouseButtonDblClick(
    QTabBar* tab_bar, QMouseEvent* event) {
  if(event->button() == Qt::LeftButton) {
    int tab_index = tab_bar->tabAt(event->pos());
    if(tab_index > -1) {
      auto window = dynamic_cast<MainWindowWithDetachableDockWidgets*>(
          tab_bar->window());
      if(window) {
        DockWidget* dock_widget = dynamic_cast<DockWidget*>(
            window->tabToDockWidget(tab_bar, tab_index));
        if(dock_widget) {
          dock_widget->setFloating(true);
          dock_widget->centerTitleBarOnPosition(event->globalPos());
        }
      }
    }
  }

  return false;
}

/*****************************************************************************/
/* View */
/*****************************************************************************/

std::map<QString, QIcon*> View::icons_;

View::View(QString category, QString path) {
  getOrCreateIcon(category, path);
  connect(qApp, &QGuiApplication::lastWindowClosed, &View::deleteIcons);
}

View::~View() {
}

void View::getOrCreateIcon(QString category, QString icon_path) {
  QIcon* icon = nullptr;
  auto iter = icons_.find(category);
  if (iter == icons_.end()) {
    icons_[category] = icon = new QIcon(icon_path);
  } else {
    icon = iter->second;
  }
  setWindowIcon(*icon);
}

void View::deleteIcons() {
  for(auto icon : icons_) {
    delete icon.second;
  }
  icons_.clear();
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
    QWidget* parent) : QMainWindow(parent),
    dock_widgets_with_no_title_bars_(false) {
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

  icons_on_tabs_ = true;

#ifdef Q_OS_WIN
  setDockWidgetsWithNoTitleBars(true);
#endif

  tab_bar_event_filter_ = new TabBarEventFilter(this);
  connect(this, SIGNAL(childAdded(QObject*)),
      this, SLOT(childAddedNotify(QObject*)), Qt::QueuedConnection);
  connect(this, SIGNAL(childRemoved()),
      this, SLOT(updateDocksAndTabs()), Qt::QueuedConnection);

  rubber_band_ = new QRubberBand(QRubberBand::Rectangle, this);
}

MainWindowWithDetachableDockWidgets::~MainWindowWithDetachableDockWidgets() {
  main_windows_.erase(this);
  if(this == first_main_window_) {
    first_main_window_ = 0;
  }
}

DockWidget* MainWindowWithDetachableDockWidgets::addTab(QWidget *widget,
    const QString &title, DockWidget* sibling) {
  DockWidget* dock_widget = new DockWidget;
  dock_widget->setAllowedAreas(Qt::AllDockWidgetAreas);
  dock_widget->setWindowTitle(title);
  dock_widget->setFloating(false);
  dock_widget->setFeatures(QDockWidget::DockWidgetMovable |
      QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetFloatable);
  dock_widget->setWidget(widget);
  dock_widget->setWindowIcon(widget ? widget->windowIcon() : QIcon());

  if (sibling != nullptr) {
    tabifyDockWidget(sibling, dock_widget);
  } else {
    QList<QDockWidget*> dock_widgets = findChildren<QDockWidget*>();
    if (dock_widgets.size() > 0) {
      tabifyDockWidget(dock_widgets.back(), dock_widget);
    } else {
      addDockWidget(Qt::RightDockWidgetArea, dock_widget);
    }
  }

  QApplication::processEvents();
  updateDocksAndTabs();
  bringDockWidgetToFront(dock_widget);

  return dock_widget;
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
  hideAllRubberBands();
  DockWidget* dock_widget_to_tabify = 0;
  auto children = findChildren<DockWidget*>();
  if(children.size() > 0) {
    dock_widget_to_tabify = children.first();
  }

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
  updateDocksAndTabs();
  bringDockWidgetToFront(dock_widget);
}

void MainWindowWithDetachableDockWidgets::findTwoNonTabifiedDocks(
    DockWidget*& sibling1, DockWidget*& sibling2) {
  QList<DockWidget*> dock_widgets = findChildren<DockWidget*>();
  for (auto dock_widget : dock_widgets) {
    if (sibling1 == nullptr) {
      sibling1 = sibling2 = dock_widget;
    } else if (!tabifiedDockWidgets(dock_widget).contains(sibling1)) {
      sibling2 = dock_widget;
      break;
    }
  }
}

DockWidget* MainWindowWithDetachableDockWidgets::findDockNotTabifiedWith(
    DockWidget* dock_widget) {
  if (dock_widget == nullptr) {
    return nullptr;
  }

  QList<DockWidget*> dock_widgets = findChildren<DockWidget*>();
  for (auto dock : dock_widgets) {
    if (dock != dock_widget && !tabifiedDockWidgets(dock_widget).contains(dock)) {
      return dock;
    }
  }

  return nullptr;
}

DockWidget* MainWindowWithDetachableDockWidgets::findDockNotTabifiedWith(
    QWidget* widget) {
  DockWidget* dock_widget = nullptr;
  while (widget) {
    dock_widget = dynamic_cast<DockWidget*>(widget);
    if(dock_widget) break;
    widget = widget->parentWidget();
  }

  if(dock_widget) {
    return findDockNotTabifiedWith(dock_widget);
  }

  return nullptr;
}

QDockWidget* MainWindowWithDetachableDockWidgets::findSibling(
    QDockWidget* dock_widget) {
  auto siblings = tabifiedDockWidgets(dock_widget);
  if (siblings.size() > 0) {
    return siblings.first();
  } else {
    return nullptr;
  }
}

void MainWindowWithDetachableDockWidgets::setDockWidgetsWithNoTitleBars(
    bool no_title_bars) {
  dock_widgets_with_no_title_bars_ = no_title_bars;
  updateDockWidgetTitleBars();
}

bool MainWindowWithDetachableDockWidgets::dockWidgetsWithNoTitleBars() {
  return dock_widgets_with_no_title_bars_;
}

QDockWidget* MainWindowWithDetachableDockWidgets::tabToDockWidget(
    QTabBar* tab_bar, int index) {
  if(tab_bar) {
    // Based on undocumented feature (tested with Qt 5.5/5.7).
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

  return nullptr;
}

QTabBar* MainWindowWithDetachableDockWidgets::dockWidgetToTab(
    QDockWidget* dock_widget) {
  for (auto tab_bar : findChildren<QTabBar*>()) {
    for (int i = 0; i < tab_bar->count(); ++i) {
      if (tabToDockWidget(tab_bar, i) == dock_widget) {
        return tab_bar;
      }
    }
  }

  return nullptr;
}

void MainWindowWithDetachableDockWidgets::splitDockWidget2(
    QDockWidget* first, QDockWidget* second, Qt::Orientation orientation) {
  // Unfortunately we can not just call QMainWindow::splitDockWidget due to
  // it's limitations (both documented and not).

  if (!splitDockWidgetImpl(first, second, orientation)) {
    // As Qt may sometimes refuse to subdivide dock area according
    // to given orientation, it may be necessary to use the other
    // orientation first and then subdivide resulting area again
    // to obtain desired result.
    splitDockWidgetImpl(first, second,
        orientation == Qt::Horizontal ? Qt::Vertical : Qt::Horizontal);
    splitDockWidgetImpl(first, second, orientation);
  }

  updateDocksAndTabs();
}

void MainWindowWithDetachableDockWidgets::showRubberBand(bool show) {
  if(show) {
    rubber_band_->move(0, 0);
    rubber_band_->resize(size());
  }

  rubber_band_->setVisible(show);
}

MainWindowWithDetachableDockWidgets* MainWindowWithDetachableDockWidgets
    ::getParentMainWindow(QObject* obj) {
  while (obj != nullptr) {
    auto main_window = dynamic_cast<MainWindowWithDetachableDockWidgets*>(obj);
    if(main_window) {
      return main_window;
    }

    obj = obj->parent();
  }

  return nullptr;
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

  auto current_parent = MainWindowWithDetachableDockWidgets
      ::getParentMainWindow(dock_widget);
  if (current_parent
      && current_parent->geometry().intersects(dock_widget->geometry())) {
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

void MainWindowWithDetachableDockWidgets::hideAllRubberBands() {
  for (auto main_window : main_windows_) {
    main_window->showRubberBand(false);
  }
}

void MainWindowWithDetachableDockWidgets::dockLocationChanged(
    Qt::DockWidgetArea area) {
  QWidget* dock_widget = dynamic_cast<QWidget*>(sender());
  if(dock_widget) {
    auto main_window = dynamic_cast<MainWindowWithDetachableDockWidgets*>(
        dock_widget->window());
    if(main_window) {
      main_window->updateDocksAndTabs();
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

void MainWindowWithDetachableDockWidgets::childAddedNotify(QObject* child) {
  updateDocksAndTabs();

  auto tab_bar = dynamic_cast<QTabBar*>(child);
  if (tab_bar) {
    tab_bar->installEventFilter(tab_bar_event_filter_);
  }
}

void MainWindowWithDetachableDockWidgets::updateDockWidgetTitleBars() {
  auto children = findChildren<DockWidget*>();
  std::set<DockWidget*> dock_widgets;
  for(auto child : children) {
    dock_widgets.insert(child);
  }

  if (dock_widgets_with_no_title_bars_) {
    for (auto tab_bar : findChildren<QTabBar*>()) {
      tab_bar->setContextMenuPolicy(Qt::NoContextMenu);
      for (int i = 0; i < tab_bar->count(); ++i) {
        DockWidget* dock_widget =
            dynamic_cast<DockWidget*>(tabToDockWidget(tab_bar, i));
        if (dock_widget && !dock_widget->isFloating()
            && tabifiedDockWidgets(dock_widget).size() > 0) {
          dock_widget->switchTitleBar(false);
          dock_widgets.erase(dock_widget);
        }
      }
    }
  }

  for (auto dock_widget : dock_widgets) {
      dock_widget->switchTitleBar(true);
  }
}

void MainWindowWithDetachableDockWidgets::updateCloseButtonsAndIconsOnTabBars() {
  QList<QTabBar*> tab_bars = findChildren<QTabBar*>();
  for (auto tab_bar : tab_bars) {
    tab_bar->setTabsClosable(true);
    connect(tab_bar, SIGNAL(tabCloseRequested(int)), this,
        SLOT(tabCloseRequested(int)), Qt::UniqueConnection);

    for (int i = 0; i < tab_bar->count(); ++i) {
      QDockWidget* dock_widget = tabToDockWidget(tab_bar, i);
      if (dock_widget) {
        if (!(dock_widget->features() & QDockWidget::DockWidgetClosable)) {
          // Remove close button of a tab that controls non-closable QDockWidget.
          tab_bar->setTabButton(i, QTabBar::LeftSide, 0);
          tab_bar->setTabButton(i, QTabBar::RightSide, 0);
        }

        if (icons_on_tabs_ && dock_widget->widget()) {
          tab_bar->setTabIcon(i, dock_widget->widget()->windowIcon());
        } else {
          tab_bar->setTabIcon(i, QIcon());
        }
      }

      tab_bar->setIconSize(QSize(24, 24));
    }
  }
}

void MainWindowWithDetachableDockWidgets::updateDocksAndTabs() {
  updateCloseButtonsAndIconsOnTabBars();
  updateDockWidgetTitleBars();
  layout()->invalidate();
}

bool MainWindowWithDetachableDockWidgets::event(QEvent* event) {
  bool has_children = findChildren<QDockWidget*>().size() != 0;

  if (has_children) {
    if (event->type() == QEvent::ChildAdded) {
      emit childAdded(static_cast<QChildEvent*>(event)->child());
    } else if (event->type() == QEvent::ChildRemoved) {
      emit childRemoved();
    }
  } else {
    if (event->type() == QEvent::ChildRemoved && this != first_main_window_) {
      this->deleteLater();
    }
  }

  return QMainWindow::event(event);
}

bool MainWindowWithDetachableDockWidgets::splitDockWidgetImpl(
    QDockWidget* first, QDockWidget* second, Qt::Orientation orientation) {
  // As QMainWindow::splitDockWidget doesn't handle multiple dock widgets
  // tabified together, it's necessary to detach all excessive docks,
  // split dock area and then reattach the docks.
  auto tabified_docks = tabifiedDockWidgets(first);
  for (auto dock_widget : tabified_docks) {
    removeDockWidget(dock_widget);
  }
  tabified_docks.removeOne(second);

  second->show();
  splitDockWidget(first, second, orientation);

  for (auto dock_widget : tabified_docks) {
    dock_widget->show();
    tabifyDockWidget(first, dock_widget);
  }

  if (tabifiedDockWidgets(first).contains(second)) {
    return false;
  }

  return true;
}

}  // namespace ui
}  // namespace veles
