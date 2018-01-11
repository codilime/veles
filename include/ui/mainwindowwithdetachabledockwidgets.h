#pragma once

#include <set>

#include <QMainWindow>
#include <QString>
#include <QWidget>

#include "dockwidget.h"

namespace veles {
namespace ui {

/*****************************************************************************/
/* MainWindowWithDetachableDockWidgets */
/*****************************************************************************/

class MainWindowWithDetachableDockWidgets : public QMainWindow {
  Q_OBJECT

 public:
  explicit MainWindowWithDetachableDockWidgets(QWidget* parent = nullptr);
  ~MainWindowWithDetachableDockWidgets() override;
  DockWidget* addTab(QWidget* widget, const QString& title,
                     DockWidget* sibling = nullptr);
  DockWidget* wrapWithDock(QWidget* widget, const QString& title);

  void bringDockWidgetToFront(QDockWidget* dock_widget);
  void moveDockWidgetToWindow(DockWidget* dock_widget);
  void findTwoNonTabifiedDocks(DockWidget** sibling1, DockWidget** sibling2);
  DockWidget* findDockNotTabifiedWith(DockWidget* dock_widget);
  DockWidget* findDockNotTabifiedWith(QWidget* widget);
  QDockWidget* findSibling(QDockWidget* dock_widget);
  void setDockWidgetsWithNoTitleBars(bool no_title_bars);
  bool dockWidgetsWithNoTitleBars();
  QDockWidget* tabToDockWidget(QTabBar* tab_bar, int index);
  QPair<QTabBar*, int> dockWidgetToTab(QDockWidget* dock_widget);
  void splitDockWidget2(QDockWidget* first, QDockWidget* second,
                        Qt::Orientation orientation);
  void showRubberBand(bool show);

  static void splitDockWidget2(QMainWindow* main_window, QDockWidget* first,
                               QDockWidget* second,
                               Qt::Orientation orientation);
  static MainWindowWithDetachableDockWidgets* getParentMainWindow(QObject* obj);
  static bool intersectsWithAnyMainWindow(DockWidget* dock_widget);
  static MainWindowWithDetachableDockWidgets* getParentCandidateForDockWidget(
      DockWidget* dock_widget);
  static const std::set<MainWindowWithDetachableDockWidgets*>& getMainWindows();
  static MainWindowWithDetachableDockWidgets* getFirstMainWindow();
  static MainWindowWithDetachableDockWidgets* getOrCreateWindowForScreen(
      int screen);
  static MainWindowWithDetachableDockWidgets* getOwnerOfDockWidget(
      DockWidget* dock_widget);
  static void hideAllRubberBands();
  static void setActiveDockWidget(DockWidget* dock_widget);
  static void focusNextPrevDock(DockWidget* dock_widget, bool next);
  std::set<MainWindowWithDetachableDockWidgets*> allMainWindows();
  void createHexEditTab(const QString& fileName,
                        const dbif::ObjectHandle& fileBlob);
  void createHexEditTab(const QSharedPointer<FileBlobModel>& data_model);
  void createVisualization(const QSharedPointer<FileBlobModel>& data_model);

 public slots:
  void dockLocationChanged(Qt::DockWidgetArea area);
  void tabCloseRequested(int index);
  void childAddedNotify(QObject* child);
  void updateDockWidgetTitleBars();
  void updateCloseButtonsAndIconsOnTabBars();
  void updateActiveDockWidget();
  void updateDocksAndTabs();
  void focusChanged(QWidget* old, QWidget* now);
  void delayedFocusChanged(const QPointer<QWidget>& now);

 signals:
  void childAdded(QObject* child);
  void childRemoved();
  void updateFocus(const QPointer<QWidget>& now);

 protected:
  bool event(QEvent* event) Q_DECL_OVERRIDE;
  static bool splitDockWidgetImpl(QMainWindow* main_window, QDockWidget* first,
                                  QDockWidget* second,
                                  Qt::Orientation orientation);

 private:
  static std::set<MainWindowWithDetachableDockWidgets*> main_windows_;
  static MainWindowWithDetachableDockWidgets* first_main_window_;
  static int last_created_window_id_;
  static QPointer<DockWidget> active_dock_widget_;

  TabBarEventFilter* tab_bar_event_filter_;
  QRubberBand* rubber_band_;

  bool dock_widgets_with_no_title_bars_ = false;
  bool icons_on_tabs_;
  bool mark_active_dock_widget_;
};

}  // namespace ui
}  // namespace veles
