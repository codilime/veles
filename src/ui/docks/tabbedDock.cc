#include <iostream>
#include "ui/docks/dragger.h"
#include "ui/docks/tabbedDock.h"

namespace veles {
namespace ui {

tabbedDock::tabbedDock(QWidget *parent, Dockable * dockable) {

  tabWidget = new TabWidget(this);
  tabWidget -> setMovable(true);
  tabWidget -> addTab(dockable -> widget, dockable -> icon, dockable -> title);
  
  connect(tabWidget, &TabWidget::emptied, this, [this](){
    // todo Tell parent I'm empty
    this -> deleteLater();
  });
}
void tabbedDock::addDockable(Dockable *to_dock) {
  dockables.push_back(to_dock);
  tabWidget -> addTab(to_dock -> widget, to_dock -> icon, to_dock -> title);
}

void tabbedDock::mousePressEvent(QMouseEvent *event) {
  if (tabWidget -> tabBar() -> rect().contains(event -> pos())) {
    dragged_tab_index = tabWidget -> tabBar() -> tabAt(event -> pos());
    if (dragged_tab_index > -1) {
      drag_start = event -> globalPos();
    }
  } else {
    dragged_tab_index = -1;
  }
}

void tabbedDock::mouseMoveEvent(QMouseEvent *event) {
  if (dragged_tab_index > -1) {
    auto covered = event -> globalPos() - drag_start;
    auto indicator = QPoint(std::abs(covered.x()), std::abs(covered.y())) - detach_boundary;
    if (std::max(indicator.x(), indicator.y()) > 0) {
      // todo remove Dockable
      tabWidget->removeTab(dragged_tab_index);
      (void) new Dragger(this, tabWidget -> tabText(dragged_tab_index), tabWidget -> tabIcon(dragged_tab_index), tabWidget -> widget(dragged_tab_index));
    }
  }
}

int tabbedDock::myIndex() {
  SplittedDock * parent = qobject_cast<SplittedDock *>(this -> parentWidget());
  return parent -> splitter -> indexOf(this);
}

void tabbedDock::split() {
  SplittedDock * parent = qobject_cast<SplittedDock *>(this -> parentWidget());
  SplittedDock * split = new SplittedDock();
  (void) parent -> replaceWidget(split, myIndex());
  split -> addTabbedDock(this, 0);
}
Dockable *tabbedDock::remove(int index) {
  return nullptr;
}

} //ui
} //veles
