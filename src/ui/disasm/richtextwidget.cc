#include "ui/disasm/richtextwidget.h"

#include <QPaintEvent>
#include <QtGui/QPainter>
#include <QtWidgets/QWidget>

namespace veles {
namespace ui {

void RichTextWidget::paintEvent(QPaintEvent* event) {
  QPainter painter(this);
  painter.fillRect(event->rect(), QColor(0, 0, 0));
}
}  // namespace ui
}  // namespace veles
