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
#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QSettings>
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
QString default_default_profile("local server");
QString current_profile;

void setDefaultProfile(const QString& profile) {
  QSettings settings;
  settings.setValue("default_profile", profile);
}

QString currentProfile() {
  if (current_profile.isEmpty()) {
    QSettings settings;
    current_profile =
        settings.value("default_profile", default_default_profile).toString();
  }
  return current_profile;
}

void setCurrentProfile(const QString& profile) { current_profile = profile; }

void removeProfile(const QString& profile) {
  QSettings settings;
  QMap<QString, QVariant> profiles = settings.value("profiles").toMap();
  profiles.remove(profile);
  if (profiles.empty()) {
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
  QMap<QString, QVariant> profiles =
      settings.value("profiles", default_profiles).toMap();
  return profiles.keys();
}

QVariant profileSettings(const QString& key, const QVariant& defaultValue) {
  QSettings settings;
  QMap<QString, QVariant> profiles =
      settings.value("profiles", QSettings::SettingsMap()).toMap();
  QSettings::SettingsMap profile_settings =
      profiles.value(currentProfile(), QSettings::SettingsMap()).toMap();
  return profile_settings.value(key, defaultValue);
}

void setProfileSettings(const QString& key, const QVariant& value) {
  QSettings settings;
  QMap<QString, QVariant> profiles = settings.value("profiles").toMap();
  QSettings::SettingsMap profile = profiles[currentProfile()].toMap();
  profile[key] = value;
  profiles[currentProfile()] = profile;
  settings.setValue("profiles", profiles);
}

QString uniqueProfileName(const QString& prefix) {
  auto profile_list = profileList();
  int next_id = 0;
  QString suffix;
  while (profile_list.contains(prefix + suffix)) {
    suffix = " (" + QString::number(next_id++) + ")";
  }
  return prefix + suffix;
}

bool runServerDefault() { return default_run_server; }

bool runServer() {
  return profileSettings("connection.run_server", runServerDefault()).toBool();
}

void setRunServer(bool run_server) {
  setProfileSettings("connection.run_server", run_server);
}

QString serverHostDefault() { return localhost; }

QString serverHost() {
  return profileSettings("connection.server", serverHostDefault()).toString();
}

void setServerHost(const QString& server_host) {
  setProfileSettings("connection.server", server_host);
}

int serverPortDefault() { return default_server_port; }

int serverPort() {
  return profileSettings("connection.server_port", serverPortDefault()).toInt();
}

void setServerPort(int server_port) {
  setProfileSettings("connection.server_port", server_port);
}

QString clientInterfaceDefault() { return localhost; }

QString clientInterface() {
  return profileSettings("connection.client_interface",
                         clientInterfaceDefault())
      .toString();
}

void setClientInterface(const QString& client_interface) {
  setProfileSettings("connection.client_interface", client_interface);
}

QString clientName() {
  return profileSettings("connection.client_name", clientNameDefault())
      .toString();
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

  if (user_name.length() == 0) {
    user_name = "Veles UI";
  }

  return user_name;
}

void setClientName(const QString& client_name) {
  setProfileSettings("connection.client_name", client_name);
}

QString connectionKeyDefault() { return QString(""); }

QString connectionKey() {
  return profileSettings("connection.key", connectionKeyDefault()).toString();
}

void setConnectionKey(const QString& connection_key) {
  QSettings settings;
  setProfileSettings("connection.key", connection_key);
}

QString databaseNameDefault() {
  return QStandardPaths::writableLocation(
             QStandardPaths::AppLocalDataLocation) +
         "/" + default_database_file;
}

QString databaseName() {
  return profileSettings("connection.database", databaseNameDefault())
      .toString();
}

void setDatabaseName(const QString& database_name) {
  setProfileSettings("connection.database", database_name);
}

QString serverScriptDefault() {
  QString server_script;
#if defined(Q_OS_WIN)
  server_script = qApp->applicationDirPath() + "/../veles-server/srv.py";
#elif defined(Q_OS_LINUX)
  server_script = qApp->applicationDirPath() + "/../share/veles-server/srv.py";
#elif defined(Q_OS_MAC)
  server_script =
      qApp->applicationDirPath() + "/../Resources/veles-server/srv.py";
#else
#warning \
    "This OS is not officially supported, you may need to set this path manually."
#endif
  QFileInfo check_file(server_script);
  if (!check_file.exists()) {
    server_script = qApp->applicationDirPath() + "/python/srv.py";
  }
  return QDir::cleanPath(server_script);
}

QString serverScript() {
  return profileSettings("connection.server_script", serverScriptDefault())
      .toString();
}

void setServerScript(const QString& server_script) {
  setProfileSettings("connection.server_script", server_script);
}

QString certDirectoryDefault() {
  return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
}

QString certDirectory() {
  return profileSettings("connection.cert_dir", certDirectoryDefault())
      .toString();
}

void setCertDirectory(const QString& cert_directory) {
  setProfileSettings("connection.cert_dir", cert_directory);
}

QString serverUrlDefault() { return ""; }

QString serverUrl() {
  return profileSettings("connection.server_url", serverUrlDefault())
      .toString();
}

void setServerUrl(const QString& server_url) {
  setProfileSettings("connection.server_url", server_url);
}

bool sslEnabledDefault() { return true; }

bool sslEnabled() {
  return profileSettings("connection.ssl_enabled", sslEnabledDefault())
      .toBool();
}

void setSslEnabled(bool ssl_enabled) {
  setProfileSettings("connection.ssl_enabled", ssl_enabled);
}

}  // namespace connection
}  // namespace settings
}  // namespace util
}  // namespace veles
