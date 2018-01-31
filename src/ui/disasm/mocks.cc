#include "ui/disasm/mocks.h"

#include "ui/disasm/asmgen.h"

namespace veles {
namespace ui {
namespace disasm {
namespace mocks {

const int MAX_FILE_CHILDREN = 2;
const int MAX_SECTION_CHILDREN = 5;
const int MAX_BASICBLOCK_CHILDREN = 10;

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
  setType(chk_ptr, type);
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

void ChunkFactory::setType(ChunkMeta* chunk, ChunkType type) const {
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

void ChunkFactory::setTextRepresentation(ChunkMeta* chunk) const {
  switch (chunk->meta_type) {
    case ChunkType::INSTRUCTION: {
      chunk->text_repr = randomInstruction();
      break;
    }
    case ChunkType::DATA: {
      auto text = veles::util::generateRandomUppercaseText(10);
      chunk->text_repr = text;
      chunk->raw_bytes = veles::data::BinData(
          8, 10, reinterpret_cast<const uint8_t*>(text.toUtf8().constData()));
      break;
    }
    default: { break; }
  }
}

void ChunkFactory::setComment(ChunkMeta* chunk) const {
  chunk->comment = QString("Comment about chunk #%1").arg(chunk_id);
}

ChunkNode::ChunkNode(std::unique_ptr<ChunkMeta> chunk)
    : chunk_{std::move(chunk)} {}

void ChunkNode::setParent(ChunkNode* parent) {
  chunk_->parent_id = parent->chunk()->id;
  parent_ = parent;
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

EntryFactory::EntryFactory(ChunkNode* root) : next_bookmark_{0} {
  generate(root);
}

const std::vector<std::shared_ptr<Entry>>& EntryFactory::getEntries() {
  return entries_;
}

void EntryFactory::generate(ChunkNode* node) {
  // EntryBegin
  auto entry_begin = std::make_shared<EntryChunkBegin>(node->chunk());
  node->chunk()->pos_begin.setNum(next_bookmark_);
  entry_begin->pos.setNum(next_bookmark_++);
  entries_.emplace_back(entry_begin);

  if (!node->chunk()->collapsed) {
    switch (node->chunk()->meta_type) {
      case ChunkType::DATA: {
        auto entry = std::make_shared<EntryField>(node->chunk());
        entry->pos.setNum(next_bookmark_++);
        entry->field_type = FieldType::STRING;
        entry->name = "data";
        entry->raw_bytes = node->chunk()->raw_bytes;
        entry->value =
            std::make_unique<FieldValueString>(entry->raw_bytes.toString());
        entries_.emplace_back(entry);
        break;
      }
      default:
        break;
    }

    // add entries of child
    for (auto& child : node->children()) {
      generate(child.get());
    }
  }

  // EntryEnd
  auto entry_end = std::make_shared<EntryChunkEnd>(node->chunk());
  entry_end->pos.setNum(next_bookmark_);
  node->chunk()->pos_end.setNum(next_bookmark_++);
  entries_.emplace_back(entry_end);
}

MockWindow::MockWindow(std::shared_ptr<ChunkNode> root) {
  this->root_chunk_ = std::move(root);
  this->generateEntries();
};

void MockWindow::generateEntries() {
  EntryFactory ef(this->root_chunk_.get());
  auto entries = ef.getEntries();

  entries_ = entries;
  entries_active_.first = 0;
  entries_active_.second = 0;

  uint32_t i = 0;
  for (auto& e : entries) {
    bookmarks_.emplace_back(e->pos);
    bookmark_entry_[e->pos] = i++;
  }

  // generate [ChunkID] -> Entry
  chunk_entry_.clear();
  for (auto e : entries) {
    if (e->type() == EntryType::CHUNK_BEGIN) {
      auto* entry_begin = reinterpret_cast<EntryChunkBegin*>(e.get());
      chunk_entry_[entry_begin->chunk->id] = entry_begin->pos;
    }
  }
}

void MockWindow::seek(const Bookmark& pos, unsigned prev_n, unsigned next_n) {
  std::lock_guard<std::mutex> guard(mutex_);

  current_position_ = pos;

  auto x = std::find(std::begin(bookmarks_), std::end(bookmarks_), pos);
  if (x == std::end(bookmarks_)) {
    return;
  }

  auto entry_index = bookmark_entry_.find(*x);
  if (entry_index == std::end(bookmark_entry_)) {
    return;
  }

  if (entry_index->second < prev_n) {
    entries_active_.first = 0;
  }
  else {
    entries_active_.first = entry_index->second - prev_n;
  }

  if (entries_.empty()) {
    entries_active_.second = 0;
    return;
  }

  if (entry_index->second + next_n >= entries_.size()) {
    entries_active_.second = entries_.size() - 1;
  }
  else {
    entries_active_.second = entry_index->second + next_n;
  }
}

Bookmark MockWindow::currentPosition() {
  std::lock_guard<std::mutex> guard(mutex_);
  return current_position_;
}

ScrollbarIndex MockWindow::currentScrollbarIndex() {
  std::lock_guard<std::mutex> guard(mutex_);

  return (entries_active_.first + entries_active_.second) / 2;
}

ScrollbarIndex MockWindow::maxScrollbarIndex() {
  std::lock_guard<std::mutex> guard(mutex_);
  return entries_.size();
}

const std::vector<Chunk>& MockWindow::breadcrumbs() {
  static std::vector<Chunk> breadcrumbs;
  return breadcrumbs;
}

const std::vector<std::shared_ptr<Entry>> MockWindow::entries() {
  std::lock_guard<std::mutex> guard(mutex_);

  std::vector<std::shared_ptr<Entry>> ne;
  for (const auto e : entries_) ne.push_back(e);

  return ne;
}

QFuture<void> MockWindow::chunkCollapseToggle(const ChunkID& id) {
  return QtConcurrent::run(this, &MockWindow::runChunkCollapseToggle, id);
}

void MockWindow::runChunkCollapseToggle(const ChunkID& id) {
  std::lock_guard<std::mutex> guard(mutex_);

  chunk_collapse_toggle(id, this->root_chunk_.get());
  generateEntries();
  emit dataChanged();
}

Bookmark MockWindow::getPositionByChunk(const ChunkID& chunk) {
  assert(chunk_entry_.find(chunk) != chunk_entry_.end());
  return chunk_entry_[chunk];
}

MockBlob::MockBlob(std::shared_ptr<ChunkNode> root) { root_ = std::move(root); }

std::unique_ptr<Window> MockBlob::createWindow(const Bookmark& pos,
                                               unsigned prev_n,
                                               unsigned next_n) {
  std::unique_ptr<MockWindow> mw = std::make_unique<MockWindow>(root_);
  auto entries = mw->entries();

  auto ei = std::rand() % entries.size();
  entrypoint_ = entries[ei]->pos;

  mw->seek(pos, prev_n, next_n);

  this->window_ = mw.get();
  return std::unique_ptr<Window>(std::move(mw));
}

QFuture<Bookmark> MockBlob::getEntrypoint() {
  return QtConcurrent::run([=]() { return entrypoint_; });
}

QFuture<Bookmark> MockBlob::getPosition(ScrollbarIndex index) {
  return QtConcurrent::run([=]() {
    auto entries = window_->entries();
    assert(entries.size() <= index);
    return entries[index]->pos;
  });
}

QFuture<Bookmark> MockBlob::getPositionByChunk(const ChunkID& chunk) {
  return QtConcurrent::run(
      [=]() { return window_->getPositionByChunk(chunk); });
}

}  // namespace mocks
}  // namespace disasm
}  // namespace ui
}  // namespace veles
