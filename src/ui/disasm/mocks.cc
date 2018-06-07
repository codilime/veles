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

#include "ui/disasm/mocks.h"

namespace veles {
namespace ui {
namespace disasm {
namespace mocks {

const int MAX_FILE_CHILDREN = 10;
const int MAX_SECTION_CHILDREN = 50;
const int MAX_BASICBLOCK_CHILDREN = 100;

int random_int(int begin, int end) {
  std::uniform_int_distribution<> dis(begin, end);
  return dis(veles::util::g_mersenne_twister);
}

bool chunk_collapse_toggle(const ChunkID& id, ChunkNode* node) {
  if (!node) {
    return false;
  }

  if (node->chunk()->id == id) {
    node->chunk()->collapsed = !node->chunk()->collapsed;
    return true;
  }

  for (const auto& child : node->children()) {
    if (chunk_collapse_toggle(id, child.get())) return true;
  }

  return false;
}

ChunkFactory::ChunkFactory() : chunk_id{0} {}

std::unique_ptr<ChunkMeta> ChunkFactory::generate(ChunkType type) {
  std::unique_ptr<ChunkMeta> chk = std::make_unique<ChunkMeta>();

  auto chk_ptr = chk.get();

  setID(chk_ptr);
  setMetaType(chk_ptr, type);
  setDisplayName(chk_ptr);
  setTextRepresentation(chk_ptr);
  setComment(chk_ptr);

  chunk_id++;
  return chk;
}

void ChunkFactory::setID(ChunkMeta* chunk) const {
  chunk->id = QString("chk-id-%1").arg(chunk_id).toUtf8();
}

void ChunkFactory::setDisplayName(ChunkMeta* chunk) const {
  chunk->display_name = QString("Chunk #%1").arg(chunk_id);
}

void ChunkFactory::setMetaType(ChunkMeta* chunk, ChunkType type) const {
  chunk->meta_type = type;
  switch (type) {
    case ChunkType::FILE:
      chunk->type = "FILE";
      break;
    case ChunkType::SECTION:
      chunk->type = "SECTION";
      break;
    case ChunkType::BASIC_BLOCK:
      chunk->type = "BASIC_BLOCK";
      break;
    case ChunkType::INSTRUCTION:
      chunk->type = "INSTRUCTION";
      break;
    case ChunkType::DATA:
      chunk->type = "DATA";
      break;
    default:
      chunk->type = "UNKNOWN";
  }
}

void ChunkFactory::setDataType(ChunkMeta* chunk) const {
  if (chunk->meta_type != ChunkType::DATA) {
    return;
  }

  switch (std::rand() % 6) {
    case 0: {
      // UNKNOWN
      chunk->data_type = FieldType::UNKNOWN;
      break;
    }
    case 1: {
      // FIXED
      chunk->data_type = FieldType::FIXED;
      break;
    }
    case 2: {
      // FLOAT
      chunk->data_type = FieldType::FLOAT;
      break;
    }
    case 3: {
      // STRING
      chunk->data_type = FieldType::STRING;

      break;
    }
    case 4: {
      // REF
      chunk->data_type = FieldType::REF;
      break;
    }
    case 5: {
      // COMPOSED
      chunk->data_type = FieldType::COMPOSED;
      break;
    }
    default: { break; }
  }
}

void ChunkFactory::setTextRepresentation(ChunkMeta* chunk) const {
  switch (chunk->meta_type) {
    case ChunkType::INSTRUCTION: {
      chunk->text_repr = randomInstruction(chunk->id);
      break;
    }
    case ChunkType::DATA: {
      auto text = veles::util::generateRandomUppercaseText(10);
      chunk->text_repr = std::make_unique<String>("DATA");
      chunk->raw_bytes = veles::data::BinData(
          8, 10, reinterpret_cast<const uint8_t*>(text.toUtf8().constData()));
      break;
    }
    default: {
      chunk->text_repr = std::make_unique<String>("CHUNK");
      break;
    }
  }
}

void ChunkFactory::setComment(ChunkMeta* chunk) const {
  if (std::rand() % 5 == 0) {
    chunk->comment = QString("Comment about chunk #%1").arg(chunk_id);
  }
}

ChunkNode::ChunkNode(std::unique_ptr<ChunkMeta> chunk)
    : chunk_{std::move(chunk)} {}

ChunkNode::ChunkNode(ChunkMeta* chunk)
    : chunk_{std::unique_ptr<ChunkMeta>(chunk)} {}

void ChunkNode::setParent(ChunkNode* parent) {
  chunk_->parent_id = parent->chunk()->id;
  parent_ = parent;
}

void ChunkNode::addChild(ChunkNode* child) {
  children_.emplace_back(std::unique_ptr<ChunkNode>(child));
}

void ChunkNode::addChild(std::unique_ptr<ChunkNode> child) {
  children_.emplace_back(std::move(child));
}

const std::vector<std::unique_ptr<ChunkNode>>& ChunkNode::children() const {
  return children_;
}

ChunkNode* ChunkNode::parent() { return parent_; }

std::shared_ptr<ChunkMeta> ChunkNode::chunk() { return chunk_; }

void ChunkTreeFactory::setAddresses(ChunkNode* root_node, Address begin,
                                    Address end) {
  auto root = root_node->chunk();
  root->addr_begin = begin;
  root->addr_end = end;

  auto& children = root_node->children();
  if (children.empty()) {
    return;
  }

  Address length = (end - begin + 1) / children.size();

  Address child_begin, child_end;
  child_begin = begin;
  for (auto& child : children) {
    child_end = child_begin + length;
    if (child_end > end) {
      child_end = end;
    }
    setAddresses(child.get(), child_begin, child_end);
    child_begin += length;
  }
}

std::unique_ptr<ChunkNode> ChunkTreeFactory::generateTree(ChunkType type) {
  auto root = std::make_unique<ChunkNode>(chunk_factory_.generate(type));

  if (type == ChunkType::INSTRUCTION || type == ChunkType::DATA) {
    root->chunk()->collapsed = true;
    return root;
  }

  ChunkType next_type;
  switch (type) {
    case ChunkType::FILE:
      next_type = ChunkType::SECTION;
      break;
    case ChunkType::SECTION:
      if (std::rand() % 2 != 0)
        next_type = ChunkType::BASIC_BLOCK;
      else
        next_type = ChunkType::DATA;
      break;
    case ChunkType::BASIC_BLOCK:
      next_type = ChunkType::INSTRUCTION;
      break;
    default:
      return root;
  }

  int max_child_num;
  switch (next_type) {
    case ChunkType::INSTRUCTION:
      max_child_num = MAX_BASICBLOCK_CHILDREN;
      break;
    case ChunkType::BASIC_BLOCK:
      max_child_num = MAX_SECTION_CHILDREN;
      break;
    case ChunkType::SECTION:
      max_child_num = MAX_FILE_CHILDREN;
      break;
    default:
      max_child_num = 2;
      break;
  }

  int child_num = random_int(2, max_child_num);

  for (int i = 0; i < child_num; i++) {
    auto instr = generateTree(next_type);
    instr->setParent(root.get());
    root->addChild(std::move(instr));
  }

  return root;
}

EntryFactory::EntryFactory(ChunkNode* root, bool ignore_collapsed = false)
    : ignore_collapsed_{ignore_collapsed} {
  generateBookmarkMap(root);
  generate(root);
}

const std::vector<std::shared_ptr<Entry>>& EntryFactory::getEntries() {
  return entries_;
}

void EntryFactory::fillEntryField(EntryField* e, ChunkMeta* chunk) {
  switch (chunk->data_type) {
    case FieldType::UNKNOWN: {
      break;
    }
    case FieldType::FIXED: {
      break;
    }
    case FieldType::FLOAT: {
      break;
    }
    case FieldType::STRING: {
      e->name = "data::string";
      e->raw_bytes = chunk->raw_bytes;
      e->value = std::make_unique<FieldValueString>(e->raw_bytes.toString());
      break;
    }
    case FieldType::REF: {
      break;
    }
    case FieldType::COMPOSED: {
      break;
    }
  }
}

std::map<ChunkID, Bookmark>&& EntryFactory::getBookmarkMap() {
  return std::move(bookmark_map_);
}

void EntryFactory::generateBookmarkMap(ChunkNode* node) {
  if (node == nullptr) {
    return;
  }
  bool is_parent_collapsed = false;
  ChunkNode* parent = node->parent();
  while (parent != nullptr) {
    if (parent->chunk()->collapsed) {
      is_parent_collapsed = true;
      break;
    }
    parent = parent->parent();
  }
  if (!is_parent_collapsed) {
    bookmark_map_[node->chunk()->id] = node->chunk()->pos_begin;
  } else {
    bookmark_map_[node->chunk()->id] = parent->chunk()->pos_begin;
  }
  for (const auto& child : node->children()) {
    generateBookmarkMap(child.get());
  }
}

void EntryFactory::generate(ChunkNode* node) {
  if (node->chunk()->meta_type == ChunkType::INSTRUCTION) {
    auto entry = std::make_shared<EntryChunkCollapsed>(node->chunk());
    entry->pos = node->chunk()->pos_begin;
    entries_.emplace_back(entry);
    return;
  }

  if (!ignore_collapsed_ && node->chunk()->collapsed) {
    auto entry = std::make_shared<EntryChunkCollapsed>(node->chunk());
    entry->pos = node->chunk()->pos_begin;
    entries_.emplace_back(entry);
    return;
  }

  // EntryBegin
  auto entry_begin = std::make_shared<EntryChunkBegin>(node->chunk());
  entry_begin->pos = node->chunk()->pos_begin;
  entries_.emplace_back(entry_begin);

  switch (node->chunk()->meta_type) {
    case ChunkType::DATA: {
      auto entry = std::make_shared<EntryField>(node->chunk());
      entry->pos = node->chunk()->pos_begin;
      entry->name = "data";
      fillEntryField(entry.get(), node->chunk().get());
      entries_.emplace_back(entry);
      break;
    }
    default:
      break;
  }
  // add entries of child
  for (const auto& child : node->children()) {
    generate(child.get());
  }

  // EntryEnd
  auto entry_end = std::make_shared<EntryChunkEnd>(node->chunk());
  entry_end->pos = node->chunk()->pos_end;
  entries_.emplace_back(entry_end);
}

MockWindow::MockWindow(std::shared_ptr<MockBackend> backend) {
  backend_ = std::move(backend);
};

void MockWindow::seek(const Bookmark& pos, unsigned prev_n, unsigned next_n) {
  mutex_.lock();
  current_position_ = pos;

  last_prev_n_ = prev_n;
  last_next_n_ = next_n;

  generateEntries(prev_n, next_n);
  scrollbar_index_ = backend_->getEntryIndexByPosition(pos);

  mutex_.unlock();

  emit dataChanged();
}

void MockWindow::seekPosition(const Bookmark &pos) {
  mutex_.lock();
  current_position_ = pos;
  scrollbar_index_ = backend_->getEntryIndexByPosition(pos);
  mutex_.unlock();
}

void MockWindow::generateEntries(unsigned prev_n, unsigned next_n) {
  auto entries = backend_->getEntries();

  // determine entries to display
  auto i = static_cast<int>(scrollbar_index_ - prev_n);
  if (i < 0) {
    i = 0;
  }

  entries_visible_.clear();
  for (; i < entries.size() && i < scrollbar_index_ + next_n + 1; i++) {
    entries_visible_.push_back(std::move(entries[i]));
  }
}

Bookmark MockWindow::currentPosition() {
  std::lock_guard<std::mutex> guard(mutex_);
  return current_position_;
}

ScrollbarIndex MockWindow::currentScrollbarIndex() {
  std::lock_guard<std::mutex> guard(mutex_);
  return scrollbar_index_;
}

ScrollbarIndex MockWindow::maxScrollbarIndex() {
  std::lock_guard<std::mutex> guard(mutex_);
  return backend_->getEntriesSize();
}

const std::vector<Chunk>& MockWindow::breadcrumbs() {
  static std::vector<Chunk> breadcrumbs;
  return breadcrumbs;
}

const std::vector<std::shared_ptr<Entry>> MockWindow::entries() {
  std::lock_guard<std::mutex> guard(mutex_);
  return entries_visible_;
}

QFuture<void> MockWindow::chunkCollapseToggle(const ChunkID& chunk) {
  return QtConcurrent::run([=]() {
    mutex_.lock();
    backend_->chunkCollapse(chunk);
    generateEntries(last_prev_n_, last_next_n_);
    mutex_.unlock();

    emit dataChanged();
  });
}

int MockWindow::entryIndentation(Bookmark pos) {
  return backend_->getEntryIndentation(pos);
}

MockBlob::MockBlob(std::shared_ptr<ChunkNode> root) {
  root_ = std::move(root);
  backend_ = std::make_shared<MockBackend>(root_);
}

std::unique_ptr<Window> MockBlob::createWindow(const Bookmark& pos,
                                               unsigned prev_n,
                                               unsigned next_n) {
  std::unique_ptr<MockWindow> mw = std::make_unique<MockWindow>(backend_);
  mw->seek(pos, prev_n, next_n);

  return std::unique_ptr<Window>(std::move(mw));
}

QFuture<Bookmark> MockBlob::getEntrypoint() {
  return QtConcurrent::run([=]() { return backend_->getEntrypoint(); });
}

QFuture<Bookmark> MockBlob::getPosition(ScrollbarIndex index) {
  return QtConcurrent::run([=]() { return backend_->getPosition(index); });
}

QFuture<Bookmark> MockBlob::getPositionByChunk(const ChunkID& chunk) {
  return QtConcurrent::run(
      [=]() { return backend_->getPositionByChunk(chunk); });
}

MockBackend::MockBackend() {}

MockBackend::MockBackend(std::shared_ptr<ChunkNode> root) {
  root_ = std::move(root);
  generateEntries();

  EntryFactory ef(root_.get(), true);
  flat_entries_ = ef.getEntries();
  if (!entries_.empty()) {
    entrypoint_ = entries_[entries_.size() / 2]->pos;
  }
}

Bookmark MockBackend::getPosition(ScrollbarIndex idx) {
  std::lock_guard<std::mutex> guard(mutex_);
  assert(idx < entries_.size());
  return entries_[idx]->pos;
}

Bookmark MockBackend::getEntrypoint() {
  std::lock_guard<std::mutex> guard(mutex_);
  return entrypoint_;
}

const std::vector<std::shared_ptr<Entry>> MockBackend::getEntries() {
  std::lock_guard<std::mutex> guard(mutex_);
  return entries_;
}

int MockBackend::getEntryIndentation(Bookmark pos) {
  std::lock_guard<std::mutex> guard(mutex_);

  if (position_index_.find(pos) == position_index_.end()) {
    return 0;
  }

  auto i = position_index_[pos];
  return entry_indentation_[i];
}

Bookmark MockBackend::getPositionByChunk(const ChunkID& chunk) {
  std::lock_guard<std::mutex> guard(mutex_);
  assert(chunk_entry_.find(chunk) != chunk_entry_.end());
  return chunk_entry_[chunk];
}

ScrollbarIndex MockBackend::getEntryIndexByPosition(const Bookmark& pos) {
  std::lock_guard<std::mutex> guard(mutex_);
  return position_index_[pos];
}

ScrollbarIndex MockBackend::getEntriesSize() { return entries_.size(); }

void MockBackend::generateEntries() {
  std::lock_guard<std::mutex> guard(mutex_);

  // rebuild entry stream based on current
  // collapse configuration
  EntryFactory ef(root_.get(), false);
  chunk_entry_ = std::move(ef.getBookmarkMap());
  entries_ = ef.getEntries();

  // rebuild bookmark -> scrollbar index cache
  position_index_.clear();
  for (int i = 0; i < entries_.size(); i++) {
    position_index_[entries_[i]->pos] = i;
  }

  // rebuild entry_indentation
  entry_indentation_.clear();
  int indent = 0;
  for (int i = 0; i < entries_.size(); i++) {
    switch (entries_[i]->type()) {
      case EntryType::CHUNK_BEGIN: {
        entry_indentation_.emplace_back(indent);
        indent++;
        break;
      }
      case EntryType::CHUNK_END: {
        indent--;
        entry_indentation_.emplace_back(indent);
        break;
      }
      default: {
        entry_indentation_.emplace_back(indent);
      }
    }
  }
}

void MockBackend::chunkCollapse(const ChunkID& chunk) {
  chunk_collapse_toggle(chunk, root_.get());
  generateEntries();
}

}  // namespace mocks
}  // namespace disasm
}  // namespace ui
}  // namespace veles
