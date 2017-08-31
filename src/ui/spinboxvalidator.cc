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
#include "include/ui/spinboxvalidator.h"

#include <cassert>

namespace veles {
namespace ui {

SpinBoxValidator::SpinBoxValidator(int base, QObject* parent)
    : QValidator(parent), base_(base), bottom_(0), top_(UINT64_MAX) {}

SpinBoxValidator::SpinBoxValidator(uint64_t minimum, uint64_t maximum, int base,
                                   QObject* parent)
    : QValidator(parent), base_(base), bottom_(minimum), top_(maximum) {}

QValidator::State SpinBoxValidator::validate(QString& input,
                                             int& /*pos*/) const {
  uint64_t value;

  if (input.isEmpty()) {
    value = 0;
  } else {
    bool ok;
    value = input.toULongLong(&ok, base_);
    if (!ok) {
      return isValidNum(input) ? Intermediate : Invalid;
    }
  }

  if (value < bottom_ || value > top_ || input != input.trimmed()) {
    return Intermediate;
  }

  return Acceptable;
}

void SpinBoxValidator::fixup(QString& input) const {
  uint64_t value;
  bool ok;

  value = input.toULongLong(&ok, base_);
  if (!ok) {
    if (isValidNum(input)) {
      input.replace(input, QString::number(top_, base_));
    }
    return;
  }

  if (value < bottom_) {
    input.replace(input, QString::number(bottom_, base_));
  } else if (value > top_) {
    input.replace(input, QString::number(top_, base_));
  } else if (input != input.trimmed()) {
    input.replace(input, input.trimmed());
  }
}

void SpinBoxValidator::setBase(int base) {
  // I wanted to handle input of numbers bigger than max uint64_t
  // but I didn't want to create regex for any possible base at this point
  assert(base == 10 || base == 16);
  base_ = base;
}

void SpinBoxValidator::setBottom(uint64_t bottom) { bottom_ = bottom; }

void SpinBoxValidator::setTop(uint64_t top) { top_ = top; }

void SpinBoxValidator::setRange(uint64_t bottom, uint64_t top) {
  bottom_ = bottom;
  top_ = top;
}

bool SpinBoxValidator::isValidNum(const QString& input) const {
  QRegularExpression re;

  re.setPatternOptions(QRegularExpression::CaseInsensitiveOption);
  if (base_ == 10) {
    re.setPattern("^ *[0-9]* *$");
  } else {
    re.setPattern("^ *[0-9A-F]* *$");
  }

  QRegularExpressionMatch match = re.match(input);
  return match.hasMatch();
}

}  // namespace ui
}  // namespace veles
