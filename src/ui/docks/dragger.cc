#include <QtWidgets/QtWidgets>
#include "ui/docks/dragger.h"

namespace veles {
namespace ui {

Dragger::Dragger(Dock * origin, QString labelText, QIcon icon, QWidget* draggedWidget) :
    QWidget(nullptr, Qt::Window | Qt::FramelessWindowHint /*| Qt::X11BypassWindowManagerHint  |  Qt::WindowStaysOnTopHint | Qt::Tool*/),
    tabText(labelText), tabIcon(icon), widget(draggedWidget) {
  origin -> releaseMouse();
  if (origin -> state == Dock::DockState::Consistent and origin -> tabWidget -> count() == 1)
    origin -> hide;
  this -> origin = origin;
  this -> setLayout(mainLayout);
  label = new QLabel(labelText);
  mainLayout -> addWidget(label);
//  this -> setWindowModality(Qt::ApplicationModal);
  label -> show();
  this -> show();
  grabMouse();
}

void Dragger::mouseMoveEvent(QMouseEvent * event) {
  this -> move(event->globalPos());
//  if (event -> globalX() % 10 == 0)
//    this -> activateWindow();
}

void Dragger::mouseReleaseEvent(QMouseEvent * event) {
  releaseMouse();
  auto* droppedOverWidget = qApp -> widgetAt(event -> globalX() - 20, event -> globalY() - 20);
//  auto windows = qApp -> allWindows();
//  for (auto window: windows) {
//    if (window -> mapToGlobal(0,0))
//
//  }
//      -> widgetAt(event -> globalX() - 20, event -> globalY() - 20);
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
void Dragger::changeEvent(QEvent *event) {
  if (event -> type() == QEvent::ActivationChange) {
    puts("Activation change");
  } else if (event -> type() == QEvent::MouseTrackingChange) {
    puts("Mouse Tracking change");
  }
}

} //ui
} //veles
