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
#include "include/ui/createchunkdialog.h"
#include "ui_createchunkdialog.h"

namespace veles {
namespace ui {

CreateChunkDialog::CreateChunkDialog(FileBlobModel* chunksModel,
                                     QItemSelectionModel* selectionModel,
                                     QWidget* parent)
    : QDialog(parent),
      ui(new Ui::CreateChunkDialog),
      chunksModel_(chunksModel),
      chunkSelectionModel_(selectionModel),
      useChildOfSelected_(false) {
  ui->setupUi(this);
  init();
}

CreateChunkDialog::~CreateChunkDialog() { delete ui; }

void CreateChunkDialog::init() {
  ui->typeBox->clear();
  ui->typeBox->addItem("custom");
  ui->typeBox->addItem("auto");
  ui->typeBox->addItem("pyc");
  ui->typeBox->addItem("png");
  ui->typeBox->setCurrentIndex(0);
  displayPath();
  updateBinDataSize();
  connect(chunksModel_, &FileBlobModel::newBinData, this,
          &CreateChunkDialog::updateBinDataSize);
}

QModelIndex CreateChunkDialog::parentChunk() {
  if (chunkSelectionModel_ == nullptr) {
    return {};
  }

  auto currentIndex = chunkSelectionModel_->currentIndex();
  if (useChildOfSelected_) {
    return currentIndex;
  }

  return currentIndex.parent();
}

void CreateChunkDialog::displayPath() {
  QModelIndex index = parentChunk();
  QString path;
  while (index.isValid()) {
    path = index.data().toString() + "/" + path;
    index = index.parent();
  }

  path = "/" + path;
  ui->pathValueLabel->setText(path);
}

void CreateChunkDialog::updateParent(bool childOfSelected) {
  useChildOfSelected_ = childOfSelected;
  displayPath();
}

void CreateChunkDialog::accept() {
  chunksModel_->addChunk(ui->nameEdit->text(), ui->typeBox->currentText(),
                         ui->commentEdit->text(), ui->beginSpinBox->value(),
                         ui->endSpinBox->value(), parentChunk());
  emit accepted();
  QDialog::hide();
}

void CreateChunkDialog::updateBinDataSize() {
  ui->beginSpinBox->setMaximum(
      static_cast<int>(chunksModel_->binData().size()));
  ui->endSpinBox->setMaximum(static_cast<int>(chunksModel_->binData().size()));
}

void CreateChunkDialog::setRange(uint64_t begin, uint64_t end) {
  ui->beginSpinBox->setValue(begin);
  ui->endSpinBox->setValue(end);
}

void CreateChunkDialog::showEvent(QShowEvent* /*event*/) {
  ui->nameEdit->setFocus(Qt::OtherFocusReason);
  ui->commentEdit->clear();
}

}  // namespace ui
}  // namespace veles
