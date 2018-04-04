
#include "ui/docks/tabbar.h"

namespace veles {
namespace ui {
void TabBar::mousePressEvent(QMouseEvent *event) {
  QTabBar::mousePressEvent(event);
  event -> ignore();
  //  if (event -> button() == Qt::LeftButton) {
  //    puts("Krzycz trybson\n");
  //  }
}

void TabBar::mouseMoveEvent(QMouseEvent *event) {
  //  QTabBar::mouseMoveEvent(event);
  event -> ignore();
  //  if (event -> button() == Qt::LeftButton) {
  //    puts("Krzycz trybson\n");
  //  }
}
void TabBar::mouseReleaseEvent(QMouseEvent *event) {
  QTabBar::mouseReleaseEvent(event);
  event -> ignore();
}

}}