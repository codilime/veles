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

#include <QAbstractItemModel>
#include <QAction>
#include <QDataStream>
#include <QDialog>
#include <QKeySequence>
#include <QMenu>
#include <QMetaType>
#include <QPointer>
#include <QString>

namespace veles {
namespace util {
namespace settings {
namespace shortcuts {

enum ShortcutType : uint32_t {
  UNKNOWN_SHORTCUT = 0,
  EXIT_APPLICATION = 1,
  OPEN_FILE = 2,
  NEW_FILE = 3,
  SHOW_DATABASE = 4,
  SHOW_LOG = 5,
  SHOW_ABOUT = 6,
  SHOW_ABOUT_QT = 7,
  SHOW_OPTIONS = 8,
  SHOW_SHORTCUT_OPTIONS = 9,
  SHOW_CONNECT_DIALOG = 10,
  DISCONNECT_FROM_SERVER = 11,
  KILL_LOCAL_SERVER = 12,
  DOCK_MOVE_TO_TOP = 13,
  DOCK_MOVE_TO_TOP_MAX = 14,
  DOCK_SPLIT_HORIZ = 15,
  DOCK_SPLIT_VERT = 16,
  CREATE_CHUNK = 17,
  CREATE_CHILD_CHUNK = 18,
  GO_TO_ADDRESS = 19,
  REMOVE_CHUNK = 20,
  SAVE_SELECTION_TO_FILE = 21,
  HEX_FIND = 22,
  HEX_FIND_NEXT = 23,
  OPEN_VISUALIZATION = 24,
  OPEN_HEX = 25,
  SHOW_NODE_TREE = 26,
  SHOW_MINIMAP = 27,
  VISUALIZATION_DIGRAM = 28,
  VISUALIZATION_TRIGRAM = 29,
  VISUALIZATION_LAYERED_DIGRAM = 30,
  VISUALIZATION_OPTIONS = 31,
  TRIGRAM_CUBE = 32,
  TRIGRAM_CYLINDER = 33,
  TRIGRAM_SPHERE = 34,
  VISUALIZATION_MANIPULATOR_SPIN = 35,
  VISUALIZATION_MANIPULATOR_TRACKBALL = 36,
  VISUALIZATION_MANIPULATOR_FREE = 37,
  COPY = 38,
  PASTE = 39,
  UPLOAD = 40,
  UNDO = 41,
  DISCARD = 42,
  HEX_MOVE_TO_NEXT_CHAR = 43,
  HEX_MOVE_TO_PREV_CHAR = 44,
  HEX_MOVE_TO_NEXT_LINE = 45,
  HEX_MOVE_TO_PREV_LINE = 46,
  HEX_MOVE_TO_NEXT_PAGE = 47,
  HEX_MOVE_TO_PREV_PAGE = 48,
  HEX_MOVE_TO_START_OF_LINE = 49,
  HEX_MOVE_TO_END_OF_LINE = 50,
  HEX_MOVE_TO_START_OF_FILE = 51,
  HEX_MOVE_TO_END_OF_FILE = 52,
  HEX_MOVE_TO_NEXT_DIGIT = 53,
  HEX_MOVE_TO_PREV_DIGIT = 54,
  HEX_REMOVE_SELECTION = 55,
  HEX_SELECT_ALL = 56,
  HEX_SELECT_NEXT_CHAR = 57,
  HEX_SELECT_PREV_CHAR = 58,
  HEX_SELECT_NEXT_LINE = 59,
  HEX_SELECT_PREV_LINE = 60,
  HEX_SELECT_NEXT_PAGE = 61,
  HEX_SELECT_PREV_PAGE = 62,
  HEX_SELECT_START_OF_LINE = 63,
  HEX_SELECT_END_OF_LINE = 64,
  HEX_SELECT_START_OF_DOCUMENT = 65,
  HEX_SELECT_END_OF_DOCUMENT = 66,
  DOCK_CLOSE = 67,
  SWITCH_TAB_NEXT = 68,
  SWITCH_TAB_PREV = 69,
  HEX_SCROLL_UP = 70,
  HEX_SCROLL_DOWN = 71,
  HEX_ADD_COLUMN = 72,
  HEX_REMOVE_COLUMN = 73,
  SELECT_CHUNK = 74,
  SAVE_CHUNK_TO_FILE = 75,
};

QMap<ShortcutType, QList<QKeySequence>> defaultShortcuts();
QList<QKeySequence> getShortcuts(ShortcutType type);
void setShortcuts(ShortcutType type, const QList<QKeySequence>& shortcuts);

class ShortcutsItem {
 public:
  explicit ShortcutsItem(const QString& display_name = QString(""),
                         ShortcutsItem* parent = nullptr);
  ShortcutsItem(const QString& display_name, ShortcutsItem* parent,
                util::settings::shortcuts::ShortcutType type);
  ShortcutsItem(const QString& name, const QString& display_name,
                ShortcutsItem* parent,
                util::settings::shortcuts::ShortcutType type);
  ~ShortcutsItem();

  QAction* createQAction(QObject* parent,
                         Qt::ShortcutContext context = Qt::WindowShortcut);
  QAction* createQAction(QObject* parent, const QIcon& icon,
                         Qt::ShortcutContext context = Qt::WindowShortcut);
  QString name() const;
  QString displayName() const;
  ShortcutsItem* parent() const;
  QList<QKeySequence> shortcuts() const;
  QString displayShortcuts() const;
  bool deleteShortcut(const QKeySequence& shortcut);
  bool addShortcut(const QKeySequence& shortcut);
  void setConflict(bool conflict);
  bool hasConflict() const;
  util::settings::shortcuts::ShortcutType type() const;
  void updateShortcutsForActions();
  bool isCategory() const;

  QList<ShortcutsItem*> children;

 private:
  QList<QPointer<QAction>> actions_;
  QList<QKeySequence> shortcuts_;
  QString display_shortcuts_;
  QString name_;
  QString display_name_;
  ShortcutsItem* parent_;
  bool conflict_ = false;
  util::settings::shortcuts::ShortcutType type_;
  bool is_category_;
};

class ShortcutsModel : public QAbstractItemModel {
 public:
  static const int COLUMN_INDEX_NAME = 0;
  static const int COLUMN_INDEX_SHORTCUTS = 1;
  static const int CATEGORY_ROLE = Qt::UserRole;
  static const int TYPE_ROLE = Qt::UserRole + 1;
  static const int SHORTCUTS_ROLE = Qt::UserRole + 2;

  ~ShortcutsModel() override;

  QModelIndex index(int row, int column,
                    const QModelIndex& parent) const override;
  QModelIndex parent(const QModelIndex& index) const override;
  int rowCount(const QModelIndex& parent) const override;
  int columnCount(const QModelIndex& parent) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role) const override;
  static ShortcutsModel* getShortcutsModel();
  QAction* createQAction(util::settings::shortcuts::ShortcutType type,
                         QObject* parent, Qt::ShortcutContext context);
  QAction* createQAction(util::settings::shortcuts::ShortcutType type,
                         QObject* parent, const QIcon& icon,
                         Qt::ShortcutContext context);
  void addShortcut(util::settings::shortcuts::ShortcutType type,
                   const QKeySequence& shortcut);
  void removeShortcut(util::settings::shortcuts::ShortcutType type,
                      const QKeySequence& shortcut);
  ShortcutsItem* itemFromIndex(const QModelIndex& index) const;
  QModelIndex indexFromItem(ShortcutsItem* item) const;
  QList<ShortcutsItem*> getItemsForSequence(const QKeySequence& sequence);
  void updateShortcutsForType(util::settings::shortcuts::ShortcutType type);
  void resetShortcutsForType(util::settings::shortcuts::ShortcutType type);
  void resetShortcutsToDefault(util::settings::shortcuts::ShortcutType type);
  void resetAllShortcutsToDefaults();
  QList<util::settings::shortcuts::ShortcutType> validShortcutTypes() const;

 private:
  ShortcutsModel();

  ShortcutsItem* addCategory(const QString& name, ShortcutsItem* parent);
  ShortcutsItem* addShortcutType(util::settings::shortcuts::ShortcutType type,
                                 ShortcutsItem* parent, const QString& name,
                                 const QString& display_name = QString(""));
  bool checkIfConflicts(ShortcutsItem* item) const;

  ShortcutsItem* root_;
  QMap<util::settings::shortcuts::ShortcutType, ShortcutsItem*>
      type_to_shortcut_;
  QMap<QKeySequence, QList<ShortcutsItem*>> sequence_to_shortcut_;
};

}  // namespace shortcuts
}  // namespace settings
}  // namespace util
}  // namespace veles

Q_DECLARE_METATYPE(veles::util::settings::shortcuts::ShortcutType)
