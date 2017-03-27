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
#include "ui/connectiondialog.h"
#include "ui_connectiondialog.h"

#include <climits>
#include <cstdint>
#include <ctime>
#include <limits>
#include <random>

#include <QtGlobal>
#include <QSettings>

namespace veles {
namespace ui {

ConnectionDialog::ConnectionDialog(QWidget* parent)
    : QDialog(parent),
      ui_(new Ui::ConnectionDialog) {
  ui_->setupUi(this);

  connect(ui_->ok_button, &QPushButton::clicked, this, &QDialog::accept);
  connect(ui_->cancel_button, &QPushButton::clicked, this, &QDialog::reject);
  connect(ui_->server_localhost_button, &QPushButton::clicked,
      this, &ConnectionDialog::serverLocalhost);
  connect(ui_->client_localhost_button, &QPushButton::clicked,
        this, &ConnectionDialog::clientLocalhost);
  connect(ui_->random_key_button, &QPushButton::clicked,
      this, &ConnectionDialog::randomKey);
  connect(ui_->new_server_radio_button, &QPushButton::toggled,
      this, &ConnectionDialog::newServerToggled);

  db_file_dialog_ = new QFileDialog(this);
  db_file_dialog_->setAcceptMode(QFileDialog::AcceptOpen);
  db_file_dialog_->setFileMode(QFileDialog::AnyFile);
  db_file_dialog_->setNameFilters({"All files (*.*)"});

  connect(ui_->select_database_button, &QPushButton::clicked,
        db_file_dialog_, &QFileDialog::show);
  connect(db_file_dialog_, &QFileDialog::fileSelected,
      this, &ConnectionDialog::databaseFileSelected);

  server_file_dialog_ = new QFileDialog(this);
  server_file_dialog_->setAcceptMode(QFileDialog::AcceptOpen);
  server_file_dialog_->setFileMode(QFileDialog::ExistingFile);
  server_file_dialog_->setNameFilters({"Python scripts (*.py)"});

  connect(ui_->select_server_executable_button, &QPushButton::clicked,
      server_file_dialog_, &QFileDialog::show);
  connect(server_file_dialog_, &QFileDialog::fileSelected,
      this, &ConnectionDialog::serverFileSelected);
  connect(ui_->save_settings_check_box, &QCheckBox::toggled,
      this, &ConnectionDialog::saveSettingsToggled);
  connect(ui_->load_defaults_button, &QPushButton::clicked,
      this, &ConnectionDialog::loadDefaultValues);
  connect(this, &QDialog::accepted, this, &ConnectionDialog::dialogAccepted);

  ui_->save_settings_check_box->setChecked(false);
  newServerToggled(ui_->new_server_radio_button->isChecked());
}

ConnectionDialog::~ConnectionDialog() {
  db_file_dialog_->deleteLater();
  delete ui_;
}

bool ConnectionDialog::runANewServer() {
  return ui_->new_server_radio_button->isChecked();
}

QString ConnectionDialog::serverHost() {
  return ui_->server_host_line_edit->text();
}

int ConnectionDialog::serverPort() {
  return ui_->port_spin_box->value();
}

QString ConnectionDialog::clientInterface() {
  return ui_->client_interface_line_edit->text();
}

QString ConnectionDialog::authenticationKey() {
  return ui_->authentication_key_label->text();
}

QString ConnectionDialog::clientName() {
  return ui_->client_name_line_edit->text();
}

QString ConnectionDialog::databaseFile() {
  return ui_->database_line_edit->text();
}

QString ConnectionDialog::serverScript() {
  return ui_->server_executable_line_edit->text();
}

bool ConnectionDialog::shutDownServerOnClose() {
  return ui_->shut_down_server_check_box->isChecked();
}

QString ConnectionDialog::userName() {
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

void ConnectionDialog::serverLocalhost() {
  ui_->server_host_line_edit->setText("127.0.0.1");
}

void ConnectionDialog::clientLocalhost() {
  ui_->client_interface_line_edit->setText("127.0.0.1");
}

void ConnectionDialog::randomKey() {
  // TODO: This is cryptographically-secure on all modern OS-es, but this isn't
  // explicitely guarateed by the standard. We should fix it someday.
  std::random_device rd;
  std::uniform_int_distribution<uint32_t> uniform;
  auto gen_key_part = [&rd, &uniform](){
    return QString("%1").arg(uniform(rd), 8 /* width */, 16 /* base */,
                             QChar('0'));
  };
  ui_->key_line_edit->setText(gen_key_part() + gen_key_part() + gen_key_part()
                              + gen_key_part());
}

void ConnectionDialog::userAsClientName() {
  ui_->client_name_line_edit->setText(userName());
}

void ConnectionDialog::newServerToggled(bool toggled) {
  ui_->database_label->setEnabled(toggled);
  ui_->database_line_edit->setEnabled(toggled);
  ui_->select_database_button->setEnabled(toggled);

  ui_->server_executable_label->setEnabled(toggled);
  ui_->server_executable_line_edit->setEnabled(toggled);
  ui_->select_server_executable_button->setEnabled(toggled);

  ui_->random_key_button->setEnabled(toggled);
  ui_->shut_down_server_check_box->setEnabled(toggled);
}

void ConnectionDialog::databaseFileSelected(const QString& file_name) {
  ui_->database_line_edit->setText(file_name);
}

void ConnectionDialog::serverFileSelected(const QString& file_name) {
  ui_->server_executable_line_edit->setText(file_name);
}

QString localhost("127.0.0.1");
QString default_database("veles.vdb");
int default_port = 1905;
bool default_shut_down_server = true;
QString settings_true("yes, please");
QString settings_false("no, thanks");

void ConnectionDialog::loadDefaultValues() {
  ui_->new_server_radio_button->setChecked(true);
  serverLocalhost();
  ui_->port_spin_box->setValue(default_port);
  ui_->key_line_edit->setText("");
  clientLocalhost();
  userAsClientName();
  ui_->database_line_edit->setText(default_database);
  ui_->server_executable_line_edit->setText("");
  ui_->shut_down_server_check_box->setChecked(default_shut_down_server);
}

void ConnectionDialog::loadSettings() {
  QSettings settings;

  (settings.value("conection.run_server", settings_true).toString()
      == settings_true ? ui_->new_server_radio_button
      : ui_->existing_server_radio_button)->setChecked(true);
  ui_->server_host_line_edit->setText(
      settings.value("conection.server", localhost).toString());
  ui_->port_spin_box->setValue(
      settings.value("conection.server_port", default_port).toInt());
  ui_->key_line_edit->setText(
      settings.value("conection.key").toString());
  ui_->client_interface_line_edit->setText(
      settings.value("conection.client_interface", localhost).toString());
  ui_->client_name_line_edit->setText(
      settings.value("conection.client_name", userName()).toString());
  ui_->database_line_edit->setText(
      settings.value("conection.database", default_database).toString());
  ui_->server_executable_line_edit->setText(
      settings.value("connection.server_script", "").toString());
  ui_->shut_down_server_check_box->setChecked(
      settings.value("connection.shut_down_server_on_quit",
      default_shut_down_server ? settings_true : settings_false).toString()
      == settings_true);
}

void ConnectionDialog::saveSettings() {
  QSettings settings;

  settings.setValue("conection.run_server",
      ui_->new_server_radio_button->isChecked() ?
      settings_true : settings_false);
  settings.setValue("conection.server",
      ui_->server_host_line_edit->text());
  settings.setValue("conection.server_port",
      ui_->port_spin_box->value());
  settings.setValue("conection.client_interface",
      ui_->client_interface_line_edit->text());
  settings.setValue("conection.client_name",
      ui_->client_name_line_edit->text());
  settings.setValue("conection.key",
      ui_->save_key_check_box->isChecked() ?
      ui_->key_line_edit->text() : "");
  settings.setValue("conection.database",
      ui_->database_line_edit->text());
  settings.setValue("connection.server_script",
      ui_->server_executable_line_edit->text());
  settings.setValue("connection.shut_down_server_on_quit",
      ui_->shut_down_server_check_box->isChecked() ?
          settings_true : settings_false);
}

void ConnectionDialog::saveSettingsToggled(bool toggled) {
  ui_->save_key_check_box->setEnabled(toggled);
}

void ConnectionDialog::dialogAccepted() {
  saveSettings();
}

void ConnectionDialog::showEvent(QShowEvent* event) {
  loadSettings();
  ui_->server_host_line_edit->setFocus(Qt::OtherFocusReason);
}

}  // namespace ui
}  // namespace veles
