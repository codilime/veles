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

#include <QDockWidget>
#include <QDropEvent>
#include <QIcon>
#include <QLabel>
#include <QMainWindow>
#include <QMenu>
#include <QProcess>
#include <QString>
#include <QStringList>

#include "client/networkclient.h"
#include "dbif/promise.h"
#include "dbif/types.h"

#include "ui/connectiondialog.h"
#include "ui/connectionmanager.h"
#include "ui/dockwidget.h"
#include "ui/optionsdialog.h"
#include "ui/shortcutssettings.h"

namespace veles {
namespace ui {

/*****************************************************************************/
/* VelesMainWindow */
/*****************************************************************************/

class VelesMainWindow : public MainWindowWithDetachableDockWidgets {
  Q_OBJECT

 public:
  VelesMainWindow();
  void addFile(const QString& path);
  QStringList parsersList() { return parsers_list_; }

 protected:
  void dropEvent(QDropEvent* ev) override;
  void dragEnterEvent(QDragEnterEvent* ev) override;
  void showEvent(QShowEvent* event) override;

 private slots:
  void newFile();
  void open();
  void about();
  void updateParsers(const dbif::PInfoReply& reply);
  void showDatabase();
  void showLog();
  void updateConnectionStatus(
      client::NetworkClient::ConnectionStatus connection_status);

 signals:
  void shown();

 private:
  void init();
  void createActions();
  void createMenus();
  void createDb();
  void createFileBlob(const QString& file_name);
  void createHexEditTab(const QString& fileName,
                        const dbif::ObjectHandle& fileBlob);
  void createLogWindow();

  QMenu* file_menu_;
  QMenu* view_menu_;
  QMenu* help_menu_;

  QAction* new_file_act_;
  QAction* open_act_;
  QAction* exit_act_;
  QAction* options_act_;
  QAction* shortcuts_act_;

  QAction* about_act_;
  QAction* about_qt_act_;

  QAction* show_database_act_;
  QAction* show_log_act_;

  dbif::ObjectHandle database_;
  OptionsDialog* options_dialog_;
  ShortcutsDialog* shortcuts_dialog_;

  QStringList parsers_list_;

  QPointer<DockWidget> database_dock_widget_;
  QPointer<DockWidget> log_dock_widget_;

  ConnectionManager* connection_manager_;
  ConnectionNotificationWidget* connection_notification_widget_;

  QToolBar* tool_bar_;

  std::list<QString> files_to_upload_once_connected_;
};

}  // namespace ui
}  // namespace veles
