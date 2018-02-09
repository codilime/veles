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

#include <QWidget>
#include <QtWidgets/QVBoxLayout>

#include "disasm.h"
#include "row.h"

namespace veles {
namespace ui {
namespace disasm {

class Rows : public QWidget {
  Q_OBJECT

  QVBoxLayout *layout_;

 public:
  Rows();

  void generate(std::vector<std::shared_ptr<Entry>> entries);
};

}  // namespace disasm
}  // namespace ui
}  // namespace veles
