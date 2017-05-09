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
void setServerHost(QString server_host);

int serverPortDefault();
int serverPort();
void setServerPort(int server_port);

QString clientInterfaceDefault();
QString clientInterface();
void setClientInterface(QString client_interface);

QString clientNameDefault();
QString clientName();
void setClientName(QString client_name);

QString connectionKeyDefault();
QString connectionKey();
void setConnectionKey(QString connection_key);

QString databaseNameDefault();
QString databaseName();
void setDatabaseName(QString database_name);

QString serverScriptDefault();
QString serverScript();
void setServerScript(QString server_script);

bool shutDownServerOnDisconnectDefault();
bool shutDownServerOnDisconnect();
void setShutDownServerOnDisconnect(bool shut_down_server);

QString currentProfile();
void setCurrentProfile(QString profile);
QStringList profileList();
void removeProfile(QString profile);
QString uniqueProfileName(QString prefix);
void setDefaultProfile(QString profile);

}  // namespace connection
}  // namespace settings
}  // namespace util
}  // namespace veles
