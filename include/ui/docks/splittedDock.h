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

enum DropArea
{
  Invalid = 0,
  Top = 1,
  Right = 2,
  Bottom = 4,
  Left = 8,
  Center = 16,

  AllAreas = Top | Right | Bottom | Left | Center,
  Sides = Right | Left,
  Sth = Top | Bottom,
  NotCenter = Sides | Sth
};

Q_DECLARE_FLAGS(DropAreas, DropArea)


class SplittedDock : public QWidget {

  Q_OBJECT

 public:

  explicit SplittedDock(QWidget *parent = nullptr, Qt::Orientation orientation = Qt::Horizontal);
  QWidget * replaceWidget(QWidget * newWidget, int index);
  void addTabbedDock(QWidget * dock, int index = 0);

 private:
  Qt::Orientation orientation;
  QStackedLayout * stacked_layout;
  QSplitter * splitter;
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