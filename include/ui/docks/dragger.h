#pragma once

#include <memory>
#include <QtWidgets/QWidget>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QStackedLayout>
#include <QtWidgets/QTabBar>
#include <QtCore/QPointer>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QLabel>
#include <QMouseEvent>

#include "ui/docks/tabwidget.h"
#include "ui/docks/dock.h"


namespace veles {
namespace ui {

class Dragger : public QWidget {

 Q_OBJECT

 public:

  explicit Dragger(QString text, QIcon icon, QWidget *draggedWidget);

  void mouseMoveEvent(QMouseEvent * event) override;
  void mouseReleaseEvent(QMouseEvent * event) override;
  void changeEvent(QEvent * event) override;

  QHBoxLayout * mainLayout = new QHBoxLayout();

  QString tabText;
  QIcon tabIcon;
  QWidget* widget;
  QLabel* label;

};

}
} //veles