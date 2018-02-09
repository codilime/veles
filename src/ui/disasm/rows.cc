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

#include "ui/disasm/rows.h"

namespace veles {
namespace ui {
namespace disasm {

Rows::Rows() {
  layout_ = new QVBoxLayout;
  setLayout(layout_);
}

void Rows::generate(std::vector<std::shared_ptr<Entry>> entries) {
  for (const auto& entry : entries) {
    auto r = new Row(*entry.get());
    this->layout_->addWidget(r);
  }
};

}  // namespace disasm
}  // namespace ui
}  // namespace veles
