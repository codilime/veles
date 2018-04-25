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
#include "dockable.h"

namespace veles {
namespace ui {

class tabbedDock : public QWidget {

  Q_OBJECT

 public:

  explicit tabbedDock(QWidget *parent = nullptr, Dockable * dockable);
  void addDockable(Dockable * to_dock);
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *) override;
  void split();
  void showOverlay();
  int myIndex();
  Dockable * remove(int index);

  QList<Dockable *> dockables;
  TabWidget * tabWidget;
  int dragged_tab_index = -1;
  QPoint drag_start;
  const QPoint detach_boundary = QPoint(50, 50);
};

}
} //veles


/*
 *
 * Problem:
 *  - nie ma type safety na Dockach -> Docki czy mają 0,1,2 elementy są dalej tym samym typem. Ciężko się tym zarządza
 *
 *  Rozwiązanie: zrobić różne klasy do docków które mają jeden lub 0 elementów i dla już podzielonych docków.
 *
 *
 *
 */