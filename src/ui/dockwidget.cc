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
#include <QCursor>
#include <QDesktopWidget>
#include <QLayout>
#include <QMenu>
#include <QMenuBar>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>
#include <QTabWidget>
#include <QUrl>

#include "ui/dockwidget.h"
#include "ui/dockwidget_native.h"
#include "ui/nodewidget.h"
#include "util/settings/shortcuts.h"
#include "visualization/panel.h"

namespace veles {
namespace ui {

using util::settings::shortcuts::ShortcutsModel;

/*****************************************************************************/
/* QProxyStyleForDockWidgetWithIconOnTitleBar */
/*****************************************************************************/

QProxyStyleForDockWidgetWithIconOnTitleBar::
    QProxyStyleForDockWidgetWithIconOnTitleBar(QStyle* default_style)
    : QProxyStyle(default_style) {}

void QProxyStyleForDockWidgetWithIconOnTitleBar::drawControl(
    QStyle::ControlElement element, const QStyleOption* option,
    QPainter* painter, const QWidget* widget) const {
  if (element == QStyle::CE_DockWidgetTitle && !widget->windowIcon().isNull()) {
    int title_margin =
        baseStyle()->pixelMetric(QStyle::PM_DockWidgetTitleMargin);
    int icon_size = pixelMetric(QStyle::PM_SmallIconSize);
    QPoint origin(title_margin + option->rect.left(),
                  option->rect.center().y() - icon_size / 2);
    painter->drawPixmap(origin,
                        widget->windowIcon().pixmap(icon_size, icon_size));
    const_cast<QStyleOption*>(option)->rect =
        option->rect.adjusted(icon_size + 2 * title_margin, 0, 0, 0);
  }

  baseStyle()->drawControl(element, option, painter, widget);
}

/*****************************************************************************/
/* ActivateDockEventFilter */
/*****************************************************************************/

ActivateDockEventFilter::ActivateDockEventFilter(QObject* parent)
    : QObject(parent) {}

bool ActivateDockEventFilter::eventFilter(QObject* watched, QEvent* event) {
  if (event->type() == QEvent::MouseButtonPress) {
    auto parent = DockWidget::getParentDockWidget(watched);
    if (parent != nullptr) {
      MainWindowWithDetachableDockWidgets::setActiveDockWidget(parent);
    }
  }

  return false;
}

/*****************************************************************************/
/* DockWidget */
/*****************************************************************************/

DockWidget::DockWidget() : empty_title_bar_(new QWidget(this)) {
  QStyle* style = new QProxyStyleForDockWidgetWithIconOnTitleBar(this->style());
  setStyle(style);

  setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);
  setContextMenuPolicy(Qt::CustomContextMenu);
  auto first_main_window =
      MainWindowWithDetachableDockWidgets::getFirstMainWindow();
  if (first_main_window != nullptr) {
    connect(this, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
            first_main_window, SLOT(dockLocationChanged(Qt::DockWidgetArea)));
  }

  maximize_here_action_ = createMoveToNewWindowAndMaximizeAction();
  addAction(maximize_here_action_);
  detach_action_ = createMoveToNewWindowAction();
  addAction(detach_action_);
  createSplitActions();

  connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this,
          SLOT(displayContextMenu(const QPoint&)));
  connect(this, SIGNAL(topLevelChanged(bool)), this,
          SLOT(topLevelChangedNotify(bool)), Qt::QueuedConnection);

  next_tab_action_ = ShortcutsModel::getShortcutsModel()->createQAction(
      util::settings::shortcuts::SWITCH_TAB_NEXT, this,
      Qt::WidgetWithChildrenShortcut);
  connect(next_tab_action_, &QAction::triggered, [this]() {
    MainWindowWithDetachableDockWidgets::focusNextPrevDock(this, /*next=*/true);
  });
  addAction(next_tab_action_);

  prev_tab_action_ = ShortcutsModel::getShortcutsModel()->createQAction(
      util::settings::shortcuts::SWITCH_TAB_PREV, this,
      Qt::WidgetWithChildrenShortcut);
  connect(prev_tab_action_, &QAction::triggered, [this]() {
    MainWindowWithDetachableDockWidgets::focusNextPrevDock(this,
                                                           /*next=*/false);
  });
  addAction(prev_tab_action_);
}

DockWidget::~DockWidget() {
  if (timer_id_ != 0) {
    killTimer(timer_id_);
  }
}

const QAction* DockWidget::maximizeHereAction() {
  return maximize_here_action_;
}

DockWidget* DockWidget::getParentDockWidget(QObject* obj) {
  while (obj != nullptr) {
    auto* dock = dynamic_cast<DockWidget*>(obj);
    if (dock == nullptr) {
      auto* widget = dynamic_cast<QWidget*>(obj);
      if (widget != nullptr && widget->isTopLevel()) {
        return nullptr;
      }
      obj = obj->parent();
    } else {
      return dock;
    }
  }

  return nullptr;
}

void DockWidget::addCloseAction() {
  if (dock_close_action_ == nullptr) {
    dock_close_action_ = ShortcutsModel::getShortcutsModel()->createQAction(
        util::settings::shortcuts::DOCK_CLOSE, this,
        Qt::WidgetWithChildrenShortcut);
    connect(dock_close_action_, &QAction::triggered, [this]() {
      deleteLater();
      auto parent =
          MainWindowWithDetachableDockWidgets::getOwnerOfDockWidget(this);

      if (parent != nullptr) {
        auto tab_pair = parent->dockWidgetToTab(this);
        if (tab_pair.first != nullptr) {
          tab_pair.first->tabCloseRequested(tab_pair.second);
        } else {
          deleteLater();
        }
      }
    });
    addAction(dock_close_action_);
  }
}

void DockWidget::displayContextMenu(const QPoint& pos) {
  if (context_menu_ != nullptr) {
    context_menu_->clear();
  } else {
    context_menu_ = new QMenu(this);
  }

  context_menu_->addMenu(createMoveToDesktopMenu());
  context_menu_->addMenu(createMoveToWindowMenu());
  context_menu_->addAction(detach_action_);
  context_menu_->addAction(maximize_here_action_);

  auto parent = MainWindowWithDetachableDockWidgets::getParentMainWindow(this);
  if (parent != nullptr && !parent->tabifiedDockWidgets(this).empty()) {
    context_menu_->addAction(split_horizontally_action_);
    context_menu_->addAction(split_vertically_action_);
  }

  context_menu_->popup(mapToGlobal(pos));
}

void DockWidget::moveToDesktop() {
  auto action = dynamic_cast<QAction*>(sender());
  if (action != nullptr) {
    bool ok;
    int screen = action->data().toInt(&ok);
    if (ok) {
      MainWindowWithDetachableDockWidgets::getOrCreateWindowForScreen(screen)
          ->moveDockWidgetToWindow(this);
    }
  }
}

void DockWidget::moveToWindow() {
  auto action = dynamic_cast<QAction*>(sender());
  if (action != nullptr) {
    MainWindowWithDetachableDockWidgets* window =
        reinterpret_cast<MainWindowWithDetachableDockWidgets*>(
            qvariant_cast<quintptr>(action->data()));
    if (window != nullptr) {
      window->moveDockWidgetToWindow(this);
    }
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
  auto* main_window = new MainWindowWithDetachableDockWidgets;
  main_window->setGeometry(QDockWidget::geometry());
  main_window->addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, this);
  main_window->show();
}

void DockWidget::detachToNewTopLevelWindowAndMaximize() {
  auto current_main_window =
      MainWindowWithDetachableDockWidgets::getOwnerOfDockWidget(this);
  auto docks = current_main_window->findChildren<DockWidget*>();
  if (docks.size() == 1) {
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

void DockWidget::topLevelChangedNotify(bool /*top_level*/) {
  auto parent = MainWindowWithDetachableDockWidgets::getOwnerOfDockWidget(this);

  if (parent != nullptr) {
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
    if (sibling != nullptr) {
      parent->splitDockWidget2(sibling, this, Qt::Horizontal);
    }
  }
}

void DockWidget::splitVertically() {
  auto parent = MainWindowWithDetachableDockWidgets::getParentMainWindow(this);

  if (parent != nullptr) {
    auto sibling = parent->findSibling(this);
    if (sibling != nullptr) {
      parent->splitDockWidget2(sibling, this, Qt::Vertical);
    }
  }
}

void DockWidget::centerTitleBarOnPosition(const QPoint& pos) {
  int local_pos_x = frameGeometry().width() / 2;
  int local_pos_y = (frameGeometry().height() - geometry().height()) / 2;
  QPoint startPos(local_pos_x, local_pos_y);
  move(pos - startPos);
}

void DockWidget::focusInEvent(QFocusEvent* /*event*/) {
  if (widget() != nullptr) {
    widget()->setFocus();
  }
}

void DockWidget::moveEvent(QMoveEvent* /*event*/) {
  ticks_ = 0;

  if (timer_id_ == 0) {
    timer_id_ = startTimer(step_msec_);
  }

  auto candidate =
      MainWindowWithDetachableDockWidgets::getParentCandidateForDockWidget(
          this);
  MainWindowWithDetachableDockWidgets::hideAllRubberBands();

  if (candidate != nullptr) {
    candidate->showRubberBand(true);
  }
}

void DockWidget::timerEvent(QTimerEvent* /*event*/) {
  if (isFloating()) {
    ++ticks_;
  }

  if (QApplication::mouseButtons() != Qt::NoButton) {
    ticks_ = 0;
  }

  if (ticks_ > max_ticks_ && isFloating()) {
    if (MainWindowWithDetachableDockWidgets::intersectsWithAnyMainWindow(
            this)) {
      auto candidate =
          MainWindowWithDetachableDockWidgets::getParentCandidateForDockWidget(
              this);
      if (candidate != nullptr) {
        candidate->moveDockWidgetToWindow(this);
      }
    } else {
      auto* main_window = new MainWindowWithDetachableDockWidgets;
      main_window->setGeometry(QDockWidget::geometry());
      main_window->addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, this);
      main_window->show();
    }
  }

  if (timer_id_ != 0 && (ticks_ > max_ticks_ || !isFloating())) {
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
        already_there
            ? QString("Desktop %1 (it's already there)").arg(screen + 1)
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

    if (!isFloating() && window->findChildren<DockWidget*>().contains(this)) {
      already_there = true;
    }

    auto action = new QAction(
        already_there ? window->windowTitle() + " (it's already there)"
                      : window->windowTitle(),
        menu_move_to_window);
    action->setData(quintptr(window));
    action->setEnabled(!already_there);
    connect(action, SIGNAL(triggered()), this, SLOT(moveToWindow()));
    menu_move_to_window->addAction(action);
  }

  return menu_move_to_window;
}

QAction* DockWidget::createMoveToNewWindowAction() {
  auto action = ShortcutsModel::getShortcutsModel()->createQAction(
      util::settings::shortcuts::DOCK_MOVE_TO_TOP, this,
      Qt::WidgetWithChildrenShortcut);
  connect(action, SIGNAL(triggered()), this, SLOT(detachToNewTopLevelWindow()));

  return action;
}

QAction* DockWidget::createMoveToNewWindowAndMaximizeAction() {
  auto action = ShortcutsModel::getShortcutsModel()->createQAction(
      util::settings::shortcuts::DOCK_MOVE_TO_TOP_MAX, this,
      QIcon(":/images/maximize.png"), Qt::WidgetWithChildrenShortcut);
  connect(action, SIGNAL(triggered()), this,
          SLOT(detachToNewTopLevelWindowAndMaximize()));

  return action;
}

void DockWidget::createSplitActions() {
  split_horizontally_action_ =
      ShortcutsModel::getShortcutsModel()->createQAction(
          util::settings::shortcuts::DOCK_SPLIT_HORIZ, this,
          QIcon(":/images/split_horizontally.png"),
          Qt::WidgetWithChildrenShortcut);
  connect(split_horizontally_action_, SIGNAL(triggered()), this,
          SLOT(splitHorizontally()));
  addAction(split_horizontally_action_);

  split_vertically_action_ = ShortcutsModel::getShortcutsModel()->createQAction(
      util::settings::shortcuts::DOCK_SPLIT_VERT, this,
      QIcon(":/images/split_vertically.png"), Qt::WidgetWithChildrenShortcut);
  connect(split_vertically_action_, SIGNAL(triggered()), this,
          SLOT(splitVertically()));
  addAction(split_vertically_action_);
}

/*****************************************************************************/
/* TabBarEventFilter */
/*****************************************************************************/

TabBarEventFilter::TabBarEventFilter(QObject* parent)
    : QObject(parent), drag_init_pos_(0, 0) {}

void TabBarEventFilter::tabMoved(int /*from*/, int /*to*/) {
  if (dragged_tab_bar_ != nullptr) {
    dragged_tab_index_ = dragged_tab_bar_->currentIndex();
  }
}

void TabBarEventFilter::currentChanged(int index) {
  auto main_window =
      MainWindowWithDetachableDockWidgets::getParentMainWindow(sender());
  if (main_window != nullptr) {
    auto dock_widget = dynamic_cast<DockWidget*>(
        main_window->tabToDockWidget(dynamic_cast<QTabBar*>(sender()), index));

    if (dock_widget != nullptr) {
      MainWindowWithDetachableDockWidgets::setActiveDockWidget(dock_widget);
    }
  }
}

bool TabBarEventFilter::eventFilter(QObject* watched, QEvent* event) {
  auto* tab_bar = dynamic_cast<QTabBar*>(watched);
  if (tab_bar == nullptr) {
    return false;
  }

  connect(tab_bar, &QTabBar::currentChanged, this,
          &TabBarEventFilter::currentChanged, Qt::UniqueConnection);

  auto main_window =
      MainWindowWithDetachableDockWidgets::getParentMainWindow(watched);

  if (main_window != nullptr && !main_window->dockWidgetsWithNoTitleBars()) {
    return false;
  }

  connect(tab_bar, &QTabBar::tabMoved, this, &TabBarEventFilter::tabMoved,
          Qt::UniqueConnection);

  if (event->type() != QEvent::MouseMove &&
      event->type() != QEvent::MouseButtonPress &&
      event->type() != QEvent::MouseButtonRelease &&
      event->type() != QEvent::MouseButtonDblClick) {
    return false;
  }

  auto* mouse_event = static_cast<QMouseEvent*>(event);

  if (mouse_event->type() == QEvent::MouseMove) {
    return mouseMove(tab_bar, mouse_event);
  }
  if (mouse_event->type() == QEvent::MouseButtonPress) {
    return mouseButtonPress(tab_bar, mouse_event);
  }
  if (mouse_event->type() == QEvent::MouseButtonRelease) {
    return mouseButtonRelease(tab_bar, mouse_event);
  }
  if (mouse_event->type() == QEvent::MouseButtonDblClick) {
    return mouseButtonDblClick(tab_bar, mouse_event);
  }
  return false;
}

bool TabBarEventFilter::mouseMove(QTabBar* tab_bar, QMouseEvent* event) {
  if (dragged_tab_bar_ != nullptr) {
    bool horizontal_tabs =
        dragged_tab_bar_->shape() == QTabBar::RoundedNorth ||
        dragged_tab_bar_->shape() == QTabBar::RoundedSouth ||
        dragged_tab_bar_->shape() == QTabBar::TriangularNorth ||
        dragged_tab_bar_->shape() == QTabBar::TriangularSouth;

    if ((horizontal_tabs ? (event->pos() - drag_init_pos_).y()
                         : (event->pos() - drag_init_pos_).x()) >
        k_drag_treshold_ * QApplication::startDragDistance()) {
      auto window =
          dynamic_cast<MainWindowWithDetachableDockWidgets*>(tab_bar->window());
      if (window != nullptr) {
        auto* dock_widget = dynamic_cast<DockWidget*>(
            window->tabToDockWidget(tab_bar, dragged_tab_index_));
        if (dock_widget != nullptr) {
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

bool TabBarEventFilter::mouseButtonPress(QTabBar* tab_bar, QMouseEvent* event) {
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

bool TabBarEventFilter::mouseButtonRelease(QTabBar* tab_bar,
                                           QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    dragged_tab_bar_ = nullptr;
    dragged_tab_index_ = -1;
  } else if (event->button() == Qt::RightButton) {
    int tab_index = tab_bar->tabAt(event->pos());
    if (tab_index > -1) {
      auto window =
          dynamic_cast<MainWindowWithDetachableDockWidgets*>(tab_bar->window());
      if (window != nullptr) {
        auto* dock_widget = dynamic_cast<DockWidget*>(
            window->tabToDockWidget(tab_bar, tab_index));
        if (dock_widget != nullptr) {
          dock_widget->displayContextMenu(
              dock_widget->mapFromGlobal(event->globalPos()));
        }
      }
    }
  }

  return false;
}

bool TabBarEventFilter::mouseButtonDblClick(QTabBar* tab_bar,
                                            QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    int tab_index = tab_bar->tabAt(event->pos());
    if (tab_index > -1) {
      auto window =
          dynamic_cast<MainWindowWithDetachableDockWidgets*>(tab_bar->window());
      if (window != nullptr) {
        auto* dock_widget = dynamic_cast<DockWidget*>(
            window->tabToDockWidget(tab_bar, tab_index));
        if (dock_widget != nullptr) {
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

View::View(const QString& category, const QString& path) {
  getOrCreateIcon(category, path);
  connect(qApp, &QGuiApplication::lastWindowClosed, &View::deleteIcons);
  setFocusPolicy(Qt::StrongFocus);
}

View::~View() {}

void View::getOrCreateIcon(const QString& category, const QString& icon_path) {
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
  for (auto icon : icons_) {
    delete icon.second;
  }
  icons_.clear();
}

void View::createVisualization(
    MainWindowWithDetachableDockWidgets* main_window,
    const QSharedPointer<FileBlobModel>& data_model) {
  auto* panel = new visualization::VisualizationPanel(main_window, data_model);
  panel->setData(
      QByteArray(reinterpret_cast<const char*>(data_model->binData().rawData()),
                 static_cast<int>(data_model->binData().size())));
  panel->setAttribute(Qt::WA_DeleteOnClose);

  // FIXME: main_window_ needs to be updated when docks are moved around,
  // then we can use this behaviour without any weird effects
  // auto sibling = DockWidget::getParentDockWidget(this);
  DockWidget* sibling = nullptr;

  auto dock_widget =
      main_window->addTab(panel, data_model->path().join(" : "), sibling);
  connect(dock_widget, &DockWidget::visibilityChanged, panel,
          &visualization::VisualizationPanel::visibilityChanged);
  //  if (sibling == nullptr) {
  //    main_window->addDockWidget(Qt::RightDockWidgetArea, dock_widget);
  //  }
}

void View::createHexEditor(MainWindowWithDetachableDockWidgets* main_window,
                           const QSharedPointer<FileBlobModel>& data_model) {
  QSharedPointer<QItemSelectionModel> new_selection_model(
      new QItemSelectionModel(data_model.data()));
  auto* node_edit =
      new NodeWidget(main_window, data_model, new_selection_model);

  // FIXME: main_window_ needs to be updated when docks are moved around,
  // then we can use this behaviour without any weird effects
  // auto sibling = DockWidget::getParentDockWidget(this);
  DockWidget* sibling = nullptr;

  main_window->addTab(node_edit, data_model->path().join(" : "), sibling);
  //  if (sibling == nullptr) {
  //    main_window->addDockWidget(Qt::RightDockWidgetArea, dock_widget);
  //  }
}

/*****************************************************************************/
/* MainWindowWithDetachableDockWidgets */
/*****************************************************************************/

std::set<MainWindowWithDetachableDockWidgets*>
    MainWindowWithDetachableDockWidgets::main_windows_;
MainWindowWithDetachableDockWidgets*
    MainWindowWithDetachableDockWidgets::first_main_window_ = nullptr;
int MainWindowWithDetachableDockWidgets::last_created_window_id_ = 0;
QPointer<DockWidget> MainWindowWithDetachableDockWidgets::active_dock_widget_ =
    nullptr;

MainWindowWithDetachableDockWidgets::MainWindowWithDetachableDockWidgets(
    QWidget* parent)
    : QMainWindow(parent) {
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
    connect(qApp, &QApplication::focusChanged, this,
            &MainWindowWithDetachableDockWidgets::focusChanged);
    connect(this, &MainWindowWithDetachableDockWidgets::updateFocus, this,
            &MainWindowWithDetachableDockWidgets::delayedFocusChanged,
            Qt::QueuedConnection);
    auto activate_dock_event_filter = new ActivateDockEventFilter(this);
    qApp->installEventFilter(activate_dock_event_filter);
  } else {
    setAttribute(Qt::WA_QuitOnClose, false);
    setWindowTitle(QString("Veles - window #%1").arg(last_created_window_id_));
  }

  icons_on_tabs_ = true;

#ifdef Q_OS_WIN
  setDockWidgetsWithNoTitleBars(true);
#endif

  mark_active_dock_widget_ = true;

  tab_bar_event_filter_ = new TabBarEventFilter(this);
  connect(this, SIGNAL(childAdded(QObject*)), this,
          SLOT(childAddedNotify(QObject*)), Qt::QueuedConnection);
  connect(this, SIGNAL(childRemoved()), this, SLOT(updateDocksAndTabs()),
          Qt::QueuedConnection);

  rubber_band_ = new QRubberBand(QRubberBand::Rectangle, this);
}

MainWindowWithDetachableDockWidgets::~MainWindowWithDetachableDockWidgets() {
  main_windows_.erase(this);
  if (this == first_main_window_) {
    first_main_window_ = nullptr;
  }
}

DockWidget* MainWindowWithDetachableDockWidgets::addTab(QWidget* widget,
                                                        const QString& title,
                                                        DockWidget* sibling) {
  auto* dock_widget = new DockWidget;
  dock_widget->setAllowedAreas(Qt::AllDockWidgetAreas);
  dock_widget->setWindowTitle(title);
  dock_widget->setFloating(false);
  dock_widget->setFeatures(QDockWidget::DockWidgetMovable |
                           QDockWidget::DockWidgetClosable |
                           QDockWidget::DockWidgetFloatable);
  dock_widget->setWidget(widget);
  dock_widget->setWindowIcon(widget != nullptr ? widget->windowIcon()
                                               : QIcon());
  dock_widget->addCloseAction();

  if (sibling != nullptr) {
    tabifyDockWidget(sibling, dock_widget);
  } else {
    auto dock_widgets = findChildren<DockWidget*>();
    if (!dock_widgets.empty()) {
      tabifyDockWidget(dock_widgets.back(), dock_widget);
    } else {
      addDockWidget(Qt::RightDockWidgetArea, dock_widget);
    }
  }

  QApplication::processEvents();
  updateDocksAndTabs();
  bringDockWidgetToFront(dock_widget);
  setActiveDockWidget(dock_widget);

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
  DockWidget* dock_widget_to_tabify = nullptr;
  auto children = findChildren<DockWidget*>();
  if (!children.empty()) {
    dock_widget_to_tabify = children.first();
  }

  if (dock_widget->isFloating()) {
    if (children.contains(dock_widget)) {
      dock_widget->setFloating(false);
      return;
    }
    dock_widget->setParent(nullptr);

    for (auto candidate : findChildren<DockWidget*>()) {
      if (candidate->geometry().intersects(dock_widget->geometry()) &&
          !candidate->isFloating()) {
        dock_widget_to_tabify = candidate;
        break;
      }
    }
  }

  if (dock_widget_to_tabify != nullptr) {
    tabifyDockWidget(dock_widget_to_tabify, dock_widget);
  } else {
    addDockWidget(Qt::LeftDockWidgetArea, dock_widget);
  }

  QApplication::processEvents();
  updateDocksAndTabs();
  bringDockWidgetToFront(dock_widget);
  setActiveDockWidget(dock_widget);
}

void MainWindowWithDetachableDockWidgets::findTwoNonTabifiedDocks(
    DockWidget** sibling1, DockWidget** sibling2) {
  auto dock_widgets = findChildren<DockWidget*>();
  for (auto dock_widget : dock_widgets) {
    if (*sibling1 == nullptr) {
      *sibling1 = *sibling2 = dock_widget;
    } else if (!tabifiedDockWidgets(dock_widget).contains(*sibling1)) {
      *sibling2 = dock_widget;
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
    if (dock != dock_widget &&
        !tabifiedDockWidgets(dock_widget).contains(dock)) {
      return dock;
    }
  }

  return nullptr;
}

DockWidget* MainWindowWithDetachableDockWidgets::findDockNotTabifiedWith(
    QWidget* widget) {
  DockWidget* dock_widget = nullptr;
  while (widget != nullptr) {
    dock_widget = dynamic_cast<DockWidget*>(widget);
    if (dock_widget != nullptr) {
      break;
    }
    widget = widget->parentWidget();
  }

  if (dock_widget != nullptr) {
    return findDockNotTabifiedWith(dock_widget);
  }

  return nullptr;
}

QDockWidget* MainWindowWithDetachableDockWidgets::findSibling(
    QDockWidget* dock_widget) {
  auto siblings = tabifiedDockWidgets(dock_widget);
  if (!siblings.empty()) {
    return siblings.first();
  }
  return nullptr;
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
  if (tab_bar != nullptr) {
    // Based on undocumented feature (tested with Qt 5.5/5.7).
    // QTabBars that control visibility of QDockWidgets grouped in
    // QDockWidgetGroupWindows hold ids of respective QDockAreaLayoutInfos
    // in tabData. Conveniently those ids are just pointers to
    // QDockWidgets (stored as quintptr) and can be retrieved through public
    // interface.
    QDockWidget* dock = reinterpret_cast<QDockWidget*>(
        qvariant_cast<quintptr>(tab_bar->tabData(index)));

    return dock;
  }

  return nullptr;
}

QPair<QTabBar*, int> MainWindowWithDetachableDockWidgets::dockWidgetToTab(
    QDockWidget* dock_widget) {
  for (auto tab_bar : findChildren<QTabBar*>()) {
    for (int i = 0; i < tab_bar->count(); ++i) {
      if (tabToDockWidget(tab_bar, i) == dock_widget) {
        return qMakePair(tab_bar, i);
      }
    }
  }

  return qMakePair(nullptr, -1);
}

void MainWindowWithDetachableDockWidgets::splitDockWidget2(
    QDockWidget* first, QDockWidget* second, Qt::Orientation orientation) {
  // Unfortunately we can not just call QMainWindow::splitDockWidget due to
  // its limitations (both documented and not).
  splitDockWidget2(this, first, second, orientation);
  updateDocksAndTabs();
}

void MainWindowWithDetachableDockWidgets::showRubberBand(bool show) {
  if (show) {
    rubber_band_->move(0, 0);
    rubber_band_->resize(size());
  }

  rubber_band_->setVisible(show);
}

void MainWindowWithDetachableDockWidgets::splitDockWidget2(
    QMainWindow* main_window, QDockWidget* first, QDockWidget* second,
    Qt::Orientation orientation) {
  if (!splitDockWidgetImpl(main_window, first, second, orientation)) {
    // As Qt may sometimes refuse to subdivide dock area according
    // to given orientation, it may be necessary to use the other
    // orientation first and then subdivide resulting area again
    // to obtain desired result.
    splitDockWidgetImpl(
        main_window, first, second,
        orientation == Qt::Horizontal ? Qt::Vertical : Qt::Horizontal);
    splitDockWidgetImpl(main_window, first, second, orientation);
  }
}

MainWindowWithDetachableDockWidgets*
MainWindowWithDetachableDockWidgets::getParentMainWindow(QObject* obj) {
  while (obj != nullptr) {
    auto main_window = dynamic_cast<MainWindowWithDetachableDockWidgets*>(obj);
    if (main_window != nullptr) {
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

  for (auto main_window : main_windows_) {
    if (main_window->frameGeometry().intersects(dock_widget->frameGeometry())) {
      return true;
    }
  }

  return false;
}

MainWindowWithDetachableDockWidgets*
MainWindowWithDetachableDockWidgets::getParentCandidateForDockWidget(
    DockWidget* dock_widget) {
  if (dock_widget == nullptr || !dock_widget->isFloating()) {
    return nullptr;
  }

  auto current_parent =
      MainWindowWithDetachableDockWidgets::getParentMainWindow(dock_widget);
  if (current_parent != nullptr &&
      current_parent->geometry().intersects(dock_widget->geometry())) {
    return nullptr;
  }

  for (auto window : main_windows_) {
    if (window->geometry().intersects(dock_widget->geometry()) &&
        !window->isAncestorOf(dock_widget)) {
      if (window->findChildren<DockWidget*>().contains(dock_widget)) {
        return nullptr;
      }

      return window;
    }
  }

  return nullptr;
}

const std::set<MainWindowWithDetachableDockWidgets*>&
MainWindowWithDetachableDockWidgets::getMainWindows() {
  return main_windows_;
}

MainWindowWithDetachableDockWidgets*
MainWindowWithDetachableDockWidgets::getFirstMainWindow() {
  return first_main_window_;
}

MainWindowWithDetachableDockWidgets*
MainWindowWithDetachableDockWidgets::getOrCreateWindowForScreen(int screen) {
  if (screen >= QApplication::desktop()->screenCount()) {
    return nullptr;
  }

  for (auto window : MainWindowWithDetachableDockWidgets::getMainWindows()) {
    int wnd_screen = QApplication::desktop()->screenNumber(window);
    if (wnd_screen == screen) {
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

MainWindowWithDetachableDockWidgets*
MainWindowWithDetachableDockWidgets::getOwnerOfDockWidget(
    DockWidget* dock_widget) {
  for (auto window : main_windows_) {
    if (window->findChildren<DockWidget*>().contains(dock_widget)) {
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

void MainWindowWithDetachableDockWidgets::setActiveDockWidget(
    DockWidget* dock_widget) {
  // TODO(jacek-izykowski): more in depth look into active dock marking
  // In theory this condition can cause some degradation in drawing which dock
  // is active, but currently it isn't working correctly despite it, and this
  // condition fixes some more severe problems.
  if (active_dock_widget_ != dock_widget) {
    active_dock_widget_ = dock_widget;

    for (auto window : main_windows_) {
      window->updateDocksAndTabs();
    }
    dock_widget->setFocus();
  }
}

void MainWindowWithDetachableDockWidgets::focusNextPrevDock(
    DockWidget* dock_widget, bool next) {
  auto parent = getOwnerOfDockWidget(dock_widget);

  if (parent != nullptr) {
    auto tab_pair = parent->dockWidgetToTab(dock_widget);
    if (tab_pair.first != nullptr) {
      int index;
      if (next) {
        index = (tab_pair.second + 1) % tab_pair.first->count();
      } else {
        index = (tab_pair.second - 1 + tab_pair.first->count()) %
                tab_pair.first->count();
      }
      tab_pair.first->setCurrentIndex(index);
    }
  }
}

std::set<MainWindowWithDetachableDockWidgets*>
MainWindowWithDetachableDockWidgets::allMainWindows() {
  return main_windows_;
}

void MainWindowWithDetachableDockWidgets::dockLocationChanged(
    Qt::DockWidgetArea /*area*/) {
  auto* dock_widget = dynamic_cast<QWidget*>(sender());
  if (dock_widget != nullptr) {
    auto main_window = dynamic_cast<MainWindowWithDetachableDockWidgets*>(
        dock_widget->window());
    if (main_window != nullptr) {
      main_window->updateDocksAndTabs();
    }
  }
}

void MainWindowWithDetachableDockWidgets::tabCloseRequested(int index) {
  auto* tab_bar = dynamic_cast<QTabBar*>(sender());
  if (tab_bar != nullptr) {
    QDockWidget* dock_widget = tabToDockWidget(tab_bar, index);
    if (dock_widget != nullptr) {
      dock_widget->deleteLater();
    }
    tab_bar->removeTab(index);
  }
}

void MainWindowWithDetachableDockWidgets::childAddedNotify(QObject* child) {
  updateDocksAndTabs();

  // We need to determine if a new child is actually an instance of QTabBar.
  // It's not possible when ChildAdded event is delivered (initialization of
  // an object might not be finished at that stage). However, as we allow
  // main loop to process other events before calling childAddedNotify, it's
  // possible that a child has already been deleted. It implies that we need
  // to check if child is still a valid pointer.

  if (!findChildren<QObject*>().contains(child)) {
    return;
  }

  auto tab_bar = dynamic_cast<QTabBar*>(child);
  if (tab_bar != nullptr) {
    tab_bar->installEventFilter(tab_bar_event_filter_);
  }
}

void MainWindowWithDetachableDockWidgets::updateDockWidgetTitleBars() {
  auto children = findChildren<DockWidget*>();
  std::set<DockWidget*> dock_widgets;
  for (auto child : children) {
    dock_widgets.insert(child);
  }

  if (dock_widgets_with_no_title_bars_) {
    for (auto tab_bar : findChildren<QTabBar*>()) {
      tab_bar->setContextMenuPolicy(Qt::NoContextMenu);
      for (int i = 0; i < tab_bar->count(); ++i) {
        auto* dock_widget =
            dynamic_cast<DockWidget*>(tabToDockWidget(tab_bar, i));
        if (dock_widget != nullptr && !dock_widget->isFloating() &&
            !tabifiedDockWidgets(dock_widget).empty()) {
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

void MainWindowWithDetachableDockWidgets::
    updateCloseButtonsAndIconsOnTabBars() {
  QList<QTabBar*> tab_bars = findChildren<QTabBar*>();
  for (auto tab_bar : tab_bars) {
    tab_bar->setTabsClosable(true);
    connect(tab_bar, SIGNAL(tabCloseRequested(int)), this,
            SLOT(tabCloseRequested(int)), Qt::UniqueConnection);

    for (int i = 0; i < tab_bar->count(); ++i) {
      QDockWidget* dock_widget = tabToDockWidget(tab_bar, i);
      if (dock_widget != nullptr) {
        if (!(dock_widget->features() & QDockWidget::DockWidgetClosable)) {
          // Remove close button of a tab that controls non-closable
          // QDockWidget.
          tab_bar->setTabButton(i, QTabBar::LeftSide, nullptr);
          tab_bar->setTabButton(i, QTabBar::RightSide, nullptr);
        }

        if (icons_on_tabs_ && dock_widget->widget() != nullptr) {
          tab_bar->setTabIcon(i, dock_widget->widget()->windowIcon());
        } else {
          tab_bar->setTabIcon(i, QIcon());
        }
      }

      tab_bar->setIconSize(QSize(24, 24));
    }
  }
}

void MainWindowWithDetachableDockWidgets::updateActiveDockWidget() {
  QApplication::processEvents();

  QString tab_bar_stylesheet = QString(
      "QTabBar::tab {"
      "background : palette(window);"
      "color : palette(window-text);"
      "border-top-left-radius: 4px;"
      "border-top-right-radius: 4px;"
      "border: 1px solid palette(shadow);"
      "padding: 4px;"
      "}"
      "QTabBar::tab:selected {"
      "border-bottom: 0px solid palette(shadow);"
      "}"
      "QTabBar::tab:!selected {"
      "margin-top: 2px;"
      "}");

  QList<QTabBar*> tab_bars = findChildren<QTabBar*>();
  for (auto tab_bar : tab_bars) {
    if (mark_active_dock_widget_ && dock_widgets_with_no_title_bars_ &&
        active_dock_widget_ == dynamic_cast<DockWidget*>(tabToDockWidget(
                                   tab_bar, tab_bar->currentIndex())) &&
        active_dock_widget_ != nullptr) {
      QString stylesheet =
          tab_bar_stylesheet + QString(
                                   "QTabBar::tab:selected {"
                                   "background : palette(highlight);"
                                   "color : palette(highlighted-text);"
                                   "border-bottom: 0px solid palette(shadow);"
                                   "}");
      if (tab_bar->styleSheet() != stylesheet) {
        tab_bar->setStyleSheet(stylesheet);
      }
    } else {
      if (tab_bar->styleSheet() != tab_bar_stylesheet) {
        tab_bar->setStyleSheet(tab_bar_stylesheet);
      }
    }
  }

  QList<DockWidget*> dock_widgets = findChildren<DockWidget*>();
  for (auto dock_widget : dock_widgets) {
    if (mark_active_dock_widget_ && active_dock_widget_ == dock_widget) {
      dock_widget->setStyleSheet(
          QString("%1::title {"
                  "background : palette(highlight);"
                  "color : palette(highlighted-text);"
                  "}")
              .arg(QString(dock_widget->metaObject()->className())
                       .replace(':', '-')));
    } else {
      dock_widget->setStyleSheet("");
    }
  }
}

void MainWindowWithDetachableDockWidgets::updateDocksAndTabs() {
  updateCloseButtonsAndIconsOnTabBars();
  updateDockWidgetTitleBars();
  updateActiveDockWidget();
  layout()->invalidate();
}

void MainWindowWithDetachableDockWidgets::focusChanged(QWidget* /*old*/,
                                                       QWidget* now) {
  if (now != nullptr) {
    emit updateFocus(now);
  }
}

void MainWindowWithDetachableDockWidgets::delayedFocusChanged(
    const QPointer<QWidget>& now) {
  if (now.isNull()) {
    return;
  }

  DockWidget* dock_widget = DockWidget::getParentDockWidget(now);
  if (dock_widget != nullptr && dock_widget != active_dock_widget_) {
    setActiveDockWidget(dock_widget);
  } else {
    auto* tab_bar = dynamic_cast<QTabBar*>(now.data());
    if (tab_bar != nullptr) {
      auto main_window =
          MainWindowWithDetachableDockWidgets::getParentMainWindow(tab_bar);
      if (main_window != nullptr) {
        auto* dock_widget = dynamic_cast<DockWidget*>(
            main_window->tabToDockWidget(tab_bar, tab_bar->currentIndex()));
        if (dock_widget != nullptr) {
          MainWindowWithDetachableDockWidgets::setActiveDockWidget(dock_widget);
        }
      }
    }
  }
}

bool MainWindowWithDetachableDockWidgets::event(QEvent* event) {
  bool has_children = !findChildren<QDockWidget*>().empty();

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
    QMainWindow* main_window, QDockWidget* first, QDockWidget* second,
    Qt::Orientation orientation) {
  // As QMainWindow::splitDockWidget doesn't handle multiple dock widgets
  // tabified together, it's necessary to detach all excessive docks,
  // split dock area and then reattach the docks.
  auto tabified_docks = main_window->tabifiedDockWidgets(first);
  for (auto dock_widget : tabified_docks) {
    main_window->removeDockWidget(dock_widget);
  }
  tabified_docks.removeOne(second);

  second->show();
  main_window->splitDockWidget(first, second, orientation);

  for (auto dock_widget : tabified_docks) {
    dock_widget->show();
    main_window->tabifyDockWidget(first, dock_widget);
  }

  return !main_window->tabifiedDockWidgets(first).contains(second);
}

}  // namespace ui
}  // namespace veles
