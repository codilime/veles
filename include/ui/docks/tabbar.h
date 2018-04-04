#pragma once

#include <QtWidgets/QTabBar>
#include <QMouseEvent>

namespace veles {
namespace ui {

class TabBar : public QTabBar {

  Q_OBJECT

  // customizable TabBar for enabling tab dragging and (un)docking

  void mousePressEvent(QMouseEvent *event);

  void mouseMoveEvent(QMouseEvent *event);

  void mouseReleaseEvent(QMouseEvent *event);


};

}}