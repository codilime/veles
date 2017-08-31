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

#include "ui/connectiondialog.h"
#include "ui/connectionmanager.h"
#include "ui/logwidget.h"
#include "util/settings/connection_client.h"
#include "util/settings/shortcuts.h"
#include "util/version.h"

namespace veles {
namespace ui {

using util::settings::shortcuts::ShortcutsModel;

/*****************************************************************************/
/* ConnectionManager */
/*****************************************************************************/

ConnectionManager::ConnectionManager(QWidget* parent) : QObject(parent) {
  connection_dialog_ = new ConnectionDialog(parent);
  show_connection_dialog_action_ =
      ShortcutsModel::getShortcutsModel()->createQAction(
          util::settings::shortcuts::SHOW_CONNECT_DIALOG, this,
          Qt::ApplicationShortcut);
  disconnect_action_ = ShortcutsModel::getShortcutsModel()->createQAction(
      util::settings::shortcuts::DISCONNECT_FROM_SERVER, this,
      Qt::ApplicationShortcut);
  kill_locally_created_server_action_ =
      ShortcutsModel::getShortcutsModel()->createQAction(
          util::settings::shortcuts::KILL_LOCAL_SERVER, this,
          Qt::ApplicationShortcut);
  kill_locally_created_server_action_->setEnabled(false);
  disconnect_action_->setEnabled(false);

  connect(show_connection_dialog_action_, &QAction::triggered,
          connection_dialog_, &QDialog::show);
  connect(connection_dialog_, &QDialog::accepted, this,
          &ConnectionManager::connectionDialogAccepted);
  connect(kill_locally_created_server_action_, &QAction::triggered, this,
          &ConnectionManager::killLocalServer);
  connect(disconnect_action_, &QAction::triggered, this,
          &ConnectionManager::disconnect);

  network_client_ = new client::NetworkClient(this);
  network_client_output_ = new QTextStream(LogWidget::output());
  network_client_output_->setCodec("UTF-8");
  network_client_->setOutput(network_client_output_);

  connect(network_client_, &client::NetworkClient::connectionStatusChanged,
          this, &ConnectionManager::updateConnectionStatus);
  connect(network_client_, &client::NetworkClient::messageReceived, this,
          &ConnectionManager::messageReceived);
}

ConnectionManager::~ConnectionManager() {
  if (server_process_ != nullptr) {
    killLocalServer();
  }

  connection_dialog_->deleteLater();
  delete network_client_output_;
}

client::NetworkClient* ConnectionManager::networkClient() {
  return network_client_;
}

QAction* ConnectionManager::showConnectionDialogAction() {
  return show_connection_dialog_action_;
}

QAction* ConnectionManager::disconnectAction() { return disconnect_action_; }

QAction* ConnectionManager::killLocallyCreatedServerAction() {
  return kill_locally_created_server_action_;
}

void ConnectionManager::locallyCreatedServerStarted() {
  QTextStream out(veles::ui::LogWidget::output());
  out << "Process of locally created server started." << endl;
  kill_locally_created_server_action_->setEnabled(true);
}

void ConnectionManager::locallyCreatedServerFinished(
    int exit_code, QProcess::ExitStatus /*exit_status*/) {
  serverProcessReadyRead();

  QTextStream out(LogWidget::output());
  out << "Process of locally created server finished. Exit code: " << exit_code
      << "." << endl;
  kill_locally_created_server_action_->setEnabled(false);
  server_process_->deleteLater();
  server_process_ = nullptr;
}

void ConnectionManager::connectionDialogAccepted() {
  is_local_server_ = connection_dialog_->runANewServer();
  if (is_local_server_) {
    startLocalServer();
    QTextStream out(LogWidget::output());
    out << "Waiting for a new server to start..." << endl;
  } else {
    startClient();
  }
}

void ConnectionManager::startClient() {
  network_client_->connect(
      connection_dialog_->serverUrl(), connection_dialog_->clientInterface(),
      connection_dialog_->clientName(), veles::util::version::string,
      "Veles UI", "Veles UI", is_local_server_);
}

void ConnectionManager::startLocalServer() {
  if (server_process_ != nullptr) {
    server_process_->deleteLater();
  }
  server_process_ = new QProcess(this);
  server_process_->setProcessChannelMode(QProcess::MergedChannels);
  connect(server_process_, &QProcess::started, this,
          &ConnectionManager::locallyCreatedServerStarted);
  connect(server_process_,
          static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(
              &QProcess::finished),
          this, &ConnectionManager::locallyCreatedServerFinished);
  connect(server_process_, &QIODevice::readyRead, this,
          &ConnectionManager::serverProcessReadyRead);

#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
  connect(server_process_, &QProcess::errorOccurred, this,
          &ConnectionManager::serverProcessErrorOccurred);
#else
  connect(server_process_, SIGNAL(error(QProcess::ProcessError)), this,
          SLOT(serverProcessErrorOccurred(QProcess::ProcessError)));
#endif

  QString server_file_path = connection_dialog_->serverScript();
  QFileInfo server_file(server_file_path);
  QString directory_path = server_file.absoluteDir().path();
  QString server_file_name = server_file.fileName();

  server_process_->setWorkingDirectory(directory_path);
  QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
  env.insert("PYTHONUNBUFFERED", "1");
  env.insert("PYTHONIOENCODING", "UTF-8");
  server_process_->setProcessEnvironment(env);

  QString scheme;

  if (connection_dialog_->sslEnabled()) {
    scheme = client::SCHEME_SSL;
  } else {
    scheme = client::SCHEME_TCP;
  }

  QStringList arguments;
  arguments << server_file_name << "--cert-dir"
            << connection_dialog_->certificateDir()
            << QString("%1://%2@%3:%4")
                   .arg(scheme)
                   .arg(connection_dialog_->authenticationKey())
                   .arg(connection_dialog_->serverHost())
                   .arg(connection_dialog_->serverPort())
            << connection_dialog_->databaseFile();

#if defined(Q_OS_LINUX)
  QString python_interpreter_executable(
      "/usr/share/veles-server/venv/bin/python3");
  QFileInfo check_file(python_interpreter_executable);
  if (!check_file.exists()) {
    python_interpreter_executable = QString("python3");
  }
#elif defined(Q_OS_WIN)
  QString python_interpreter_executable(qApp->applicationDirPath() +
                                        "/../veles-server/python/python.exe");
  QFileInfo check_file(python_interpreter_executable);
  if (!check_file.exists()) {
    python_interpreter_executable = QString("py.exe");
  }
#elif defined(Q_OS_MAC)
  QString python_interpreter_executable(
      qApp->applicationDirPath() +
      "/../Resources/veles-server/venv/bin/python3");
  QFileInfo check_file(python_interpreter_executable);
  if (!check_file.exists()) {
    python_interpreter_executable = QString("python3");
  }
#else
#warning \
    "This OS is not officially supported, you may need to set this string manually."
  python_interpreter_executable = QString("python3");
#endif

  QTextStream out(veles::ui::LogWidget::output());
  out << "Trying to start a new server..." << endl
      << "    working directory: " << directory_path << endl
      << "    python script name: " << server_file_name << endl
      << "    python interpreter executable: " << python_interpreter_executable
      << endl
      << "    arguments:" << endl;
  for (const auto& arg : arguments) {
    out << "        " << arg << endl;
  }
  out << endl;

  server_process_->start(python_interpreter_executable, arguments,
                         QIODevice::ReadWrite | QIODevice::Text);
}

void ConnectionManager::killLocalServer() {
  if (server_process_ != nullptr) {
#ifdef Q_OS_WIN
    server_process_->kill();
#else
    server_process_->terminate();
#endif
  }
}

void ConnectionManager::disconnect() {
  if (network_client_ != nullptr) {
    network_client_->disconnect();
  }
}

void ConnectionManager::serverProcessReadyRead() {
  if (server_process_ != nullptr && server_process_->bytesAvailable() > 0) {
    QByteArray out = server_process_->read(server_process_->bytesAvailable());
    if (out.size() > 0) {
      if (network_client_->connectionStatus() ==
          client::NetworkClient::ConnectionStatus::NotConnected) {
        QString marker("Client url: ");
        int start_pos = out.indexOf(marker);
        if (start_pos != -1) {
          start_pos += marker.length();
          int end_pos = out.indexOf("\n", start_pos);
          if (end_pos == -1) {
            server_process_->waitForReadyRead();
            QByteArray additional =
                server_process_->readLine(server_process_->bytesAvailable());
            out += additional;
            end_pos = out.indexOf("\n", start_pos);
          }
          QByteArray url = out.mid(start_pos, end_pos - start_pos);
          network_client_->connect(
              QString::fromUtf8(url), connection_dialog_->clientInterface(),
              connection_dialog_->clientName(), veles::util::version::string,
              "Veles UI", "Veles UI", is_local_server_);
        }
      }
      LogWidget::output()->write(out);
    }
  }
}

void ConnectionManager::serverProcessErrorOccurred(
    QProcess::ProcessError error) {
  QTextStream out(veles::ui::LogWidget::output());

  if (error == QProcess::FailedToStart) {
    out << "*************************************" << endl
        << "Failed to run python interpreter." << endl
        << "Make sure that python >= 3.5 is installed and" << endl
        << "server script location is set correctly." << endl
        << "*************************************" << endl;
  }
}

void ConnectionManager::updateConnectionStatus(
    client::NetworkClient::ConnectionStatus connection_status) {
  emit connectionStatusChanged(connection_status);
  disconnect_action_->setEnabled(
      connection_status !=
      client::NetworkClient::ConnectionStatus::NotConnected);
  if (connection_status == client::NetworkClient::ConnectionStatus::Connected) {
    sendListConnectionsMessage();
  }
}

void ConnectionManager::raiseConnectionDialog() {
  connection_dialog_->raise();
  connection_dialog_->activateWindow();
}

void ConnectionManager::sendListConnectionsMessage() {
  if (network_client_ != nullptr) {
    std::shared_ptr<proto::MsgListConnections> list_connections_message =
        std::make_shared<proto::MsgListConnections>(network_client_->nextQid(),
                                                    true);
    network_client_->sendMessage(list_connections_message);
  }
}

void ConnectionManager::messageReceived(const client::msg_ptr& message) {
  if (message->object_type == "connections_reply") {
    std::shared_ptr<proto::MsgConnectionsReply> connections_reply =
        std::dynamic_pointer_cast<proto::MsgConnectionsReply>(message);
    if (connections_reply) {
      QTextStream& out = *network_client_output_;
      out << "ConnectionManager: received updated list of connections:" << endl;

      if (connections_reply->connections) {
        for (const auto& connection : *connections_reply->connections) {
          out << "    id = " << connection->client_id << " name = \""
              << QString::fromStdString(*connection->client_name) << "\""
              << " type = \""
              << QString::fromStdString(*connection->client_type) << "\""
              << endl;
        }
      } else {
        out << "    -" << endl;
      }

      emit connectionsChanged(connections_reply->connections);
    }
  }
}

/*****************************************************************************/
/* ConnectionNotificationWidget */
/*****************************************************************************/

ConnectionNotificationWidget::ConnectionNotificationWidget(QWidget* parent)
    : QWidget(parent) {
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
      ui_->connection_status_text_label->setText("Not Connected");
      ui_->connection_status_icon_label->setPixmap(icon_not_connected_);
      break;
    case client::NetworkClient::ConnectionStatus::Connecting:
      ui_->connection_status_text_label->setText(
          "Connecting: " + util::settings::connection::currentProfile());
      ui_->connection_status_icon_label->setPixmap(icon_not_connected_);
      break;
    case client::NetworkClient::ConnectionStatus::Connected:
      ui_->connection_status_text_label->setText(
          "Connected: " + util::settings::connection::currentProfile());
      ui_->connection_status_icon_label->setPixmap(icon_connected_);
      break;
  }

  connection_status_ = connection_status;
}

void ConnectionNotificationWidget::timerEvent(QTimerEvent* /*event*/) {
  ++frame_;

  if (connection_status_ ==
      client::NetworkClient::ConnectionStatus::Connecting) {
    ui_->connection_status_icon_label->setPixmap(
        frame_ % 2 == 1 ? icon_connected_ : icon_not_connected_);
  } else if (connection_status_ ==
             client::NetworkClient::ConnectionStatus::NotConnected) {
    if (frame_ - last_status_change_ < 10) {
      ui_->connection_status_icon_label->setPixmap(
          frame_ % 2 == 1 ? icon_alarm_ : icon_not_connected_);
    } else {
      ui_->connection_status_icon_label->setPixmap(icon_not_connected_);
    }
  }
}

/*****************************************************************************/
/* ConnectionsWidget */
/*****************************************************************************/

ConnectionsWidget::ConnectionsWidget(QWidget* parent) : QWidget(parent) {
  users_icon_ = QPixmap(":/images/clients.png");
  users_icon_label_ = new QLabel(this);
  users_icon_label_->setPixmap(users_icon_);
  layout_ = new QHBoxLayout(this);
  setLayout(layout_);

  auto* scroll_area = new QScrollArea(this);
  scroll_area->setFrameShape(QFrame::NoFrame);
  scroll_area->setWidgetResizable(true);
  QWidget* scroll_area_contents = new QWidget(scroll_area);
  scroll_area_layout_ = new QHBoxLayout(scroll_area_contents);
  scroll_area_contents->setSizePolicy(QSizePolicy::Expanding,
                                      QSizePolicy::Minimum);
  scroll_area->setMaximumHeight(users_icon_label_->sizeHint().height());
  scroll_area_contents->setLayout(scroll_area_layout_);
  scroll_area_layout_->setSizeConstraint(QLayout::SetMinAndMaxSize);
  scroll_area_layout_->setMargin(0);
  scroll_area_layout_->addStretch(1);
  scroll_area_layout_->addWidget(users_icon_label_);
  scroll_area->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
  scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  scroll_area->setWidget(scroll_area_contents);

  layout_->addWidget(scroll_area);
  layout_->setStretchFactor(scroll_area, 1);
  layout_->addSpacing(10);

  users_icon_label_->hide();

  label_stylesheet_ = QString(
      "QLabel {"
      "background : palette(highlight);"
      "color : palette(highlighted-text);"
      "border-radius: 4px;"
      "border-width: 0px;"
      "padding-left: 5px;"
      "padding-right: 5px;"
      "}");
}

ConnectionsWidget::~ConnectionsWidget() {}

void ConnectionsWidget::updateConnectionStatus(
    client::NetworkClient::ConnectionStatus connection_status) {
  if (connection_status ==
      client::NetworkClient::ConnectionStatus::NotConnected) {
    clear();
  }
}

void ConnectionsWidget::updateConnections(
    const std::shared_ptr<std::vector<std::shared_ptr<proto::Connection>>>&
        connections) {
  clear();
  if (connections != nullptr && !connections->empty()) {
    users_icon_label_->show();

    for (const auto& connection : *connections) {
      QLabel* client_label = new QLabel;
      client_label->setText(QString::fromStdString(*connection->client_name));
      client_label->setToolTip(
          QString("<p><b>Client name:</b> %1</p>"
                  "<p><b>Client type:</b> %2</p>"
                  "<p><b>Client version:</b> %3</p>"
                  "<p><b>Client description:</b> %4</p>")
              .arg(QString::fromStdString(*connection->client_name))
              .arg(QString::fromStdString(*connection->client_type))
              .arg(QString::fromStdString(*connection->client_version))
              .arg(QString::fromStdString(*connection->client_description)));
      client_label->setStyleSheet(label_stylesheet_);
      scroll_area_layout_->addWidget(client_label);
      user_labels_.push_back(client_label);
    }
  }
}

void ConnectionsWidget::clear() {
  for (auto label : user_labels_) {
    label->deleteLater();
  }

  user_labels_.clear();
  users_icon_label_->hide();
}

}  // namespace ui
}  // namespace veles
