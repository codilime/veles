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
#include <QFile>
#include <QMutexLocker>

#include "ui/logwidget.h"

namespace veles {
namespace ui {

/*****************************************************************************/
/* IODeviceProxy */
/*****************************************************************************/

QMutex* IODeviceProxy::mutexHistory() { return &mutex_; }

QList<QString>& IODeviceProxy::history() { return history_; }

qint64 IODeviceProxy::readData(char* /*data*/, qint64 /*maxSize*/) { return 0; }

qint64 IODeviceProxy::writeData(const char* data, qint64 maxSize) {
  QString message = QString::fromUtf8(data, maxSize);

  {
    QMutexLocker locker(&mutex_);
    history_.push_back(message);
    if (history_.size() > max_history_size_) {
      history_.pop_front();
    }
  }

  emit newString(message);
  return maxSize;
}

/*****************************************************************************/
/* LogWidget */
/*****************************************************************************/

IODeviceProxy* LogWidget::io_proxy_ = nullptr;

LogWidget::LogWidget(QWidget* parent) : QMainWindow(parent) {
  setWindowFlags(Qt::WindowFlags(windowFlags() & ~Qt::Window));
  setWindowTitle("Log");
  ui_ = new Ui::LogWidget;
  ui_->setupUi(this);
  connect(ui_->action_clear_, SIGNAL(triggered()), this, SLOT(clearLog()));

  checkIODevice();
  connect(io_proxy_, SIGNAL(newString(QString)), this, SLOT(append(QString)),
          Qt::QueuedConnection);

  setupSaveFileDialog();
  appendHistory();
}

LogWidget::~LogWidget() { delete ui_; }

QIODevice* LogWidget::output() {
  checkIODevice();
  return io_proxy_;
}

void LogWidget::clearLog() { ui_->text_edit_->clear(); }

void LogWidget::append(QString text) {
  if (text.endsWith("\n")) {
    text.chop(1);
  }
  ui_->text_edit_->appendPlainText(text);
}

void LogWidget::saveFileSelected(const QString& file) {
  QFile save_file(file);
  QTextStream out(output());

  if (save_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QByteArray data = ui_->text_edit_->toPlainText().toUtf8();
    qint64 result = save_file.write(data);
    save_file.close();

    if (result == data.size()) {
      out << tr("DONE: Log messages successfully saved to a file.") << endl;
    } else {
      out << tr("ERROR: File %1 successfully opened but write attempt failed.")
                 .arg(file)
          << endl;
    }
  } else {
    out << tr("ERROR: Could not open a file: %1.").arg(file) << endl;
  }
}

void LogWidget::appendHistory() {
  QMutexLocker lock(io_proxy_->mutexHistory());
  for (const auto& str : io_proxy_->history()) {
    append(str);
  }
}

void LogWidget::setupSaveFileDialog() {
  file_dialog_ = new QFileDialog(this);
  file_dialog_->setFileMode(QFileDialog::AnyFile);
  file_dialog_->setAcceptMode(QFileDialog::AcceptSave);
  file_dialog_->setWindowTitle("Save log to a file");
  connect(ui_->action_save_, SIGNAL(triggered()), file_dialog_, SLOT(show()));
  connect(file_dialog_, SIGNAL(fileSelected(const QString&)), this,
          SLOT(saveFileSelected(const QString&)));
}

void LogWidget::checkIODevice() {
  if (io_proxy_ == nullptr) {
    io_proxy_ = new IODeviceProxy;
    io_proxy_->open(QIODevice::WriteOnly);
    io_proxy_->moveToThread(QApplication::instance()->thread());
  }
}

}  // namespace ui
}  // namespace veles
