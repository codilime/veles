#pragma once

#include <memory>
#include <QtWidgets/QWidget>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QStackedLayout>
#include <QtWidgets/QTabBar>
#include <QtCore/QPointer>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QTabWidget>
#include "ui/docks/tabwidget.h"

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


class Dock : public QWidget {

  Q_OBJECT

 public:

  explicit Dock(QWidget *parent = nullptr);

  enum DockState {Empty = 0, Consistent = 1, Divided = 2};

  void addWidget(QWidget * widget, const QString& label, DropArea area);
  void addWidget(QWidget * widget, const QIcon& Icon, const QString& label, DropArea area);

 public slots:

  void childDockStateChange(DockState new_state, QPointer<Dock> child);
  void setState(DockState state);

 signals:
  void stateChanged(DockState new_state);

 private:
  DockState state;
  Qt::Orientation orientation;

  QStackedLayout * stacked_layout;
  QSplitter * splitter;
  TabWidget * tabWidget;
  QPointer<Dock> dock1, dock2;

  void initDocks();
  void clearDocks();
  void setFromChild(Dock * dock);

};

}
} //veles