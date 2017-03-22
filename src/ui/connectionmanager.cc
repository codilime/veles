/*
 * Copyright 2017 CodiLime
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
#include <QAction>
#include <QFileDialog>

#include "ui/logwidget.h"
#include "ui/connectiondialog.h"
#include "ui/connectionmanager.h"

namespace veles {
namespace ui {

/*****************************************************************************/
/* ConnectionManager */
/*****************************************************************************/

ConnectionManager::ConnectionManager(QWidget* parent)
    : QObject(parent), server_process_(nullptr) {
  connection_dialog_ = new ConnectionDialog(parent);
  show_connection_dialog_action_ = new QAction("Connect...", this);
  disconnect_action_ = new QAction("Disconnect", this);
  kill_locally_created_server_action_ = new QAction(
      "Kill locally created server", this);
  kill_locally_created_server_action_->setEnabled(false);
  disconnect_action_->setEnabled(false);

  connect(show_connection_dialog_action_, &QAction::triggered,
      connection_dialog_, &QDialog::show);
  connect(connection_dialog_, &QDialog::accepted,
      this, &ConnectionManager::connectionDialogAccepted);
  connect(kill_locally_created_server_action_, &QAction::triggered,
      this, &ConnectionManager::killLocalServer);
  connect(disconnect_action_, &QAction::triggered,
        this, &ConnectionManager::disconnect);
}

ConnectionManager::~ConnectionManager() {
  connection_dialog_->deleteLater();
}

QAction* ConnectionManager::showConnectionDialogAction() {
  return show_connection_dialog_action_;
}

QAction* ConnectionManager::disconnectAction() {
  return disconnect_action_;
}

QAction* ConnectionManager::killLocallyCreatedServerAction() {
  return kill_locally_created_server_action_;
}

void ConnectionManager::locallyCreatedServerStarted() {
  QTextStream out(veles::ui::LogWidget::output());
  out << "Process of locally created server started." << endl;
  kill_locally_created_server_action_->setEnabled(true);
}

void ConnectionManager::locallyCreatedServerFinished(int exit_code,
    QProcess::ExitStatus exit_status) {
  QTextStream out(veles::ui::LogWidget::output());
  out << "Process of locally created server finished." << endl;
  kill_locally_created_server_action_->setEnabled(false);
  server_process_->deleteLater();
  server_process_ = nullptr;
}

void ConnectionManager::connectionDialogAccepted() {
  if(connection_dialog_->runANewServer()) {
    startLocalServer();
  }

  // TODO actually connect with the server
  // Can be done once C++ client library is created.
}

void ConnectionManager::startLocalServer() {
  if(server_process_) {
    server_process_->deleteLater();
  }

  server_process_ = new QProcess(this);
  server_process_->setProcessChannelMode(QProcess::MergedChannels);
  connect(server_process_, &QProcess::started,
      this, &ConnectionManager::locallyCreatedServerStarted);
  connect(server_process_, static_cast<void(QProcess::*)
      (int, QProcess::ExitStatus)>(&QProcess::finished),
      this, &ConnectionManager::locallyCreatedServerFinished);
  connect(server_process_, &QIODevice::readyRead,
        this, &ConnectionManager::serverProcessReadyRead);

  QString server_file_path = connection_dialog_->serverScript();
  QFileInfo server_file(server_file_path);
  QString directory_path = server_file.absoluteDir().path();
  QString server_file_name = server_file.fileName();

  server_process_->setWorkingDirectory(directory_path);
  QStringList arguments;
  arguments
      << server_file_name
      << connection_dialog_->databaseFile()
      << QString("%1:%2").arg(connection_dialog_->serverHost())
      .arg(connection_dialog_->serverPort());

  QTextStream out(veles::ui::LogWidget::output());
  out
      << "Trying to start a new server..." << endl
      << "    working directory: " << directory_path << endl
      << "    python script name: " << server_file_name << endl
      << "    arguments:" << endl;
  for (auto arg : arguments) {
    out << "        " << arg << endl;
  }
  out << endl;

  server_process_->start("python3", arguments);
}

void ConnectionManager::killLocalServer() {
  if(server_process_) {
    server_process_->terminate();
  }
}

void ConnectionManager::disconnect() {
  // TODO disconnect from the server
  // Can be done once C++ client library is created.
}

void ConnectionManager::serverProcessReadyRead() {
  char buf[1024];
  if(server_process_) {
    qint64 len = server_process_->read(buf, sizeof(buf));
    if(len > 0) {
      LogWidget::output()->write(buf, len);
    }
  }
}

/*****************************************************************************/
/* ConnectionNotificationWidget */
/*****************************************************************************/

ConnectionNotificationWidget::ConnectionNotificationWidget(QWidget* parent)
    : QLabel(parent) {
  setText("Not connected");
}

ConnectionNotificationWidget::~ConnectionNotificationWidget() {}

void ConnectionNotificationWidget::updateConnectionState(
    ConnectionManager::ConnectionState connection_state) {
  switch (connection_state) {
  case ConnectionManager::ConnectionState::NotConnected:
    setText("Not connected");
    break;
  case ConnectionManager::ConnectionState::Connecting:
    setText("Connecting");
    break;
  case ConnectionManager::ConnectionState::Connected:
    setText("Connected");
    break;
  }
}

}  // namespace ui
}  // namespace veles
