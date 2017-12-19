/*
 * Copyright 2018 CodiLime
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
#include "ui/filters/tabbareventfilter.h"

#include <QApplication>

#include "ui/dockwidget_native.h"
#include "ui/mainwindowwithdetachabledockwidgets.h"

namespace veles {
namespace ui {

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

}  // namespace ui
}  // namespace veles
