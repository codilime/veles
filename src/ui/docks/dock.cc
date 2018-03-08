#include "ui/docks/dock.h"

namespace veles {
namespace ui {

Dock::Dock(QWidget *parent) : QWidget(parent), state(DockState::Empty) {
  top_layout = new QVBoxLayout();
  setLayout(top_layout);

  tab_bar = new QTabBar();
  top_layout -> addWidget(tab_bar);
  tab_bar -> hide();

  stacked_layout = new QStackedLayout;
  top_layout -> addLayout(stacked_layout);

  sections_layout = new QBoxLayout(QBoxLayout::LeftToRight);
  top_layout -> addLayout(sections_layout);
}


void Dock::addWidget(QWidget * widget, DropArea area) {
  switch (this -> state) {
    case DockState::Empty:
      state = DockState::Consistent;
      stacked_widgets.push_back(widget);
      stacked_layout -> addWidget(widget);
      break;

    case DockState::Consistent:
      if (area && DropArea::Center) {
        stacked_layout -> addWidget(widget);
        stacked_widgets.push_back(widget);
        updateTabBar(widget);
      } else {
        initDocks();
        for (auto child_widget : stacked_widgets) {
          stacked_layout -> removeWidget(child_widget);
          dock1 -> addWidget(child_widget);
        }
        stacked_widgets.clear();
        dock2 -> addWidget(widget);
        state = DockState::Divided;
      }
      break;

    default:
      puts("Nie można tak!\n");
      break;
  }
}

void Dock::updateTabBar(QWidget *added) {
  // TODO
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
  connect(dock1, &Dock::stateChanged, [this](DockState new_state){this -> dockStateChange(new_state, this -> dock1);});
  connect(dock1, &Dock::stateChanged, [this](DockState new_state){this -> dockStateChange(new_state, this -> dock2);});
}

void Dock::dockStateChange(DockState new_state, Dock *child) {
  if (new_state == DockState::Empty) {
    puts("Hey someone is empty!\n");
  }

}

} //ui
} //veles
