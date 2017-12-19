#pragma once

#include <QDockWidget>
#include <QObject>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QtWidgets/QPushButton>

class DisasmWidget : public QDockWidget {
  Q_OBJECT

  QVBoxLayout main_layout_;
  QTextBrowser debug_output_widget_;

 public:
  DisasmWidget();

  void setDebugText(QString text);

  virtual ~DisasmWidget();
};
