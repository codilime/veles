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
#pragma once

#include <QValidator>

namespace veles {
namespace ui {

class SpinBoxValidator : public QValidator {
 public:
  explicit SpinBoxValidator(int base = 10, QObject* parent = nullptr);
  SpinBoxValidator(uint64_t minimum, uint64_t maximum, int base = 10,
                   QObject* parent = nullptr);

  State validate(QString& input, int& pos) const override;
  void fixup(QString& input) const override;

  void setBase(int base);
  void setBottom(uint64_t bottom);
  void setTop(uint64_t top);
  void setRange(uint64_t bottom, uint64_t top);

 private:
  int base_;
  uint64_t bottom_;
  uint64_t top_;

  bool isValidNum(const QString& input) const;
};

}  // namespace ui
}  // namespace veles
