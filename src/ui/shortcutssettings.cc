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

#include "ui/shortcutssettings.h"

#include <QMessageBox>

#include "util/settings/shortcuts.h"

namespace veles {
namespace ui {

using util::settings::shortcuts::ShortcutType;
using util::settings::shortcuts::ShortcutsModel;

ShortcutsDialog::ShortcutsDialog(QWidget* parent)
    : QDialog(parent,
              Qt::WindowTitleHint | Qt::WindowSystemMenuHint |
                  Qt::WindowCloseButtonHint),
      ui_(new Ui::ShortcutsDialog),
      shortcut_selection_(new ShortcutEditDialog(this)),
      model_(ShortcutsModel::getShortcutsModel()),
      filter_(new ShortcutsFilter(this)) {
  ui_->setupUi(this);
  filter_->setSourceModel(model_);
  ui_->shortcutsTree->setModel(filter_);
  ui_->shortcutsTree->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui_->shortcutsTree, &QTreeView::customContextMenuRequested, this,
          &ShortcutsDialog::onContextMenu);
  context_menu_ = new QMenu(ui_->shortcutsTree);
  connect(ui_->filter, &QLineEdit::textChanged, [this](const QString& text) {
    ui_->shortcutsTree->collapseAll();
    filter_->setFilterRegExp(
        QRegExp(text, Qt::CaseInsensitive, QRegExp::FixedString));
    if (!text.isEmpty()) {
      ui_->shortcutsTree->expandAll();
    }
  });
  connect(this, &ShortcutsDialog::accepted, [this]() {
    for (auto modified : modified_types_) {
      model_->updateShortcutsForType(modified);
    }
    modified_types_.clear();
  });
  connect(this, &ShortcutsDialog::rejected, [this]() {
    for (auto modified : modified_types_) {
      model_->resetShortcutsForType(modified);
    }
    modified_types_.clear();
  });
  connect(ui_->resetButton, &QPushButton::clicked, [this]() {
    auto reply = QMessageBox::question(
        this, "Reset all shortcuts",
        "Do you wish to reset all shortcuts to their defaults?",
        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
      model_->resetAllShortcutsToDefaults();
      modified_types_ = QSet<util::settings::shortcuts::ShortcutType>::fromList(
          model_->validShortcutTypes());
    }
  });
  connect(ui_->shortcutsTree, &QTreeView::doubleClicked, this,
          static_cast<void (ShortcutsDialog::*)(const QModelIndex&)>(
              &ShortcutsDialog::showShortcutSelectionDialog));
  auto* activate = new QAction(ui_->shortcutsTree);
  activate->setShortcuts({QKeySequence(Qt::Key_Enter),
                          QKeySequence(Qt::Key_Return),
                          QKeySequence(Qt::Key_Space)});
  activate->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  connect(activate, &QAction::triggered, [this]() {
    showShortcutSelectionDialog(ui_->shortcutsTree->currentIndex());
  });
  ui_->shortcutsTree->addAction(activate);
}

ShortcutsDialog::~ShortcutsDialog() { delete ui_; }

void ShortcutsDialog::onContextMenu(const QPoint& point) {
  QModelIndex index = ui_->shortcutsTree->indexAt(point);
  if (index.isValid()) {
    auto is_category = index.data(ShortcutsModel::CATEGORY_ROLE).toBool();
    context_menu_->clear();
    if (!is_category) {
      auto add_action = new QAction(tr("Add keyboard shortcut"), context_menu_);
      context_menu_->addAction(add_action);
      auto item_type = static_cast<ShortcutType>(
          index.data(ShortcutsModel::TYPE_ROLE).value<uint32_t>());
      auto item_shortcuts = index.data(ShortcutsModel::SHORTCUTS_ROLE)
                                .value<QList<QKeySequence>>();
      connect(add_action, &QAction::triggered,
              [this, item_type]() { showShortcutSelectionDialog(item_type); });
      auto reset_action =
          new QAction(tr("Reset shortcut to default"), context_menu_);
      context_menu_->addAction(reset_action);
      connect(reset_action, &QAction::triggered, [this, item_type]() {
        model_->resetShortcutsToDefault(item_type);
        modified_types_.insert(item_type);
      });
      context_menu_->addSeparator();
      for (const auto& shortcut : item_shortcuts) {
        auto remove_action = new QAction(
            tr("Remove %1").arg(shortcut.toString()), context_menu_);
        connect(remove_action, &QAction::triggered,
                [this, item_type, shortcut]() {
                  model_->removeShortcut(item_type, shortcut);
                  modified_types_.insert(item_type);
                });
        context_menu_->addAction(remove_action);
      }
      context_menu_->exec(ui_->shortcutsTree->mapToGlobal(point));
    }
  }
}

void ShortcutsDialog::showEvent(QShowEvent* event) {
  ui_->shortcutsTree->setColumnWidth(0, ui_->shortcutsTree->width() * 2 / 3);
  ui_->filter->clear();
  QDialog::showEvent(event);
}

void ShortcutsDialog::showShortcutSelectionDialog(ShortcutType type) {
  shortcut_selection_->reset();
  connect(shortcut_selection_, &ShortcutEditDialog::accepted, [this, type]() {
    QKeySequence sequence = shortcut_selection_->getSequence();
    if (!sequence.isEmpty()) {
      model_->addShortcut(type, sequence);
      modified_types_.insert(type);
    }
  });
  shortcut_selection_->exec();
}

void ShortcutsDialog::showShortcutSelectionDialog(const QModelIndex& index) {
  if (!index.isValid()) {
    return;
  }
  auto is_category = index.data(ShortcutsModel::CATEGORY_ROLE).toBool();
  if (is_category) {
    return;
  }
  auto type = static_cast<ShortcutType>(
      index.data(ShortcutsModel::TYPE_ROLE).value<uint32_t>());
  showShortcutSelectionDialog(type);
}

ShortcutEditDialog::ShortcutEditDialog(QWidget* parent)
    : QDialog(parent,
              Qt::WindowTitleHint | Qt::WindowSystemMenuHint |
                  Qt::WindowCloseButtonHint),
      ui(new Ui::ShortcutEditDialog) {
  ui->setupUi(this);
  ui->conflictLabel->setStyleSheet("QLabel { color : red; }");
  ui->conflictLabel->setWordWrap(true);
  connect(ui->lineEdit, &ShortcutEdit::textChanged, this,
          &ShortcutEditDialog::checkConflicts);
  connect(ui->altBox, &QCheckBox::stateChanged, this,
          &ShortcutEditDialog::checkConflicts);
  connect(ui->ctrlBox, &QCheckBox::stateChanged, this,
          &ShortcutEditDialog::checkConflicts);
  connect(ui->shiftBox, &QCheckBox::stateChanged, this,
          &ShortcutEditDialog::checkConflicts);
  connect(ui->metaBox, &QCheckBox::stateChanged, this,
          &ShortcutEditDialog::checkConflicts);
  auto accept = new QAction(this);
  accept->setShortcuts({QKeySequence(Qt::CTRL | Qt::Key_Enter),
                        QKeySequence(Qt::CTRL | Qt::Key_Return)});
  connect(accept, &QAction::triggered, this, &ShortcutEditDialog::accept);
  addAction(accept);
  auto reject = new QAction(this);
  reject->setShortcut(QKeySequence(Qt::SHIFT | Qt::Key_Escape));
  connect(reject, &QAction::triggered, this, &ShortcutEditDialog::reject);
  addAction(reject);
#ifndef Q_OS_MAC
  ui->metaBox->setVisible(false);
#endif
}

ShortcutEditDialog::~ShortcutEditDialog() { delete ui; }

void ShortcutEditDialog::reset() {
  ui->altBox->setChecked(false);
  ui->ctrlBox->setChecked(false);
  ui->shiftBox->setChecked(false);
  ui->metaBox->setChecked(false);
  ui->lineEdit->reset();
  ui->conflictLabel->clear();
  disconnect();
}

QKeySequence ShortcutEditDialog::getSequence() const {
  int key = ui->lineEdit->key();
  if (key == Qt::Key_unknown) {
    return QKeySequence();
  }
  if (ui->altBox->isChecked()) {
    key |= Qt::ALT;
  }
  if (ui->ctrlBox->isChecked()) {
    key |= Qt::CTRL;
  }
  if (ui->shiftBox->isChecked()) {
    key |= Qt::SHIFT;
  }
  if (ui->metaBox->isChecked()) {
    key |= Qt::META;
  }
  return QKeySequence(key);
}

void ShortcutEditDialog::showEvent(QShowEvent* /*event*/) {
  ui->lineEdit->setFocus();
}

void ShortcutEditDialog::checkConflicts() {
  QKeySequence result = getSequence();
  auto conflicts =
      ShortcutsModel::getShortcutsModel()->getItemsForSequence(result);
  if (!conflicts.empty()) {
    QStringList conflicts_list;
    for (auto conflict : conflicts) {
      conflicts_list.append(conflict->displayName());
    }
    ui->conflictLabel->setText(
        tr("Possible conflicts with : %1.").arg(conflicts_list.join(", ")));
  } else {
    ui->conflictLabel->clear();
  }
}

bool ShortcutsFilter::filterAcceptsRow(int source_row,
                                       const QModelIndex& source_parent) const {
  auto index = sourceModel()->index(source_row, 0, source_parent);
  if (index.data(ShortcutsModel::CATEGORY_ROLE).toBool()) {
    for (int i = 0; i < sourceModel()->rowCount(index); ++i) {
      if (filterAcceptsRow(i, index)) {
        return true;
      }
    }
    return false;
  }
  return index.data().toString().contains(filterRegExp());
}

}  // namespace ui
}  // namespace veles
