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
QString default_default_profile("local server");
QString current_profile;

void setDefaultProfile(QString profile) {
  QSettings settings;
  settings.setValue("default_profile", profile);
}

QString currentProfile() {
  if (current_profile.isEmpty()) {
    QSettings settings;
    current_profile = settings.value("default_profile", default_default_profile).toString();
  }
  return current_profile;
}

void setCurrentProfile(QString profile) {
  current_profile = profile;
}

void removeProfile(QString profile) {
  QSettings settings;
  QMap<QString, QVariant> profiles = settings.value("profiles").toMap();
  profiles.remove(profile);
  if (profiles.size() == 0) {
    settings.remove("profiles");
  } else {
    settings.setValue("profiles", profiles);
  }
  if (profile == currentProfile()) {
    auto profiles = profileList();
    if (profiles.empty()) {
      setCurrentProfile(default_default_profile);
    } else {
      setCurrentProfile(profiles[0]);
    }
  }

  if (profile == settings.value("default_profile")) {
    settings.remove("default_profile");
  }
}

QStringList profileList() {
  QSettings settings;
  QMap<QString, QVariant> default_profiles;
  default_profiles[default_default_profile] = QSettings::SettingsMap();
  QMap<QString, QVariant> profiles = settings.value("profiles", default_profiles).toMap();
  return profiles.keys();
}

QVariant profileSettings(QString key, QVariant defaultValue) {
  QSettings settings;
  QMap<QString, QVariant> profiles = settings.value("profiles", QSettings::SettingsMap()).toMap();
  QSettings::SettingsMap profile_settings = profiles.value(currentProfile(), QSettings::SettingsMap()).toMap();
  return profile_settings.value(key, defaultValue);
}

void setProfileSettings(QString key, QVariant value) {
  QSettings settings;
  QMap<QString, QVariant> profiles = settings.value("profiles").toMap();
  QSettings::SettingsMap profile = profiles[currentProfile()].toMap();
  profile[key] = value;
  profiles[currentProfile()] = profile;
  settings.setValue("profiles", profiles);
}

QString uniqueProfileName(QString prefix) {
  auto profile_list = profileList();
  int next_id = 0;
  QString suffix;
  while (profile_list.contains(prefix + suffix)) {
    suffix = " (" + QString::number(next_id++) + ")";
  }
  return prefix + suffix;
}

bool runServerDefault() {
  return default_run_server;
}

bool runServer() {
  return profileSettings("conection.run_server", runServerDefault()).toBool();
}

void setRunServer(bool run_server) {
  setProfileSettings("conection.run_server", run_server);
}

QString serverHostDefault() {
  return localhost;
}

QString serverHost() {
  return profileSettings("conection.server", serverHostDefault()).toString();
}

void setServerHost(QString server_host) {
  setProfileSettings("conection.server", server_host);
}

int serverPortDefault() {
  return default_server_port;
}

int serverPort() {
  return profileSettings("conection.server_port", serverPortDefault()).toInt();
}

void setServerPort(int server_port) {
  setProfileSettings("conection.server_port", server_port);
}

QString clientInterfaceDefault() {
  return localhost;
}

QString clientInterface() {
  return profileSettings("conection.client_interface",
      clientInterfaceDefault()).toString();
}

void setClientInterface(QString client_interface) {
  setProfileSettings("conection.client_interface", client_interface);
}

QString clientName() {
  return profileSettings("conection.client_name", clientNameDefault()).toString();
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
  setProfileSettings("conection.client_name", client_name);
}

QString connectionKeyDefault() {
  return QString("");
}

QString connectionKey() {
  return profileSettings("conection.key", connectionKeyDefault()).toString();
}

void setConnectionKey(QString connection_key) {
  QSettings settings;
  setProfileSettings("conection.key", connection_key);
}

QString databaseNameDefault() {
  return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/" + default_database_file;
}

QString databaseName() {
  return profileSettings("conection.database",
      databaseNameDefault()).toString();
}

void setDatabaseName(QString database_name) {
  setProfileSettings("conection.database", database_name);
}

QString serverScriptDefault() {
#if defined(Q_OS_WIN)
  return qApp->applicationDirPath() + "/../veles-server/srv.py";
#elif defined(Q_OS_LINUX)
  return qApp->applicationDirPath() + "/../share/veles-server/srv.py";
#else
  return qApp->applicationDirPath() + "/../Resources/veles-server/srv.py";
#endif
}

QString serverScript() {
  return profileSettings("connection.server_script",
      serverScriptDefault()).toString();
}

void setServerScript(QString server_script) {
  setProfileSettings("connection.server_script", server_script);
}

bool shutDownServerOnQuitDefault() {
  return default_shut_down_server;
}

bool shutDownServerOnQuit() {
  return profileSettings("connection.shut_down_server_on_quit",
      shutDownServerOnQuitDefault()).toBool();
}

void setShutDownServerOnQuit(bool shut_down_server) {
  setProfileSettings("connection.shut_down_server_on_quit", shut_down_server);
}

}  // namespace connection
}  // namespace settings
}  // namespace util
}  // namespace veles
