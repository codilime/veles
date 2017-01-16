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

CreateChunkDialog::CreateChunkDialog(FileBlobModel *chunksModel,
                                     QWidget *parent)
    : QDialog(parent),
      ui(new Ui::CreateChunkDialog),
      chunksModel_(chunksModel) {
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

  connect(chunksModel_, &FileBlobModel::newBinData, [this]() {
    ui->beginSpinBox->setMaximum(chunksModel_->binData().size());
    ui->endSpinBox->setMaximum(chunksModel_->binData().size());
  });
}

void CreateChunkDialog::displayPath() {
  QModelIndex index = parent_;
  QString path;
  while (index.isValid()) {
    path = index.data().toString() + "/" + path;
    index = index.parent();
  }

  path = "/" + path;
  ui->pathValueLabel->setText(path);
}

void CreateChunkDialog::setParent(const QModelIndex &parent) {
  parent_ = parent;
  displayPath();
}

void CreateChunkDialog::accept() {
  chunksModel_->addChunk(ui->nameEdit->text(), ui->typeBox->currentText(),
                         ui->commentEdit->text(), ui->beginSpinBox->value(),
                         ui->endSpinBox->value(), parent_);
  emit accepted();
  QDialog::hide();
}

void CreateChunkDialog::setRange(uint64_t begin, uint64_t end) {
  ui->beginSpinBox->setValue(begin);
  ui->endSpinBox->setValue(end);
}

}  // namespace ui
}  // namespace veles
