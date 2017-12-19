/*
 * Copyright 2018 CodiLime
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

#include <QObject>
#include <QTabBar>

namespace veles {
namespace ui {

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

}  // namespace ui
}  // namespace veles
