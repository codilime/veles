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

#include <QSizePolicy>

#include "ui/disasm/row.h"

/**
 * Zdebugować to wszystko :)
 */

namespace veles {
namespace ui {
namespace disasm {

Row::Row() {
  //    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  layout_ = new QHBoxLayout();
  layout_->setSpacing(0);
  layout_->setMargin(0);

  text_ = new QLabel("Dummy text");

  layout_->addWidget(text_);

  setLayout(layout_);
}

void Row::toggleColumn(Row::ColumnName column_name) {}

}  // namespace disasm
}  // namespace ui
}  // namespace veles
