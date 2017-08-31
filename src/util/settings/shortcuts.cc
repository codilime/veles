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
#include "util/settings/shortcuts.h"

#include <cassert>

#include <QSettings>

#include "util/settings/theme.h"

namespace veles {
namespace util {
namespace settings {
namespace shortcuts {

QList<QKeySequence> getShortcuts(ShortcutType type) {
  QSettings settings;
  QSettings::SettingsMap shortcuts =
      settings.value("keyboard_shortcuts").toMap();
  if (shortcuts.contains(QString(type))) {
    // This repacking is done instead of directly getting proper type
    // from QVariant because it sometimes silently fails on macOS
    QList<QVariant> var_list = shortcuts.value(QString(type)).toList();
    QList<QKeySequence> res_list;
    for (const auto& var : var_list) {
      res_list.append(var.value<QKeySequence>());
    }
    return res_list;
  }
  return defaultShortcuts().value(type);
}

void setShortcuts(ShortcutType type, const QList<QKeySequence>& shortcuts) {
  QSettings settings;
  QSettings::SettingsMap shortcuts_saved =
      settings.value("keyboard_shortcuts").toMap();
  QList<QVariant> list;
  // This repacking is done instead of directly saving the list, because it
  // sometimes silently fails on macOS
  for (const auto& shrt : shortcuts) {
    list.append(QVariant(shrt));
  }
  shortcuts_saved[QString(type)] = list;
  settings.setValue("keyboard_shortcuts", shortcuts_saved);
}

QMap<ShortcutType, QList<QKeySequence>> defaultShortcuts() {
  static QMap<ShortcutType, QList<QKeySequence>> defaults;
  if (defaults.empty()) {
    defaults[EXIT_APPLICATION] = QKeySequence::keyBindings(QKeySequence::Quit);
    defaults[OPEN_FILE] = QKeySequence::keyBindings(QKeySequence::Open);
    defaults[NEW_FILE] = QKeySequence::keyBindings(QKeySequence::New);
    defaults[SHOW_OPTIONS] = {QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_K)};
    defaults[DOCK_MOVE_TO_TOP_MAX] = {QKeySequence(Qt::Key_F12)};
    defaults[GO_TO_ADDRESS] = {QKeySequence(Qt::CTRL | Qt::Key_G)};
    defaults[REMOVE_CHUNK] = {QKeySequence(Qt::Key_Delete)};
    defaults[HEX_FIND] = QKeySequence::keyBindings(QKeySequence::Find);
    defaults[HEX_FIND_NEXT] = {QKeySequence(Qt::Key_F3)};
    defaults[OPEN_VISUALIZATION] = {
        QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_V)};
    defaults[OPEN_HEX] = {QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_H)};
    defaults[VISUALIZATION_DIGRAM] = {QKeySequence(Qt::Key_1)};
    defaults[VISUALIZATION_TRIGRAM] = {QKeySequence(Qt::Key_2)};
    defaults[VISUALIZATION_LAYERED_DIGRAM] = {QKeySequence(Qt::Key_3)};
    defaults[VISUALIZATION_MANIPULATOR_SPIN] = {
        QKeySequence(Qt::CTRL | Qt::Key_1), QKeySequence(Qt::Key_Escape)};
    defaults[VISUALIZATION_MANIPULATOR_TRACKBALL] = {
        QKeySequence(Qt::CTRL + Qt::Key_2)};
    defaults[VISUALIZATION_MANIPULATOR_FREE] = {
        QKeySequence(Qt::CTRL + Qt::Key_3)};
    defaults[COPY] = QKeySequence::keyBindings(QKeySequence::Copy);
    defaults[PASTE] = QKeySequence::keyBindings(QKeySequence::Paste);
    defaults[UPLOAD] = QKeySequence::keyBindings(QKeySequence::Save);
    defaults[UNDO] = QKeySequence::keyBindings(QKeySequence::Undo);
    defaults[HEX_MOVE_TO_NEXT_CHAR] =
        QKeySequence::keyBindings(QKeySequence::MoveToNextChar);
    defaults[HEX_MOVE_TO_PREV_CHAR] =
        QKeySequence::keyBindings(QKeySequence::MoveToPreviousChar);
    defaults[HEX_MOVE_TO_NEXT_LINE] =
        QKeySequence::keyBindings(QKeySequence::MoveToNextLine);
    defaults[HEX_MOVE_TO_PREV_LINE] =
        QKeySequence::keyBindings(QKeySequence::MoveToPreviousLine);
    defaults[HEX_MOVE_TO_NEXT_PAGE] =
        QKeySequence::keyBindings(QKeySequence::MoveToNextPage);
    defaults[HEX_MOVE_TO_PREV_PAGE] =
        QKeySequence::keyBindings(QKeySequence::MoveToPreviousPage);
    defaults[HEX_MOVE_TO_START_OF_LINE] =
        QKeySequence::keyBindings(QKeySequence::MoveToStartOfLine);
    defaults[HEX_MOVE_TO_END_OF_LINE] =
        QKeySequence::keyBindings(QKeySequence::MoveToEndOfLine);
    defaults[HEX_MOVE_TO_START_OF_FILE] =
        QKeySequence::keyBindings(QKeySequence::MoveToStartOfDocument);
    defaults[HEX_MOVE_TO_END_OF_FILE] =
        QKeySequence::keyBindings(QKeySequence::MoveToEndOfDocument);
    defaults[HEX_MOVE_TO_NEXT_DIGIT] =
        QKeySequence::keyBindings(QKeySequence::MoveToNextWord);
    defaults[HEX_MOVE_TO_PREV_DIGIT] =
        QKeySequence::keyBindings(QKeySequence::MoveToPreviousWord);
    defaults[HEX_REMOVE_SELECTION] = {QKeySequence(Qt::Key_Escape)};
    defaults[HEX_SELECT_ALL] =
        QKeySequence::keyBindings(QKeySequence::SelectAll);
    defaults[HEX_SELECT_NEXT_CHAR] =
        QKeySequence::keyBindings(QKeySequence::SelectNextChar);
    defaults[HEX_SELECT_PREV_CHAR] =
        QKeySequence::keyBindings(QKeySequence::SelectPreviousChar);
    defaults[HEX_SELECT_NEXT_LINE] =
        QKeySequence::keyBindings(QKeySequence::SelectNextLine);
    defaults[HEX_SELECT_PREV_LINE] =
        QKeySequence::keyBindings(QKeySequence::SelectPreviousLine);
    defaults[HEX_SELECT_NEXT_PAGE] =
        QKeySequence::keyBindings(QKeySequence::SelectNextPage);
    defaults[HEX_SELECT_PREV_PAGE] =
        QKeySequence::keyBindings(QKeySequence::SelectPreviousPage);
    defaults[HEX_SELECT_START_OF_LINE] =
        QKeySequence::keyBindings(QKeySequence::SelectStartOfLine);
    defaults[HEX_SELECT_END_OF_LINE] =
        QKeySequence::keyBindings(QKeySequence::SelectEndOfLine);
    defaults[HEX_SELECT_START_OF_DOCUMENT] =
        QKeySequence::keyBindings(QKeySequence::SelectStartOfDocument);
    defaults[HEX_SELECT_END_OF_DOCUMENT] =
        QKeySequence::keyBindings(QKeySequence::SelectEndOfDocument);
    defaults[DOCK_CLOSE] = QKeySequence::keyBindings(QKeySequence::Close);
    defaults[SWITCH_TAB_NEXT] = {QKeySequence(Qt::CTRL | Qt::Key_Tab),
                                 QKeySequence(Qt::CTRL | Qt::Key_PageDown)};
    defaults[SWITCH_TAB_PREV] = {
        QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Tab),
        QKeySequence(Qt::CTRL | Qt::Key_PageUp)};
    defaults[HEX_SCROLL_UP] = {QKeySequence(Qt::CTRL | Qt::Key_Up)};
    defaults[HEX_SCROLL_DOWN] = {QKeySequence(Qt::CTRL | Qt::Key_Down)};
    defaults[HEX_ADD_COLUMN] = {QKeySequence(Qt::Key_Plus),
                                QKeySequence(Qt::Key_Equal)};
    defaults[HEX_REMOVE_COLUMN] = {QKeySequence(Qt::Key_Minus)};
  }
  return defaults;
}

ShortcutsItem::ShortcutsItem(const QString& display_name, ShortcutsItem* parent)
    : display_name_(display_name), parent_(parent), is_category_(true) {}

ShortcutsItem::ShortcutsItem(const QString& display_name, ShortcutsItem* parent,
                             ShortcutType type)
    : name_(display_name),
      display_name_(display_name),
      parent_(parent),
      type_(type),
      is_category_(false) {}

ShortcutsItem::ShortcutsItem(const QString& name, const QString& display_name,
                             ShortcutsItem* parent, ShortcutType type)
    : name_(name),
      display_name_(display_name),
      parent_(parent),
      type_(type),
      is_category_(false) {}

ShortcutsItem::~ShortcutsItem() { qDeleteAll(children); }

QAction* ShortcutsItem::createQAction(QObject* parent,
                                      Qt::ShortcutContext context) {
  auto ptr = new QAction(name_, parent);
  ptr->setShortcuts(shortcuts_);
  ptr->setShortcutContext(context);
  actions_.append(ptr);
  return ptr;
}

QAction* ShortcutsItem::createQAction(QObject* parent, const QIcon& icon,
                                      Qt::ShortcutContext context) {
  auto ptr = new QAction(icon, name_, parent);
  ptr->setShortcuts(shortcuts_);
  ptr->setShortcutContext(context);
  actions_.append(ptr);
  return ptr;
}

QString ShortcutsItem::name() const { return name_; }

QString ShortcutsItem::displayName() const { return display_name_; }

ShortcutsItem* ShortcutsItem::parent() const { return parent_; }

QList<QKeySequence> ShortcutsItem::shortcuts() const { return shortcuts_; }

QString ShortcutsItem::displayShortcuts() const { return display_shortcuts_; }

bool ShortcutsItem::deleteShortcut(const QKeySequence& shortcut) {
  bool ret = shortcuts_.removeAll(shortcut) != 0;
  display_shortcuts_ = QKeySequence::listToString(shortcuts_);
  return ret;
}

bool ShortcutsItem::addShortcut(const QKeySequence& shortcut) {
  if (!shortcuts_.contains(shortcut)) {
    shortcuts_.append(shortcut);
    display_shortcuts_ = QKeySequence::listToString(shortcuts_);
    return true;
  }
  return false;
}

void ShortcutsItem::setConflict(bool conflict) { conflict_ = conflict; }

bool ShortcutsItem::hasConflict() const { return conflict_; }

ShortcutType ShortcutsItem::type() const { return type_; }

void ShortcutsItem::updateShortcutsForActions() {
  QMutableListIterator<QPointer<QAction>> it(actions_);
  while (it.hasNext()) {
    auto action = it.next();
    if (action != nullptr) {
      action->setShortcuts(shortcuts_);
    } else {
      it.remove();
    }
  }
}

bool ShortcutsItem::isCategory() const { return is_category_; }

QModelIndex ShortcutsModel::index(int row, int column,
                                  const QModelIndex& parent) const {
  if (!hasIndex(row, column, parent)) {
    return {};
  }
  ShortcutsItem* parentItem = itemFromIndex(parent);
  ShortcutsItem* child = parentItem->children.value(row);
  if (child != nullptr) {
    return createIndex(row, column, child);
  }
  return {};
}

QModelIndex ShortcutsModel::parent(const QModelIndex& index) const {
  if (!index.isValid()) {
    return {};
  }

  auto* child = static_cast<ShortcutsItem*>(index.internalPointer());
  ShortcutsItem* parentItem = child->parent();

  if (parentItem == root_) {
    return {};
  }
  return createIndex(parentItem->children.indexOf(child), 0, parentItem);
}

int ShortcutsModel::rowCount(const QModelIndex& parent) const {
  if (parent.column() > 0) {
    return 0;
  }
  ShortcutsItem* parentItem = itemFromIndex(parent);
  return parentItem->children.size();
}

int ShortcutsModel::columnCount(const QModelIndex& /*parent*/) const {
  // Action name and shortcuts
  return 2;
}

QVariant ShortcutsModel::data(const QModelIndex& index, int role) const {
  // TODO(mkow): proper icon
  static const QIcon category_ico(":/images/open.png");

  if (!index.isValid()) {
    return QVariant();
  }

  auto* item = static_cast<ShortcutsItem*>(index.internalPointer());

  switch (role) {
    case Qt::DisplayRole:
      switch (index.column()) {
        case COLUMN_INDEX_NAME:
          return item->displayName();
        case COLUMN_INDEX_SHORTCUTS:
          return item->displayShortcuts();
        default:
          return QVariant();
      }
    case Qt::ForegroundRole:
      if (item->hasConflict()) {
        return QColor(Qt::red);
      }
      return util::settings::theme::pallete().color(QPalette::Text);
    case Qt::DecorationRole:
      if (item->isCategory() && index.column() == COLUMN_INDEX_NAME) {
        return category_ico;
      } else {
        return QVariant();
      }
    case CATEGORY_ROLE:
      return item->isCategory();
    case TYPE_ROLE:
      return item->type();
    case SHORTCUTS_ROLE:
      return QVariant::fromValue(item->shortcuts());
    default:
      return QVariant();
  }
}

QVariant ShortcutsModel::headerData(int section, Qt::Orientation orientation,
                                    int role) const {
  if (orientation != Qt::Orientation::Horizontal || role != Qt::DisplayRole) {
    return QVariant();
  }

  switch (section) {
    case COLUMN_INDEX_NAME:
      return "Name";
    case COLUMN_INDEX_SHORTCUTS:
      return "Shortcuts";
    default:
      return QVariant();
  }
}

ShortcutsModel* ShortcutsModel::getShortcutsModel() {
  static auto* ptr = new ShortcutsModel();
  return ptr;
}

QAction* ShortcutsModel::createQAction(ShortcutType type, QObject* parent,
                                       Qt::ShortcutContext context) {
  return type_to_shortcut_[type]->createQAction(parent, context);
}

QAction* ShortcutsModel::createQAction(ShortcutType type, QObject* parent,
                                       const QIcon& icon,
                                       Qt::ShortcutContext context) {
  return type_to_shortcut_[type]->createQAction(parent, icon, context);
}

void ShortcutsModel::addShortcut(ShortcutType type,
                                 const QKeySequence& shortcut) {
  if (shortcut.isEmpty()) {
    return;
  }
  auto item = type_to_shortcut_.value(type);
  if (item == nullptr) {
    return;
  }
  if (item->addShortcut(shortcut)) {
    if (sequence_to_shortcut_.contains(shortcut) &&
        !sequence_to_shortcut_[shortcut].contains(item)) {
      if (!sequence_to_shortcut_[shortcut].empty()) {
        if (sequence_to_shortcut_[shortcut].size() == 1) {
          sequence_to_shortcut_[shortcut][0]->setConflict(true);
        }
        item->setConflict(true);
      }
    }
    sequence_to_shortcut_[shortcut].append(item);
    auto index = indexFromItem(item);
    emit dataChanged(index, index);
  }
}

void ShortcutsModel::removeShortcut(ShortcutType type,
                                    const QKeySequence& shortcut) {
  if (shortcut.isEmpty()) {
    return;
  }
  auto item = type_to_shortcut_.value(type);
  if (item == nullptr) {
    return;
  }
  if (item->deleteShortcut(shortcut)) {
    sequence_to_shortcut_[shortcut].removeAll(item);
    if (sequence_to_shortcut_[shortcut].size() == 1 &&
        !checkIfConflicts(sequence_to_shortcut_[shortcut][0])) {
      sequence_to_shortcut_[shortcut][0]->setConflict(false);
    }
    if (!checkIfConflicts(item)) {
      item->setConflict(false);
    }
    auto index = indexFromItem(item);
    emit dataChanged(index, index);
  }
}

ShortcutsModel::ShortcutsModel() : root_(new ShortcutsItem()) {
  auto global = addCategory(tr("Global"), root_);
  addShortcutType(EXIT_APPLICATION, global, tr("E&xit"), tr("Exit"));
  addShortcutType(OPEN_FILE, global, tr("&Open..."), tr("Open file"));
  //  Uncomment once supported
  //  addShortcutType(NEW_FILE, global, tr("&New..."), tr("New File"));
  addShortcutType(SHOW_DATABASE, global, tr("Show database view"));
  addShortcutType(SHOW_LOG, global, tr("Show log view"));
  addShortcutType(SHOW_OPTIONS, global, tr("&Options"),
                  tr("Show the dialog to select applications options"));
  addShortcutType(SHOW_SHORTCUT_OPTIONS, global, tr("Keyboard shortcuts"),
                  tr("Show the dialog to customize keyboard shortcuts"));

  auto docks = addCategory(tr("Docks"), global);
  addShortcutType(DOCK_MOVE_TO_TOP, docks, tr("Move to new top level window"));
  addShortcutType(DOCK_MOVE_TO_TOP_MAX, docks,
                  tr("Move to new top level window and maximize"));
  addShortcutType(DOCK_SPLIT_HORIZ, docks, tr("Split horizontally"));
  addShortcutType(DOCK_SPLIT_VERT, docks, tr("Split vertically"));
  addShortcutType(DOCK_CLOSE, docks, tr("Close current dock"));
  addShortcutType(SWITCH_TAB_NEXT, docks, tr("Switch to the next tab"));
  addShortcutType(SWITCH_TAB_PREV, docks, tr("Switch to the previous tab"));

  addShortcutType(OPEN_VISUALIZATION, global, tr("&Visualization"),
                  tr("Open visualization for current file"));
  addShortcutType(OPEN_HEX, global, tr("Show &hex editor"),
                  tr("Open hex editor for current file"));
  addShortcutType(SHOW_NODE_TREE, global, tr("&Node tree"),
                  tr("Open/close node tree for current dock"));
  addShortcutType(SHOW_MINIMAP, global, tr("&Minimap"),
                  tr("Open/close minimap for current dock"));
  addShortcutType(COPY, global, tr("&Copy"), tr("Copy"));
  addShortcutType(PASTE, global, tr("&Paste"), tr("Paste"));

  auto network = addCategory(tr("Network"), root_);
  addShortcutType(SHOW_CONNECT_DIALOG, network, tr("Connect..."),
                  tr("Show connect dialog"));
  addShortcutType(DISCONNECT_FROM_SERVER, network, tr("Disconnect"),
                  tr("Disconnect from current server"));
  addShortcutType(KILL_LOCAL_SERVER, network,
                  tr("Kill locally created server"));

  auto misc = addCategory(tr("Miscellaneous"), root_);
  addShortcutType(SHOW_ABOUT, misc, tr("&About"),
                  tr("Show the application's About box"));
  addShortcutType(SHOW_ABOUT_QT, misc, tr("About &Qt"),
                  tr("Show the Qt library's About box"));

  auto hex = addCategory(tr("HexEdit"), root_);
  addShortcutType(CREATE_CHUNK, hex, tr("&Create chunk"), tr("Create chunk"));
  addShortcutType(CREATE_CHILD_CHUNK, hex, tr("&Create child chunk"),
                  tr("Create child chunk"));
  addShortcutType(GO_TO_ADDRESS, hex, tr("&Go to address"),
                  tr("Go to address"));
  addShortcutType(REMOVE_CHUNK, hex, tr("Remove chunk"));
  addShortcutType(SELECT_CHUNK, hex, tr("Select chunk"), tr("Select chunk"));
  addShortcutType(SAVE_CHUNK_TO_FILE, hex, tr("Save chunk to file"),
                  tr("Save chunk to file"));
  addShortcutType(SAVE_SELECTION_TO_FILE, hex, tr("&Save selection to file"),
                  tr("Save selection to file"));
  addShortcutType(HEX_FIND, hex, tr("&Find/Replace"),
                  tr("Show the dialog for finding and replacing"));
  addShortcutType(HEX_FIND_NEXT, hex, tr("Find &next"), tr("Find next"));
  addShortcutType(UPLOAD, hex, tr("&Upload"), tr("Upload"));
  addShortcutType(UNDO, hex, tr("&Undo"), tr("Undo"));
  addShortcutType(DISCARD, hex, tr("&Discard"), tr("Discard changes"));
  addShortcutType(HEX_ADD_COLUMN, hex, tr("Add column"), tr("Add column"));
  addShortcutType(HEX_REMOVE_COLUMN, hex, tr("Remove column"),
                  tr("Remove column"));

  auto hex_move = addCategory(tr("Cursor movement"), hex);
  addShortcutType(HEX_MOVE_TO_NEXT_CHAR, hex_move, tr("Right"));
  addShortcutType(HEX_MOVE_TO_PREV_CHAR, hex_move, tr("Left"));
  addShortcutType(HEX_MOVE_TO_NEXT_LINE, hex_move, tr("Down"));
  addShortcutType(HEX_MOVE_TO_PREV_LINE, hex_move, tr("Up"));
  addShortcutType(HEX_SCROLL_UP, hex_move, tr("Scroll up one line"));
  addShortcutType(HEX_SCROLL_DOWN, hex_move, tr("Scroll down one line"));
  addShortcutType(HEX_MOVE_TO_NEXT_PAGE, hex_move, tr("Page down"));
  addShortcutType(HEX_MOVE_TO_PREV_PAGE, hex_move, tr("Page up"));
  addShortcutType(HEX_MOVE_TO_START_OF_LINE, hex_move, tr("Start of line"));
  addShortcutType(HEX_MOVE_TO_END_OF_LINE, hex_move, tr("End of line"));
  addShortcutType(HEX_MOVE_TO_NEXT_DIGIT, hex_move, tr("Next digit"));
  addShortcutType(HEX_MOVE_TO_PREV_DIGIT, hex_move, tr("Previous digit"));
  addShortcutType(HEX_MOVE_TO_START_OF_FILE, hex_move, tr("Start of file"));
  addShortcutType(HEX_MOVE_TO_END_OF_FILE, hex_move, tr("End of file"));
  addShortcutType(HEX_REMOVE_SELECTION, hex_move, tr("Remove selection"));
  addShortcutType(HEX_SELECT_ALL, hex_move, tr("Select all"));
  addShortcutType(HEX_SELECT_NEXT_CHAR, hex_move, tr("Right with selecton"));
  addShortcutType(HEX_SELECT_PREV_CHAR, hex_move, tr("Left with selection"));
  addShortcutType(HEX_SELECT_NEXT_LINE, hex_move, tr("Down with selection"));
  addShortcutType(HEX_SELECT_PREV_LINE, hex_move, tr("Up with selection"));
  addShortcutType(HEX_SELECT_NEXT_PAGE, hex_move,
                  tr("Page down with selection"));
  addShortcutType(HEX_SELECT_PREV_PAGE, hex_move, tr("Page up with selection"));
  addShortcutType(HEX_SELECT_START_OF_LINE, hex_move,
                  tr("Start of line with selection"));
  addShortcutType(HEX_SELECT_END_OF_LINE, hex_move,
                  tr("End of line with selection"));
  addShortcutType(HEX_SELECT_START_OF_DOCUMENT, hex_move,
                  tr("Start of file with selection"));
  addShortcutType(HEX_SELECT_END_OF_DOCUMENT, hex_move,
                  tr("End of file with selection"));

  auto visualization = addCategory(tr("Visualization"), root_);
  addShortcutType(VISUALIZATION_DIGRAM, visualization, tr("&Digram"),
                  tr("Change visualizaton mode to digram"));
  addShortcutType(VISUALIZATION_TRIGRAM, visualization, tr("&Trigram"),
                  tr("Change visualizaton mode to trigram"));
  addShortcutType(VISUALIZATION_LAYERED_DIGRAM, visualization,
                  tr("&Layered Digram"),
                  tr("Change visualizaton mode to layered digram"));
  addShortcutType(VISUALIZATION_OPTIONS, visualization, tr("More options"),
                  tr("Show visualization options"));

  auto three_d = addCategory(tr("3D Visualization"), visualization);
  addShortcutType(TRIGRAM_CUBE, three_d,
                  tr("Change 3D visualization display mode to cube"));
  addShortcutType(TRIGRAM_CYLINDER, three_d,
                  tr("Change 3D visualization display mode to cylinder"));
  addShortcutType(TRIGRAM_SPHERE, three_d,
                  tr("Change 3D visualization display mode to sphere"));
  addShortcutType(VISUALIZATION_MANIPULATOR_SPIN, three_d,
                  tr("Spin manipulator"), tr("Switch to spin manipulator"));
  addShortcutType(VISUALIZATION_MANIPULATOR_TRACKBALL, three_d,
                  tr("Trackball manipulator"),
                  tr("Switch to trackball manipulator"));
  addShortcutType(VISUALIZATION_MANIPULATOR_FREE, three_d,
                  tr("Free manipulator"), tr("Switch to free manipulator"));
}

ShortcutsItem* ShortcutsModel::addCategory(const QString& name,
                                           ShortcutsItem* parent) {
  auto item = new ShortcutsItem(name, parent);
  parent->children.append(item);
  return item;
}

ShortcutsItem* ShortcutsModel::addShortcutType(ShortcutType type,
                                               ShortcutsItem* parent,
                                               const QString& name,
                                               const QString& display_name) {
  ShortcutsItem* item;
  if (display_name.isEmpty()) {
    item = new ShortcutsItem(name, parent, type);
  } else {
    item = new ShortcutsItem(name, display_name, parent, type);
  }
  assert(!type_to_shortcut_.contains(type));
  type_to_shortcut_[type] = item;
  auto shortcuts = getShortcuts(type);
  for (const auto& shortcut : shortcuts) {
    addShortcut(type, shortcut);
  }
  parent->children.append(item);
  return item;
}

bool ShortcutsModel::checkIfConflicts(ShortcutsItem* item) const {
  for (const auto& shortcut : item->shortcuts()) {
    if (sequence_to_shortcut_[shortcut].size() > 1) {
      return true;
    }
  }
  return false;
}

ShortcutsModel::~ShortcutsModel() { delete root_; }

ShortcutsItem* ShortcutsModel::itemFromIndex(const QModelIndex& index) const {
  if (!index.isValid()) {
    return root_;
  }
  return static_cast<ShortcutsItem*>(index.internalPointer());
}

QModelIndex ShortcutsModel::indexFromItem(ShortcutsItem* item) const {
  auto parent = item->parent();
  return createIndex(parent->children.indexOf(item), 0, parent);
}

QList<ShortcutsItem*> ShortcutsModel::getItemsForSequence(
    const QKeySequence& sequence) {
  return sequence_to_shortcut_.value(sequence);
}

void ShortcutsModel::updateShortcutsForType(ShortcutType type) {
  auto item = type_to_shortcut_[type];
  item->updateShortcutsForActions();
  setShortcuts(type, item->shortcuts());
}

void ShortcutsModel::resetShortcutsForType(ShortcutType type) {
  auto item = type_to_shortcut_[type];
  for (const auto& shortcut : item->shortcuts()) {
    removeShortcut(type, shortcut);
  }
  for (const auto& shortcut : getShortcuts(type)) {
    addShortcut(type, shortcut);
  }
}

void ShortcutsModel::resetShortcutsToDefault(ShortcutType type) {
  auto shortcuts = defaultShortcuts();
  auto item = type_to_shortcut_[type];
  for (const auto& shortcut : item->shortcuts()) {
    removeShortcut(type, shortcut);
  }
  for (const auto& shortcut : shortcuts.value(type)) {
    addShortcut(type, shortcut);
  }
}

void ShortcutsModel::resetAllShortcutsToDefaults() {
  for (const auto& iter : type_to_shortcut_) {
    resetShortcutsToDefault(iter->type());
  }
}

QList<ShortcutType> ShortcutsModel::validShortcutTypes() const {
  return type_to_shortcut_.keys();
}

}  // namespace shortcuts
}  // namespace settings
}  // namespace util
}  // namespace veles
