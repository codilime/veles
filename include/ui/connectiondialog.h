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

#include <QDialog>
#include <QFileDialog>

namespace Ui {
class ConnectionDialog;
}

namespace veles {
namespace ui {

class ConnectionDialog : public QDialog {
  Q_OBJECT

 public:
  explicit ConnectionDialog(QWidget* parent = nullptr);
  ~ConnectionDialog() override;

  bool runANewServer();
  QString serverHost();
  int serverPort();
  QString clientInterface();
  QString authenticationKey();
  QString clientName();
  QString databaseFile();
  QString serverScript();
  QString certificateDir() const;
  QString serverUrl() const;
  bool sslEnabled() const;

 public slots:
  void serverLocalhost();
  void clientLocalhost();
  void randomKey();
  void newServerToggled(bool toggled);
  void sslEnabledToggled(bool toggled);
  void databaseFileSelected(const QString& file_name);
  void serverFileSelected(const QString& file_name);
  void certificateDirSelected(const QString& dir_name);
  void loadDefaultValues();
  void loadSettings();
  void loadProfiles();
  void saveSettings();
  void dialogAccepted();
  void profileChanged(int index);
  void profileRemoved();
  void newProfile();
  void profileNameEdited(const QString& name);
  void defaultProfile();

 protected:
  void showEvent(QShowEvent* event) override;

 private:
  Ui::ConnectionDialog* ui_;
  QFileDialog* db_file_dialog_;
  QFileDialog* server_file_dialog_;
  QFileDialog* certificate_dir_dialog_;
};

}  // namespace ui
}  // namespace veles
