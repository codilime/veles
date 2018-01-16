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

#include <QApplication>

namespace veles {
namespace ui {

// A modified QApplication which stores an additional global state.
class VelesApplication : public QApplication {
  Q_OBJECT

 public:
  using QApplication::QApplication;

  static VelesApplication* instance() {
    // This is always a proper cast, because there's only one instance of
    // `QCoreApplication` (created in `main`) which is `VelesApplication`.
    return static_cast<VelesApplication*>(QCoreApplication::instance());
  }

 signals:
  void settingsChanged();
};

}  // namespace ui
}  // namespace veles
