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
        setState(DockState::Divided);
        auto first_dock_tabs = tabWidget -> tabchildren();
        tabWidget -> clear();

        initDocks();
        for (auto tab : first_dock_tabs) {
          dock1 -> addWidget(std::get<0>(tab), std::get<1>(tab), std::get<2>(tab), DropArea::Center);
        }
        dock2 -> addWidget(widget, icon, label, area);
        splitter -> show();
        stacked_layout -> setCurrentIndex(1);
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
  connect(dock1, &Dock::stateChanged, this, [this](DockState new_state){ this->childDockStateChange(new_state, this->dock1);});
  connect(dock2, &Dock::stateChanged, this, [this](DockState new_state){ this->childDockStateChange(new_state, this->dock2);});
}

void Dock::childDockStateChange(DockState new_state, QPointer<Dock> child) {
  if (new_state == DockState::Empty) {
    auto sibiling = child == dock1 ? dock2:dock1;
    setFromChild(sibiling);
    delete sibiling;
    this -> state = DockState::Consistent;

    puts("Hey someone is empty!\n");
  } else {
    puts("Hey someone is NOT empty!\n");
  }

}

void Dock::setState(Dock::DockState state) {
  Dock::state = state;
  if (state == DockState::Empty and this -> parent() == nullptr) {
    this->close();
    puts("Tab Emptied\n");
  }
  emit stateChanged(state);
}

void Dock::setFromChild(Dock *child) {
  delete stacked_layout;
  stacked_layout = child -> stacked_layout;
  this -> setLayout(stacked_layout);

  delete tabWidget;
  tabWidget = child -> tabWidget;
  connect(tabWidget, &TabWidget::emptied, this, [this](){
    if (this -> state == DockState::Consistent)
      this -> setState(DockState::Empty);
  });

}


void Dock::mousePressEvent(QMouseEvent *event) {
  QWidget::mousePressEvent(event);
  if (tabWidget -> tabBar() -> rect().contains(event -> pos())) {
    puts("Dzień dobry");
    dragged_tab_index = tabWidget -> tabBar() -> tabAt(event -> pos());
    if (dragged_tab_index > -1) {
      auto tab_title = tabWidget -> tabBar() -> tabText(dragged_tab_index);
      dragger = new QLabel(tab_title, this, Qt::Window | Qt::FramelessWindowHint);
      dragger -> hide();
      drag_start = event -> globalPos();
    }
  }
}

void Dock::mouseMoveEvent(QMouseEvent *event) {
  QWidget::mouseMoveEvent(event);
  if (dragger) {
    auto covered = event -> globalPos() - drag_start;
    auto indicator = QPoint(std::abs(covered.x()), std::abs(covered.y())) - detach_boundary;
    if (std::max(indicator.x(), indicator.y()) > 0 and !dragger -> isVisible()) {
      dragger -> show();
      dragged_tab_index = tabWidget -> currentIndex();
      dragged_widget = tabWidget -> widget(dragged_tab_index);
      tabWidget -> removeTab(dragged_tab_index);
    }
    dragger->move(event->globalPos());

  }
}


void Dock::mouseReleaseEvent(QMouseEvent * event) {
    QWidget::mouseReleaseEvent(event);
    if (dragger) {
        if (dragged_widget) {
            auto* tabWin = new veles::ui::Dock;
            tabWin -> showMaximized();
            tabWin -> addWidget(dragged_widget, QIcon(":/images/show_hex_edit.png"), dragger->text(), veles::ui::DropArea::Center);
            dragged_widget->show();
          }
      dragger->setParent(nullptr);
      delete dragger;

    }
  }


} //ui
} //veles
