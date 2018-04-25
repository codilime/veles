#pragma once

#include <memory>
#include <QtWidgets/QWidget>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QStackedLayout>
#include <QtWidgets/QTabBar>
#include <QtCore/QPointer>
#include <QSplitter>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QLabel>
#include <QMouseEvent>

#include "ui/docks/tabwidget.h"

namespace veles {
namespace ui {


class Dockable {

 Q_OBJECT

 public:
  explicit Dockable(QWidget * widget, QIcon &icon, QString &label);
  explicit Dockable(QWidget * widget, QString &label);

  QWidget * widget;
  QIcon icon;
  QString title;
};

}
} //veles
