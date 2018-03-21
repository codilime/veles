#include "ui/docks/dock.h"

namespace veles {
namespace ui {

Dock::Dock(QWidget *parent) : QWidget(parent), state(DockState::Empty) {
  stacked_layout = new QStackedLayout;
  setLayout(stacked_layout);

  tabWidget = new TabWidget(this);
  splitter = new QSplitter(this);

  tabWidget -> hide();
  splitter -> hide();

  stacked_layout -> addWidget(tabWidget);
  stacked_layout -> addWidget(splitter);
}


void Dock::addWidget(QWidget * widget, const QIcon &icon, const QString &label, DropArea area) {
  switch (this -> state) {
    case DockState::Empty:
      setState(DockState::Consistent);
      tabWidget -> addTab(widget, icon, label);
      stacked_layout -> setCurrentIndex(0);
      tabWidget -> show();
      break;

    case DockState::Consistent:
      if (area & DropArea::Center) {
        tabWidget -> addTab(widget, icon, label);
      } else {
        auto first_dock_tabs = tabWidget -> tabchildren();
        tabWidget -> clear();

        initDocks();
        for (auto tab : first_dock_tabs) {
          dock1 -> addWidget(std::get<0>(tab), std::get<1>(tab), std::get<2>(tab), DropArea::Center);
        }
        dock2 -> addWidget(widget, icon, label, area);
        splitter -> show();
        stacked_layout -> setCurrentIndex(1);
        setState(DockState::Divided);
      }
      break;

    default:
      puts("Nie można tak!\n");
      break;
  }
}

void Dock::addWidget(QWidget *widget, const QString &label, DropArea area) {
  addWidget(widget, QIcon(), label, area);
}

void Dock::clearDocks() {
  if (dock1)
    delete dock1;
  if (dock2)
    delete dock2;
}

void Dock::initDocks() {
  clearDocks();
  dock1 = new Dock;
  dock2 = new Dock;
  splitter -> addWidget(dock1);
  splitter -> addWidget(dock2);
  connect(dock1, &Dock::stateChanged, [this](DockState new_state){this -> dockStateChange(new_state, this -> dock1);});
  connect(dock1, &Dock::stateChanged, [this](DockState new_state){this -> dockStateChange(new_state, this -> dock2);});
}
void Dock::dockStateChange(DockState new_state, Dock *child) {
  if (new_state == DockState::Empty) {
    puts("Hey someone is empty!\n");
  } else {
    puts("Hey someone is NOT empty!\n");
  }

}

void Dock::setState(Dock::DockState state) {
  Dock::state = state;
  emit stateChanged(state);
}

} //ui
} //veles
