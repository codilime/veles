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
#include <QDockWidget>
#include <QTabBar>

#include "ui/dockwidget_native.h"

namespace veles {
namespace ui {

#ifdef Q_OS_WIN

/*****************************************************************************/
/* Native Windows events */
/*****************************************************************************/

#include <windows.h>
#include <winuser.h>

void stopTabBarDragging(QTabBar* object) {
  INPUT event;
  event.type = INPUT_MOUSE;
  event.mi.dx = 0;
  event.mi.dy = 0;
  event.mi.mouseData = 0;
  event.mi.dwFlags = MOUSEEVENTF_LEFTUP;
  event.mi.time = 0;
  SendInput(1, &event, sizeof(INPUT));
}

void startDockDragging(QDockWidget* object) {
  INPUT event;
  event.type = INPUT_MOUSE;
  event.mi.dx = 0;
  event.mi.dy = 0;
  event.mi.mouseData = 0;
  event.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
  event.mi.time = 0;
  SendInput(1, &event, sizeof(INPUT));
}

#else

/*****************************************************************************/
/* Other platforms - not supported yet... */
/*****************************************************************************/

void stopTabBarDragging(QTabBar* object) {}
void startDockDragging(QDockWidget* object) {}

#endif

} // namespace ui
} // namespace veles
