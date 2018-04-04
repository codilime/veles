#include <QtWidgets/QtWidgets>
#include "ui/docks/dragger.h"

namespace veles {
namespace ui {

Dragger::Dragger(QString labelText, QIcon icon, QWidget* draggedWidget) :
    QWidget(nullptr, Qt::Window | Qt::FramelessWindowHint),
    tabText(labelText), tabIcon(icon), widget(draggedWidget) {
  grabMouse();
  this -> setLayout(mainLayout);
  label = new QLabel(labelText);
  mainLayout -> addWidget(label);
  label -> show();
  this -> show();
}

void Dragger::mouseMoveEvent(QMouseEvent * event) {
  this -> move(event->globalPos());
}

void Dragger::mouseReleaseEvent(QMouseEvent * event) {
  releaseMouse();
  auto* droppedOverWidget = qApp -> widgetAt(event -> globalX() - 20, event -> globalY() - 20);
  while (droppedOverWidget && qobject_cast<Dock*>(droppedOverWidget) == 0) {
    droppedOverWidget = droppedOverWidget -> parentWidget();
  }
  if (!droppedOverWidget) {
    auto *tabWin = new veles::ui::Dock;
    tabWin->showMaximized();
    tabWin->addWidget(widget, tabIcon, tabText, veles::ui::DropArea::Center);
  } else {
    Dock * droppedOverDock = qobject_cast<Dock *>(droppedOverWidget);
    droppedOverDock -> addWidget(widget, tabIcon, tabText, DropArea::Left);
  }
  this->close();

}

} //ui
} //veles
