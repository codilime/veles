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

#include "ui/shortcutedit.h"

#include <QKeyEvent>

namespace veles {
namespace ui {

Qt::Key ShortcutEdit::key() const { return key_; }

void ShortcutEdit::reset() {
  key_ = Qt::Key_unknown;
  clear();
}

void ShortcutEdit::keyPressEvent(QKeyEvent* event) {
  int key = event->key();
  if (key == 0 || key == Qt::Key_unknown) {
    // Unknown unhandled key pressed
    return;
  }
  if (event->modifiers() != Qt::NoModifier &&
      event->modifiers() != Qt::KeypadModifier) {
    // Modifier pressed - we can't handle it correctly so we ignore it here
    // and handle it through checkboxes
    return;
  }
  if (key == Qt::Key_Shift || key == Qt::Key_Control || key == Qt::Key_Meta ||
      key == Qt::Key_Alt || key == Qt::Key_AltGr) {
    // Only modifier pressed - it sometimes can be missed by above check (i.e.
    // meta key on Windows)
    return;
  }
  key_ = static_cast<Qt::Key>(key);
  setText(QKeySequence(key_).toString());
}

}  // namespace ui
}  // namespace veles
