#pragma once

#include <QDockWidget>
#include <QObject>
#include <QPushButton>

class DisasmWidget : public QDockWidget {
  Q_OBJECT

  QPushButton tmp;

 public:
  DisasmWidget();

  virtual void paintEvent(QPaintEvent* event);
  virtual ~DisasmWidget();
};
