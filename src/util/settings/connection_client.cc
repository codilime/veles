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
#include <QSettings>
#include <QApplication>
#include <QStandardPaths>
#include <QFileInfo>

#include "util/settings/connection_client.h"

namespace veles {
namespace util {
namespace settings {
namespace connection {

bool default_run_server = true;
QString localhost("127.0.0.1");
QString default_database_file("veles.vdb");
int default_server_port = 3135;
bool default_shut_down_server = true;

bool runServerDefault() {
  return default_run_server;
}

bool runServer() {
  QSettings settings;
  return settings.value("conection.run_server", runServerDefault()).toBool();
}

void setRunServer(bool run_server) {
  QSettings settings;
  settings.setValue("conection.run_server", run_server);
}

QString serverHostDefault() {
  return localhost;
}

QString serverHost() {
  QSettings settings;
  return settings.value("conection.server", serverHostDefault()).toString();
}

void setServerHost(QString server_host) {
  QSettings settings;
  settings.setValue("conection.server", server_host);
}

int serverPortDefault() {
  return default_server_port;
}

int serverPort() {
  QSettings settings;
  return settings.value("conection.server_port", serverPortDefault()).toInt();
}

void setServerPort(int server_port) {
  QSettings settings;
  settings.setValue("conection.server_port", server_port);
}

QString clientInterfaceDefault() {
  return localhost;
}

QString clientInterface() {
  QSettings settings;
  return settings.value("conection.client_interface",
      clientInterfaceDefault()).toString();
}

void setClientInterface(QString client_interface) {
  QSettings settings;
  settings.setValue("conection.client_interface", client_interface);
}

QString clientName() {
  QSettings settings;
  return settings.value("conection.client_name", clientNameDefault()).toString();
}

QString clientNameDefault() {
  QString user_name;
#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    user_name = qgetenv("USER");
#elif defined(Q_OS_WIN)
    user_name = qgetenv("USERNAME");
#else
    user_name = "Veles UI";
#endif

  if(user_name.length() == 0) {
    user_name = "Veles UI";
  }

  return user_name;
}

void setClientName(QString client_name) {
  QSettings settings;
  settings.setValue("conection.client_name", client_name);
}

QString connectionKeyDefault() {
  return QString("");
}

QString connectionKey() {
  QSettings settings;
  return settings.value("conection.key", connectionKeyDefault()).toString();
}

void setConnectionKey(QString connection_key) {
  QSettings settings;
  settings.setValue("conection.key", connection_key);
}

QString databaseNameDefault() {
  return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/" + default_database_file;
}

QString databaseName() {
  QSettings settings;
  return settings.value("conection.database",
      databaseNameDefault()).toString();
}

void setDatabaseName(QString database_name) {
  QSettings settings;
  settings.setValue("conection.database", database_name);
}

QString serverScriptDefault() {
  QString server_script;
#if defined(Q_OS_WIN)
  server_script = qApp->applicationDirPath() + "/../veles-server/srv.py";
#elif defined(Q_OS_LINUX)
  server_script = qApp->applicationDirPath() + "/../share/veles-server/srv.py";
#else
  server_script = qApp->applicationDirPath() + "/../Resources/veles-server/srv.py";
#endif
  QFileInfo check_file(server_script);
  if (!check_file.exists()) {
    server_script = qApp->applicationDirPath() + "/python/srv.py";
  }
  return server_script;
}

QString serverScript() {
  QSettings settings;
  return settings.value("connection.server_script",
      serverScriptDefault()).toString();
}

void setServerScript(QString server_script) {
  QSettings settings;
  settings.setValue("connection.server_script", server_script);
}

bool shutDownServerOnQuitDefault() {
  return default_shut_down_server;
}

bool shutDownServerOnQuit() {
  QSettings settings;
  return settings.value("connection.shut_down_server_on_quit",
      shutDownServerOnQuitDefault()).toBool();
}

void setShutDownServerOnQuit(bool shut_down_server) {
  QSettings settings;
  settings.setValue("connection.shut_down_server_on_quit", shut_down_server);
}

}  // namespace connection
}  // namespace settings
}  // namespace util
}  // namespace veles
