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
#ifndef VELES_UI_CONNECTIONMANAGER_H
#define VELES_UI_CONNECTIONMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QProcess>
#include <QLabel>
#include <QPixmap>
#include <QHBoxLayout>

#include "ui/connectiondialog.h"
#include "ui_connectionnotificationwidget.h"

namespace veles {
namespace ui {

/*****************************************************************************/
/* ConnectionManager */
/*****************************************************************************/

class ConnectionManager : public QObject {
  Q_OBJECT

 public:
  enum class ConnectionState {NotConnected, Connecting, Connected};

  ConnectionManager(QWidget* parent = nullptr);
  virtual ~ConnectionManager();

  QAction* showConnectionDialogAction();
  QAction* disconnectAction();
  QAction* killLocallyCreatedServerAction();

 public slots:
  void locallyCreatedServerStarted();
  void locallyCreatedServerFinished(int exit_code,
      QProcess::ExitStatus exit_status);
  void connectionDialogAccepted();
  void startLocalServer();
  void killLocalServer();
  void disconnect();
  void serverProcessReadyRead();

 signals:
  void connectionStateChanged(ConnectionState connection_state);

 private:
  QAction* show_connection_dialog_action_;
  QAction* disconnect_action_;
  QAction* kill_locally_created_server_action_;
  QProcess* server_process_;
  ConnectionDialog* connection_dialog_;
};

/*****************************************************************************/
/* ConnectionNotificationWidget */
/*****************************************************************************/

class ConnectionNotificationWidget : public QWidget {
  Q_OBJECT

 public:
  ConnectionNotificationWidget(QWidget* parent = nullptr);
  virtual ~ConnectionNotificationWidget();

 public slots:
  void updateConnectionState(
      ConnectionManager::ConnectionState connection_state);

 protected:
  void timerEvent(QTimerEvent* event);

 private:
  ConnectionManager::ConnectionState connection_state_;

  int frame_;
  int last_status_change_;

  QPixmap icon_connected_;
  QPixmap icon_not_connected_;
  QPixmap icon_alarm_;

  Ui::ConnectionNotificationWidget* ui_;
};

}  // namespace ui
}  // namespace veles

#endif // VELES_UI_CONNECTIONMANAGER_H
