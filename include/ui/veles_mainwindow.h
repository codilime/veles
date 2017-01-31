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
#ifndef VELES_MAINWINDOW_H
#define VELES_MAINWINDOW_H

#include <QDropEvent>
#include <QDockWidget>
#include <QMainWindow>
#include <QMenu>
#include <QString>
#include <QStringList>
#include <QIcon>

#include "dbif/promise.h"
#include "dbif/types.h"

#include "ui/dockwidget.h"
#include "ui/optionsdialog.h"

namespace veles {
namespace ui {

/*****************************************************************************/
/* VelesMainWindow */
/*****************************************************************************/

class VelesMainWindow : public MainWindowWithDetachableDockWidgets {
  Q_OBJECT

 public:
  VelesMainWindow();
  void addFile(QString path);
  QStringList parsersList() {return parsers_list_;}

 protected:
  void dropEvent(QDropEvent *ev) Q_DECL_OVERRIDE;
  void dragEnterEvent(QDragEnterEvent *ev) Q_DECL_OVERRIDE;

 private slots:
  void newFile();
  void open();
  void about();
  void updateParsers(dbif::PInfoReply replay);
  void showDatabase();
  void showLog();

 private:
  void init();
  void createActions();
  void createMenus();
  void createDb();
  void createFileBlob(QString);
  void createHexEditTab(QString, dbif::ObjectHandle);
  void createLogWindow();

  QMenu *file_menu_;
  QMenu *view_menu_;
  QMenu *help_menu_;

  QAction *new_file_act_;
  QAction *open_act_;
  QAction *exit_act_;
  QAction *options_act_;

  QAction *about_act_;
  QAction *about_qt_act_;

  QAction *show_database_act_;
  QAction *show_log_act_;

  dbif::ObjectHandle database_;
  OptionsDialog *options_dialog_;

  QStringList parsers_list_;

  QPointer<DockWidget> database_dock_widget_;
  QPointer<DockWidget> log_dock_widget_;
};

}  // namespace ui
}  // namespace veles

#endif
