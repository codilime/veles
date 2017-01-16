/*
 * Copyright 2016 CodiLime
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

#include <set>

#include <QDropEvent>
#include <QDockWidget>
#include <QMainWindow>
#include <QMenu>

#include "dbif/promise.h"
#include "dbif/types.h"

#include "ui/optionsdialog.h"

namespace veles {
namespace ui {

class MainWindowWithDetachableDockWidgets;

/*****************************************************************************/
/* DockWidget */
/*****************************************************************************/

class DockWidget : public QDockWidget {
  Q_OBJECT

 public:
  DockWidget();
  virtual ~DockWidget();

 public slots:
  void displayContextMenu(const QPoint& pos);
  void moveToDesktop();
  void moveToWindow();

 protected:
  void moveEvent(QMoveEvent *event) Q_DECL_OVERRIDE;
  void timerEvent(QTimerEvent* event) Q_DECL_OVERRIDE;
  QMenu* createMoveToDesktopMenu();
  QMenu* createMoveToWindowMenu();

 protected:
  static constexpr int max_ticks_ = 4;
  static constexpr int step_msec_ = 100;

  int timer_id_;
  int ticks_;
  QMenu* context_menu_;
};

/*****************************************************************************/
/* MainWindowWithDetachableDockWidgets */
/*****************************************************************************/

class MainWindowWithDetachableDockWidgets: public QMainWindow {
  Q_OBJECT

 public:
  MainWindowWithDetachableDockWidgets(QWidget* parent = nullptr);
  virtual ~MainWindowWithDetachableDockWidgets();
  void addTab(QWidget *widget, const QString &title);
  void bringDockWidgetToFront(QDockWidget* dock_widget);
  void moveDockWidgetToWindow(DockWidget* dock_widget);

  static bool intersectsWithAnyMainWindow(DockWidget* dock_widget);
  static MainWindowWithDetachableDockWidgets* getParentCandidateForDockWidget(
      DockWidget* dock_widget);
  static const std::set<MainWindowWithDetachableDockWidgets*>& getMainWindows();
  static MainWindowWithDetachableDockWidgets* getFirstMainWindow();
  static MainWindowWithDetachableDockWidgets* getOrCreateWindowForScreen(
      int screen);
  static MainWindowWithDetachableDockWidgets* getOwnerOfDockWidget(
        DockWidget* dock_widget);

 public slots:
  void dockLocationChanged(Qt::DockWidgetArea area);
  void tabCloseRequested(int index);

 protected:
  bool event(QEvent* event) Q_DECL_OVERRIDE;
  QDockWidget* tabToDockWidget(QTabBar* tab_bar, int index);
  void updateCloseButtonsOnTabBars();

 private:
  static std::set<MainWindowWithDetachableDockWidgets*> main_windows_;
  static MainWindowWithDetachableDockWidgets* first_main_window_;
  static int last_created_window_id_;
};

/*****************************************************************************/
/* VelesMainWindow */
/*****************************************************************************/

class VelesMainWindow : public MainWindowWithDetachableDockWidgets {
  Q_OBJECT

 public:
  VelesMainWindow();
  void addFile(QString path);

 protected:
  void dropEvent(QDropEvent *ev) Q_DECL_OVERRIDE;
  void dragEnterEvent(QDragEnterEvent *ev) Q_DECL_OVERRIDE;

 private slots:
  void newFile();
  void open();
  void about();

 private:
  void init();
  void createActions();
  void createMenus();
  void createDb();
  void createFileBlob(QString);
  void createHexEditTab(QString, dbif::ObjectHandle);

  QMenu *fileMenu;
  QMenu *visualisationMenu;
  QMenu *helpMenu;

  QAction *newFileAct;
  QAction *openAct;
  QAction *exitAct;
  QAction *optionsAct;

  QAction *aboutAct;
  QAction *aboutQtAct;

  dbif::ObjectHandle database;
  OptionsDialog *optionsDialog;
};

}  // namespace ui
}  // namespace veles

#endif
