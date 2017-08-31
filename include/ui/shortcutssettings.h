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
#pragma once

#include <QSortFilterProxyModel>

#include "ui_shortcutselection.h"
#include "ui_shortcutssettings.h"
#include "util/settings/shortcuts.h"

namespace veles {
namespace ui {

class ShortcutsFilter : public QSortFilterProxyModel {
  using QSortFilterProxyModel::QSortFilterProxyModel;

 protected:
  bool filterAcceptsRow(int source_row,
                        const QModelIndex& source_parent) const override;
};

class ShortcutEditDialog : public QDialog {
  Q_OBJECT

 public:
  explicit ShortcutEditDialog(QWidget* parent = nullptr);
  ~ShortcutEditDialog() override;
  void reset();
  QKeySequence getSequence() const;

 protected:
  void showEvent(QShowEvent* event) override;

 private:
  Ui::ShortcutEditDialog* ui;

 private slots:
  void checkConflicts();
};

class ShortcutsDialog : public QDialog {
  Q_OBJECT

 public:
  explicit ShortcutsDialog(QWidget* parent = nullptr);
  ~ShortcutsDialog() override;

 public slots:
  void onContextMenu(const QPoint& point);

 protected:
  void showEvent(QShowEvent* event) override;

 private:
  Ui::ShortcutsDialog* ui_;
  QMenu* context_menu_;
  ShortcutEditDialog* shortcut_selection_;
  QSet<util::settings::shortcuts::ShortcutType> modified_types_;
  util::settings::shortcuts::ShortcutsModel* model_;
  ShortcutsFilter* filter_;

  void showShortcutSelectionDialog(const QModelIndex& index);

 private slots:
  void showShortcutSelectionDialog(
      util::settings::shortcuts::ShortcutType type);
};

}  // namespace ui
}  // namespace veles
