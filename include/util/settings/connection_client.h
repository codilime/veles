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
#pragma once

#include <QString>

namespace veles {
namespace util {
namespace settings {
namespace connection {

bool runServerDefault();
bool runServer();
void setRunServer(bool run_server);

QString serverHostDefault();
QString serverHost();
void setServerHost(const QString& server_host);

int serverPortDefault();
int serverPort();
void setServerPort(int server_port);

QString clientInterfaceDefault();
QString clientInterface();
void setClientInterface(const QString& client_interface);

QString clientNameDefault();
QString clientName();
void setClientName(const QString& client_name);

QString connectionKeyDefault();
QString connectionKey();
void setConnectionKey(const QString& connection_key);

QString databaseNameDefault();
QString databaseName();
void setDatabaseName(const QString& database_name);

QString serverScriptDefault();
QString serverScript();
void setServerScript(const QString& server_script);

QString certDirectoryDefault();
QString certDirectory();
void setCertDirectory(const QString& cert_directory);

QString serverUrlDefault();
QString serverUrl();
void setServerUrl(const QString& server_url);

bool sslEnabledDefault();
bool sslEnabled();
void setSslEnabled(bool ssl_enabled);

QString currentProfile();
void setCurrentProfile(const QString& profile);
QStringList profileList();
void removeProfile(const QString& profile);
QString uniqueProfileName(const QString& prefix);
void setDefaultProfile(const QString& profile);

}  // namespace connection
}  // namespace settings
}  // namespace util
}  // namespace veles
