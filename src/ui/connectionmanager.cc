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
#include <QTimer>

#include "ui/logwidget.h"
#include "ui/connectiondialog.h"
#include "ui/connectionmanager.h"
#include "util/version.h"

namespace veles {
namespace ui {

/*****************************************************************************/
/* ConnectionManager */
/*****************************************************************************/

ConnectionManager::ConnectionManager(QWidget* parent)
    : QObject(parent), server_process_(nullptr),
    network_client_output_(nullptr) {
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

  network_client_ = new client::NetworkClient(this);
  network_client_output_ = new QTextStream(LogWidget::output());
  network_client_->setOutput(network_client_output_);

  connect(network_client_, &client::NetworkClient::connectionStatusChanged,
      this, &ConnectionManager::updateConnectionStatus);
}

ConnectionManager::~ConnectionManager() {
  if (server_process_ && shut_down_server_on_close_) {
    killLocalServer();
  }

  connection_dialog_->deleteLater();
  delete network_client_output_;
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
  serverProcessReadyRead();

  QTextStream out(LogWidget::output());
  out << "Process of locally created server finished. Exit code: "
      << exit_code << "." << endl;
  kill_locally_created_server_action_->setEnabled(false);
  server_process_->deleteLater();
  server_process_ = nullptr;
}

void ConnectionManager::connectionDialogAccepted() {
  if(connection_dialog_->runANewServer()) {
    shut_down_server_on_close_ = connection_dialog_->shutDownServerOnClose();
    startLocalServer();
    QTextStream out(LogWidget::output());
    out << "Waiting for a new server to start..." << endl;
    QTimer::singleShot(2000, this, &ConnectionManager::startClient);
  } else {
    startClient();
  }
}

void ConnectionManager::startClient() {
  network_client_->connect(
      connection_dialog_->serverHost(),
      connection_dialog_->serverPort(),
      connection_dialog_->clientInterface(),
      connection_dialog_->clientName(),
      veles::util::version::string,
      "Veles UI",
      "Veles UI",
      QByteArray::fromHex(connection_dialog_->authenticationKey().toUtf8()));
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
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("PYTHONUNBUFFERED", "1");
  env.insert("PYTHONIOENCODING", "UTF-8");
  server_process_->setProcessEnvironment(env);

  QStringList arguments;
  arguments
      << server_file_name
      << connection_dialog_->databaseFile()
      << QString("%1:%2").arg(connection_dialog_->serverHost())
      .arg(connection_dialog_->serverPort())
      << connection_dialog_->authenticationKey();

#if defined(Q_OS_LINUX)
  QString python_interpreter_executable("python3");
#elif defined(Q_OS_WIN)
  QString python_interpreter_executable("py.exe");
#else
  QString python_interpreter_executable("python");
#endif

  QTextStream out(veles::ui::LogWidget::output());
  out
      << "Trying to start a new server..." << endl
      << "    working directory: " << directory_path << endl
      << "    python script name: " << server_file_name << endl
      << "    python interpreter executable: "
      << python_interpreter_executable << endl
      << "    arguments:" << endl;
  for (const auto& arg : arguments) {
    out << "        " << arg << endl;
  }
  out << endl;

  server_process_->start(python_interpreter_executable, arguments,
      QIODevice::ReadWrite | QIODevice::Text);
}

void ConnectionManager::killLocalServer() {
  if(server_process_) {
#ifdef Q_OS_WIN
    server_process_->kill();
#else
    server_process_->terminate();
#endif
  }
}

void ConnectionManager::disconnect() {
  if (network_client_) {
    network_client_->disconnect();
  }
}

void ConnectionManager::serverProcessReadyRead() {
  char buf[10240];
  if(server_process_ && server_process_->bytesAvailable() > 0) {
    qint64 len = server_process_->read(buf, sizeof(buf));
    if(len > 0) {
      LogWidget::output()->write(buf, len);
    }
  }
}

void ConnectionManager::updateConnectionStatus(
      client::NetworkClient::ConnectionStatus connection_status) {
  emit connectionStatusChanged(connection_status);
  disconnect_action_->setEnabled(
      connection_status != client::NetworkClient::ConnectionStatus::NotConnected);
}

void ConnectionManager::raiseConnectionDialog() {
  connection_dialog_->raise();
  connection_dialog_->activateWindow();
}

/*****************************************************************************/
/* ConnectionNotificationWidget */
/*****************************************************************************/

ConnectionNotificationWidget::ConnectionNotificationWidget(QWidget* parent)
    : QWidget(parent),
      connection_status_(client::NetworkClient::ConnectionStatus::NotConnected),
    frame_(0), last_status_change_(-10) {
  ui_ = new Ui::ConnectionNotificationWidget;
  ui_->setupUi(this);

  icon_connected_ = QPixmap(":/images/connection_connected.png");
  icon_not_connected_ = QPixmap(":/images/connection_not_connected.png");
  icon_alarm_ = QPixmap(":/images/connection_alarm.png");

  updateConnectionStatus(connection_status_);
  startTimer(500);
}

ConnectionNotificationWidget::~ConnectionNotificationWidget() {}

void ConnectionNotificationWidget::updateConnectionStatus(
    client::NetworkClient::ConnectionStatus connection_status) {
  if (connection_status != connection_status_) {
    last_status_change_ = frame_;
  }

  switch (connection_status) {
  case client::NetworkClient::ConnectionStatus::NotConnected:
    ui_->connection_status_text_label->setText("Not connected");
    ui_->connection_status_icon_label->setPixmap(icon_not_connected_);
    break;
  case client::NetworkClient::ConnectionStatus::Connecting:
    ui_->connection_status_text_label->setText("Connecting");
    ui_->connection_status_icon_label->setPixmap(icon_not_connected_);
    break;
  case client::NetworkClient::ConnectionStatus::Connected:
    ui_->connection_status_text_label->setText("Connected");
    ui_->connection_status_icon_label->setPixmap(icon_connected_);
    break;
  }

  connection_status_ = connection_status;

}

void ConnectionNotificationWidget::timerEvent(QTimerEvent* event) {
  ++frame_;

  if (connection_status_ == client::NetworkClient::ConnectionStatus::Connecting) {
    ui_->connection_status_icon_label->setPixmap(
        frame_ % 2 ? icon_connected_ : icon_not_connected_);
  } else if (connection_status_
      == client::NetworkClient::ConnectionStatus::NotConnected) {
    if(frame_ - last_status_change_ < 10) {
      ui_->connection_status_icon_label->setPixmap(
          frame_ % 2 ? icon_alarm_ : icon_not_connected_);
    } else {
      ui_->connection_status_icon_label->setPixmap(
          icon_not_connected_);
    }
  }
}

}  // namespace ui
}  // namespace veles
