#include <iostream>
#include "ui/docks/dragger.h"
#include "ui/docks/dock.h"

namespace veles {
namespace ui {

Dock::Dock(QWidget *parent) : QWidget(parent), state(DockState::Empty) {
  stacked_layout = new QStackedLayout;
  setLayout(stacked_layout);

  tabWidget = new TabWidget(this);
  tabWidget -> setMovable(true);
  splitter = new QSplitter(this);

  tabWidget -> hide();
  splitter -> hide();

  stacked_layout -> addWidget(tabWidget);
  stacked_layout -> addWidget(splitter);

  connect(tabWidget, &TabWidget::emptied, this, [this](){
    if (this -> state == DockState::Consistent)
      this -> setState(DockState::Empty);
  });
}

void Dock::addWidget(QWidget *widget, const QString &label, DropArea area) {
  addWidget(widget, QIcon(), label, area);
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
        Dock * newParent = new Dock();
        std::cout<<"Created new parent: "<<newParent<<std::endl;
        Dock * newDock = new Dock();
        Dock * oldParent = parentDock();
        if (oldParent)
          oldParent -> replaceDock(this, newParent);
        newDock -> addWidget(widget, icon, label, area);
        newParent -> becomeParent(this, newDock, Qt::Vertical);
        newParent -> show();
      }
      break;

    default:
      puts("Nie można tak!\n");
      break;
  }
  printSituation();
}

void Dock::setState(Dock::DockState state) {
  Dock::state = state;
  if (state == DockState::Empty and this -> parent() == nullptr) {
    this->close();
    puts("Tab Emptied\n");
  }
  emit stateChanged(state);
}

void Dock::childDockStateChange(DockState new_state, QPointer<Dock> child) {
  if (new_state == DockState::Empty) {
    std::cout<<"Child dock "<<child.data()<<" of "<<this<<" emptied\n";
    auto sibiling = child == dock1 ? dock2:dock1;
    Dock * parentDo = parentDock();
    if (parentDo) {
      puts("Parent dock exists");
      printSituation();
      parentDo->replaceDock(this, sibiling);
      parentDo->printSituation();
    }
    else {
      puts("NO PARENT");
      printSituation();
      sibiling -> setParent(nullptr);
      sibiling -> show();
      //todo(before merge):
      //sibiling -> move(this -> globalX(), globalY())'
      this -> close();
      sibiling -> printSituation();
    }
  }
}

void Dock::mousePressEvent(QMouseEvent *event) {
  if (tabWidget -> tabBar() -> rect().contains(event -> pos())) {
    dragged_tab_index = tabWidget -> tabBar() -> tabAt(event -> pos());
    if (dragged_tab_index > -1) {
      drag_start = event -> globalPos();
    }
  } else {
    dragged_tab_index = -1;
  }
}

void Dock::mouseMoveEvent(QMouseEvent *event) {
  if (dragged_tab_index > -1) {
    auto covered = event -> globalPos() - drag_start;
    auto indicator = QPoint(std::abs(covered.x()), std::abs(covered.y())) - detach_boundary;
    if (std::max(indicator.x(), indicator.y()) > 0) {
      (void) new Dragger(tabWidget -> tabText(dragged_tab_index), tabWidget -> tabIcon(dragged_tab_index), tabWidget -> widget(dragged_tab_index));
      tabWidget->removeTab(dragged_tab_index);
    }
  }
}

void Dock::showTabs() {
  stacked_layout -> setCurrentIndex(0);
}

void Dock::showSplitter() {
  stacked_layout -> setCurrentIndex(1);
}

void Dock::becomeParent(Dock *dock1, Dock *dock2, Qt::Orientation orientation) {
  state = DockState::Divided;
  this -> dock1 = dock1;
  this  -> dock2 = dock2;
  splitter -> addWidget(dock1);
  splitter -> addWidget(dock2);
  splitter -> setOrientation(orientation);
  splitter -> show();
  showSplitter();
  connect(dock1, &Dock::stateChanged, this, [this](DockState new_state){ this->childDockStateChange(new_state, this->dock1);});
  connect(dock2, &Dock::stateChanged, this, [this](DockState new_state){ this->childDockStateChange(new_state, this->dock2);});
}

void Dock::replaceDock(Dock *replaced, Dock *replacee) {
  // todo(malpunek): Might want to use QSplitter::replaceWidget when Veles will switch to qt >= 5.9

  int index = splitter -> indexOf(replaced);
  splitter -> insertWidget(index, replacee);
  replaced -> setParent(nullptr);
  std::cout<<"Disconnect "<<replaced<<" and "<<this<<std::endl;
  disconnect(replaced, 0, this, 0);

  if (dock1 == replaced)
    dock1 = replacee;
  else
    dock2 = replacee;
  connect(replacee, &Dock::stateChanged, this, [this, replacee](DockState new_state){ this->childDockStateChange(new_state, replacee);});

}

Dock *Dock::parentDock() {
  QWidget * result = parentWidget();
  while (result && qobject_cast<Dock *>(result) == 0)
    result = result -> parentWidget();
  return qobject_cast<Dock *>(result);
}

void Dock::printSituation() {
  Dock * top = this;
  while (top -> parentDock()) top = top -> parentDock();
  std::cout<<"Printing Situation\n\n";
  top -> printSingle(0);
}

void printSpaces(int amount) {
  for (int i = 0; i < amount; i++)
    std::cout<<"  ";
}

void Dock::printSingle(int indent) {
  printSpaces(indent);
  std::cout<<"I am "<<this<<" ";
  if (state == DockState::Empty) {
    std::cout<<"Empty"<<std::endl;
  } else if (state == DockState::Consistent) {
    std::cout<<"I have a widget";
    QLabel * label = qobject_cast<QLabel *>(this -> tabWidget -> widget(0));
    if (label)
      std::cout<<label -> text() . toStdString();
    std::cout<<std::endl;
  } else {
    std::cout<<"I have children"<<std::endl;
    if (dock1.data() != splitter -> widget(0)) {
      printSpaces(indent);
      std::cout << "Alert1! " << dock1.data() << " " << splitter->widget(0)<<std::endl;
      dock1->printSingle(indent + 2);
      if (qobject_cast<Dock *>(splitter->widget(0)))
        qobject_cast<Dock *>(splitter->widget(0)) -> printSingle(indent + 2);
      else {
        printSpaces(indent);
        std::cout << "NONE!" << std::endl;
      }
    } else {
      dock1->printSingle(indent + 2);
    }
    if (dock2.data() != splitter -> widget(1)) {
      printSpaces(indent);
      std::cout<<"Alert2! " <<dock2.data()<<" "<<splitter -> widget(1)<<std::endl;
      dock2 -> printSingle(indent + 2);
      if (qobject_cast<Dock *>(splitter->widget(1)))
        qobject_cast<Dock *>(splitter->widget(1)) -> printSingle(indent + 2);
      else {
        printSpaces(indent);
        std::cout << "NONE!" << std::endl;
      }
    } else {
      dock2->printSingle(indent + 2);
    }
    std::cout<<std::endl;
  }
}

} //ui
} //veles
