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
#include "ui/dockwidget.h"

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

#include "ui/dockwidget_native.h"
#include "ui/mainwindowwithdetachabledockwidgets.h"
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
    connect(this, &DockWidget::dockLocationChanged, first_main_window,
            &MainWindowWithDetachableDockWidgets::dockLocationChanged);
  }

  maximize_here_action_ = createMoveToNewWindowAndMaximizeAction();
  addAction(maximize_here_action_);
  detach_action_ = createMoveToNewWindowAction();
  addAction(detach_action_);
  createSplitActions();

  connect(this, &DockWidget::customContextMenuRequested, this,
          &DockWidget::displayContextMenu);
  connect(this, &DockWidget::topLevelChanged, this,
          &DockWidget::topLevelChangedNotify, Qt::QueuedConnection);

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
    connect(action, &QAction::triggered, this, &DockWidget::moveToDesktop);
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
    connect(action, &QAction::triggered, this, &DockWidget::moveToWindow);
    menu_move_to_window->addAction(action);
  }

  return menu_move_to_window;
}

QAction* DockWidget::createMoveToNewWindowAction() {
  auto action = ShortcutsModel::getShortcutsModel()->createQAction(
      util::settings::shortcuts::DOCK_MOVE_TO_TOP, this,
      Qt::WidgetWithChildrenShortcut);
  connect(action, &QAction::triggered, this,
          &DockWidget::detachToNewTopLevelWindow);

  return action;
}

QAction* DockWidget::createMoveToNewWindowAndMaximizeAction() {
  auto action = ShortcutsModel::getShortcutsModel()->createQAction(
      util::settings::shortcuts::DOCK_MOVE_TO_TOP_MAX, this,
      QIcon(":/images/maximize.png"), Qt::WidgetWithChildrenShortcut);
  connect(action, &QAction::triggered, this,
          &DockWidget::detachToNewTopLevelWindowAndMaximize);

  return action;
}

void DockWidget::createSplitActions() {
  split_horizontally_action_ =
      ShortcutsModel::getShortcutsModel()->createQAction(
          util::settings::shortcuts::DOCK_SPLIT_HORIZ, this,
          QIcon(":/images/split_horizontally.png"),
          Qt::WidgetWithChildrenShortcut);
  connect(split_horizontally_action_, &QAction::triggered, this,
          &DockWidget::splitHorizontally);
  addAction(split_horizontally_action_);

  split_vertically_action_ = ShortcutsModel::getShortcutsModel()->createQAction(
      util::settings::shortcuts::DOCK_SPLIT_VERT, this,
      QIcon(":/images/split_vertically.png"), Qt::WidgetWithChildrenShortcut);
  connect(split_vertically_action_, &QAction::triggered, this,
          &DockWidget::splitVertically);
  addAction(split_vertically_action_);
}

/*****************************************************************************/
/* IconAwareView */
/*****************************************************************************/

std::map<QString, QIcon*> IconAwareView::icons_;

IconAwareView::IconAwareView(const QString& category, const QString& path) {
  getOrCreateIcon(category, path);
  connect(qApp, &QGuiApplication::lastWindowClosed,
          &IconAwareView::deleteIcons);
  setFocusPolicy(Qt::StrongFocus);
}

IconAwareView::~IconAwareView() {}

void IconAwareView::getOrCreateIcon(const QString& category,
                                    const QString& icon_path) {
  QIcon* icon = nullptr;
  auto iter = icons_.find(category);
  if (iter == icons_.end()) {
    icons_[category] = icon = new QIcon(icon_path);
  } else {
    icon = iter->second;
  }
  setWindowIcon(*icon);
}

void IconAwareView::deleteIcons() {
  for (auto icon : icons_) {
    delete icon.second;
  }
  icons_.clear();
}

}  // namespace ui
}  // namespace veles
