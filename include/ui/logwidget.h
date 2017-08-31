/*
 * Copyright 2016 CodiLime
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#pragma once

#include <QFileDialog>
#include <QIODevice>
#include <QList>
#include <QMainWindow>
#include <QMutex>
#include <QTextStream>

#include "ui_logwidget.h"

namespace veles {
namespace ui {

/*****************************************************************************/
/* IODeviceProxy */
/*****************************************************************************/

class IODeviceProxy : public QIODevice {
  Q_OBJECT
 public:
  QMutex* mutexHistory();
  QList<QString>& history();

 protected:
  qint64 readData(char* data, qint64 maxSize) Q_DECL_OVERRIDE;
  qint64 writeData(const char* data, qint64 maxSize) Q_DECL_OVERRIDE;

 signals:
  void newString(QString str);

 private:
  QList<QString> history_;
  static constexpr int max_history_size_ = 100;

  // Controls access to the history_.
  QMutex mutex_;
};

/*****************************************************************************/
/* LogWidget */
/*****************************************************************************/

// Thread-safe usage:
// QTextStream out(veles::ui::LogWidget::output());
// out << "Log message." << endl;
//
// LogWidget::output() always returns non-null pointer and it's value is not
// going to change through application's lifetime (QTextStream using
// output()'s value can be created once and safely kept).

class LogWidget : public QMainWindow {
  Q_OBJECT

 public:
  explicit LogWidget(QWidget* parent = nullptr);
  ~LogWidget() override;
  static QIODevice* output();

 public slots:
  void clearLog();
  void append(QString text);
  void saveFileSelected(const QString& file);

 private:
  void appendHistory();
  void setupSaveFileDialog();
  static void checkIODevice();

  Ui::LogWidget* ui_;
  QFileDialog* file_dialog_;

  static IODeviceProxy* io_proxy_;
};

}  // namespace ui
}  // namespace veles
