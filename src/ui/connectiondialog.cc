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

  serverLocalhost();
  clientLocalhost();
  randomKey();
  userAsClientName();

  ui_->database_line_edit->setText("hack-o-store.veles");
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
  QString client_name;
#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
  client_name = qgetenv("USER");
#elif defined(Q_OS_WINDOWS)
  client_name = qgetenv("USERNAME");
#else
  client_name = "Veles UI";
#endif

  if(client_name.length() == 0) {
    client_name = "Veles UI";
  }

  ui_->client_name_line_edit->setText(client_name);
}

void ConnectionDialog::newServerToggled(bool toggled) {
  ui_->database_label->setEnabled(toggled);
  ui_->database_line_edit->setEnabled(toggled);
  ui_->select_database_button->setEnabled(toggled);

  ui_->server_executable_label->setEnabled(toggled);
  ui_->server_executable_line_edit->setEnabled(toggled);
  ui_->select_server_executable_button->setEnabled(toggled);

  ui_->random_key_button->setEnabled(toggled);
}

void ConnectionDialog::databaseFileSelected(const QString& file_name) {
  ui_->database_line_edit->setText(file_name);
}

void ConnectionDialog::serverFileSelected(const QString& file_name) {
  ui_->server_executable_line_edit->setText(file_name);
}

void ConnectionDialog::showEvent(QShowEvent* event) {
  ui_->server_host_line_edit->setFocus(Qt::OtherFocusReason);
}

}  // namespace ui
}  // namespace veles
