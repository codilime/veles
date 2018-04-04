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
  auto* tabWin = new veles::ui::Dock;
  tabWin -> showMaximized();
  tabWin -> addWidget(widget, tabIcon, tabText, veles::ui::DropArea::Center);
  this -> close();
}

} //ui
} //veles
