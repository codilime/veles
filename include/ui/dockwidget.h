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

#include <map>
#include <set>

#include <QDateTime>
#include <QDockWidget>
#include <QDropEvent>
#include <QIcon>
#include <QMainWindow>
#include <QMenu>
#include <QObject>
#include <QPointer>
#include <QProxyStyle>
#include <QRubberBand>
#include <QString>
#include <QStringList>

#include "ui/fileblobmodel.h"

namespace veles {
namespace ui {

class MainWindowWithDetachableDockWidgets;

/*****************************************************************************/
/* QProxyStyleForDockWidgetWithIconOnTitleBar */
/*****************************************************************************/

class QProxyStyleForDockWidgetWithIconOnTitleBar : public QProxyStyle {
  Q_OBJECT

 public:
  explicit QProxyStyleForDockWidgetWithIconOnTitleBar(QStyle* default_style);

  void drawControl(QStyle::ControlElement element, const QStyleOption* option,
                   QPainter* painter, const QWidget* widget) const override;
};

/*****************************************************************************/
/* ActivateDockEventFilter */
/*****************************************************************************/
class ActivateDockEventFilter : public QObject {
  Q_OBJECT

 public:
  explicit ActivateDockEventFilter(QObject* parent = nullptr);

 protected:
  bool eventFilter(QObject* watched, QEvent* event) override;
};

/*****************************************************************************/
/* DockWidget */
/*****************************************************************************/

class DockWidget : public QDockWidget {
  Q_OBJECT

 public:
  DockWidget();
  ~DockWidget() override;
  const QAction* maximizeHereAction();
  static DockWidget* getParentDockWidget(QObject* obj);
  void addCloseAction();

 public slots:
  void displayContextMenu(const QPoint& pos);
  void moveToDesktop();
  void moveToWindow();
  void detachToNewTopLevelWindow();
  void detachToNewTopLevelWindowAndMaximize();
  void topLevelChangedNotify(bool top_level);
  void switchTitleBar(bool is_default);
  void centerTitleBarOnPosition(const QPoint& pos);
  void splitHorizontally();
  void splitVertically();

 protected:
  void focusInEvent(QFocusEvent* event) Q_DECL_OVERRIDE;
  void moveEvent(QMoveEvent* event) Q_DECL_OVERRIDE;
  void timerEvent(QTimerEvent* event) Q_DECL_OVERRIDE;
  QMenu* createMoveToDesktopMenu();
  QMenu* createMoveToWindowMenu();
  QAction* createMoveToNewWindowAction();
  QAction* createMoveToNewWindowAndMaximizeAction();
  void createSplitActions();

 protected:
  static constexpr int max_ticks_ = 4;
  static constexpr int step_msec_ = 100;

  int timer_id_ = 0;
  int ticks_ = 0;
  QMenu* context_menu_ = nullptr;
  QAction* detach_action_;
  QAction* maximize_here_action_;
  QAction* split_horizontally_action_;
  QAction* split_vertically_action_;
  QWidget* empty_title_bar_;
  QAction* dock_close_action_ = nullptr;
  QAction* next_tab_action_;
  QAction* prev_tab_action_;
};

/*****************************************************************************/
/* TabBarEventFilter */
/*****************************************************************************/

class TabBarEventFilter : public QObject {
  Q_OBJECT

 public:
  explicit TabBarEventFilter(QObject* parent = nullptr);

 public slots:
  void tabMoved(int from, int to);
  void currentChanged(int index);

 protected:
  bool eventFilter(QObject* watched, QEvent* event) Q_DECL_OVERRIDE;
  virtual bool mouseMove(QTabBar* tab_bar, QMouseEvent* event);
  virtual bool mouseButtonPress(QTabBar* tab_bar, QMouseEvent* event);
  virtual bool mouseButtonRelease(QTabBar* tab_bar, QMouseEvent* event);
  virtual bool mouseButtonDblClick(QTabBar* tab_bar, QMouseEvent* event);

  QTabBar* dragged_tab_bar_ = nullptr;
  int dragged_tab_index_ = -1;
  QPoint drag_init_pos_;
  static const int k_drag_treshold_ = 5;
};

/*****************************************************************************/
/* IconAwareView */
/*****************************************************************************/

class IconAwareView : public QMainWindow {
  Q_OBJECT

 public:
  IconAwareView(const QString& category, const QString& path);
  ~IconAwareView() override;
  virtual void reapplySettings(){};

 signals:
  void maximize();

 protected:
  void getOrCreateIcon(const QString& category, const QString& icon_path);
  static void deleteIcons();

  static std::map<QString, QIcon*> icons_;
};

}  // namespace ui
}  // namespace veles
