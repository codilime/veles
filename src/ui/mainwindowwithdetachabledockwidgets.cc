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
#include "ui/mainwindowwithdetachabledockwidgets.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QLayout>
#include <QTabBar>

#include "ui/filters/activatedockeventfilter.h"
#include "ui/nodewidget.h"
#include "visualization/panel.h"

namespace veles {
namespace ui {

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
  connect(this, &MainWindowWithDetachableDockWidgets::childAdded, this,
          &MainWindowWithDetachableDockWidgets::childAddedNotify,
          Qt::QueuedConnection);
  connect(this, &MainWindowWithDetachableDockWidgets::childRemoved, this,
          &MainWindowWithDetachableDockWidgets::updateDocksAndTabs,
          Qt::QueuedConnection);

  rubber_band_ = new QRubberBand(QRubberBand::Rectangle, this);
}

MainWindowWithDetachableDockWidgets::~MainWindowWithDetachableDockWidgets() {
  main_windows_.erase(this);
  if (this == first_main_window_) {
    first_main_window_ = nullptr;
  }
}

DockWidget* MainWindowWithDetachableDockWidgets::wrapWithDock(
    QWidget* widget, const QString& title) {
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
  return dock_widget;
}

DockWidget* MainWindowWithDetachableDockWidgets::addTab(QWidget* widget,
                                                        const QString& title,
                                                        DockWidget* sibling) {
  auto* dock_widget = wrapWithDock(widget, title);

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
    connect(tab_bar, &QTabBar::tabCloseRequested, this,
            &MainWindowWithDetachableDockWidgets::tabCloseRequested,
            Qt::UniqueConnection);

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

void MainWindowWithDetachableDockWidgets::createHexEditTab(
    const QSharedPointer<FileBlobModel>& data_model) {
  QSharedPointer<QItemSelectionModel> selection_model(
      new QItemSelectionModel(data_model.data()));

  auto* node_widget = new NodeWidget(this, data_model, selection_model);
  addTab(node_widget, data_model->path().join(" : "), nullptr);
}

void MainWindowWithDetachableDockWidgets::createHexEditTab(
    const QString& fileName, const dbif::ObjectHandle& fileBlob) {
  QSharedPointer<FileBlobModel> data_model(
      new FileBlobModel(fileBlob, {QFileInfo(fileName).fileName()}));
  createHexEditTab(data_model);
}

void MainWindowWithDetachableDockWidgets::createVisualization(
    const QSharedPointer<FileBlobModel>& data_model) {
  auto* panel = new visualization::VisualizationPanel(this, data_model);

  panel->setData(
      QByteArray(reinterpret_cast<const char*>(data_model->binData().rawData()),
                 static_cast<int>(data_model->binData().size())));
  panel->setAttribute(Qt::WA_DeleteOnClose);

  // FIXME: main_window_ needs to be updated when docks are moved around,
  // then we can use this behaviour without any weird effects
  // auto sibling = DockWidget::getParentDockWidget(this);

  auto dock_widget = addTab(panel, data_model->path().join(" : "), nullptr);
  connect(dock_widget, &DockWidget::visibilityChanged, panel,
          &visualization::VisualizationPanel::visibilityChanged);
}

}  // namespace ui
}  // namespace veles
