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

#pragma once

#include <mutex>

#include "ui/disasm/disasm.h"
#include "util/random.h"

namespace veles {
namespace ui {
namespace disasm {
namespace mocks {

// Random int in range [begin, end]
int random_int(int begin, int end);

enum class ChunkType { FILE, SECTION, BASIC_BLOCK, INSTRUCTION, DATA, UNKNOWN };

struct ChunkMeta : Chunk {
  veles::data::BinData raw_bytes;
  ChunkType meta_type = ChunkType::UNKNOWN;

  bool collapsed = false;
};

/**
 * ChunkFactory keeps track of uniqueness of chunk IDs.
 * It can generate chunks for later usage.
 * User is responsible for setting proper positions and addresses.
 */
class ChunkFactory {
 public:
  ChunkFactory();
  std::unique_ptr<ChunkMeta> generate(ChunkType type);

 protected:
  using MockChunkID = uint64_t;

  void setID(ChunkMeta* chunk) const;
  void setDisplayName(ChunkMeta* chunk) const;
  void setType(ChunkMeta* chunk, ChunkType type) const;
  void setTextRepresentation(ChunkMeta* chunk) const;
  void setComment(ChunkMeta* chunk) const;

  MockChunkID chunk_id;
};

/*
 * ChunkNode wraps Chunks for tree building.
 */
class ChunkNode {
 public:
  explicit ChunkNode(std::unique_ptr<ChunkMeta> chunk);

  void setParent(ChunkNode* parent);
  void addChild(std::unique_ptr<ChunkNode> child);

  const std::vector<std::unique_ptr<ChunkNode>>& children() const;
  ChunkNode* parent();
  std::shared_ptr<ChunkMeta> chunk();

 protected:
  ChunkNode* parent_;

  std::vector<std::unique_ptr<ChunkNode>> children_;
  std::shared_ptr<ChunkMeta> chunk_;
};

/*
 * ChunkTreeFactory builds a tree of ChunkNodes.
 */
class ChunkTreeFactory {
 public:
  void setAddresses(ChunkNode* root_node, Address begin, Address end);
  std::unique_ptr<ChunkNode> generateTree(ChunkType type);

 protected:
  ChunkFactory chunk_factory_;
};

/*
 * Iterates over tree and builds a vector of entries.
 */
class EntryFactory {
 public:
  explicit EntryFactory(ChunkNode* root);

  const std::vector<std::shared_ptr<Entry>>& getEntries();

 protected:
  void generate(ChunkNode* node);

  int next_bookmark_;
  std::vector<std::shared_ptr<Entry>> entries_;
};

class MockWindow : public Window {
 public:
  explicit MockWindow(std::shared_ptr<ChunkNode> root);

  void seek(const Bookmark& pos, unsigned prev_n, unsigned next_n) override;

  Bookmark currentPosition() override;
  ScrollbarIndex currentScrollbarIndex() override;
  ScrollbarIndex maxScrollbarIndex() override;
  const std::vector<Chunk>& breadcrumbs() override;
  const std::vector<std::shared_ptr<Entry>> entries() override;
  QFuture<void> chunkCollapseToggle(const ChunkID& id) override;

  void runChunkCollapseToggle(const ChunkID& id);

  Bookmark getPositionByChunk(const ChunkID& chunk);

 protected:
  void generateEntries();

  std::mutex mutex_;

  std::shared_ptr<ChunkNode> root_chunk_;

  std::vector<std::shared_ptr<Entry>> entries_;
  std::pair<ScrollbarIndex, ScrollbarIndex> entries_active_;

  std::map<ChunkID, Bookmark> chunk_entry_;

  std::vector<Bookmark> bookmarks_;

  Bookmark current_position_;
  std::map<Bookmark, uint32_t> bookmark_entry_;
};

class MockBlob : public Blob {
 public:
  explicit MockBlob(std::shared_ptr<ChunkNode> root);

  std::unique_ptr<Window> createWindow(const Bookmark& pos, unsigned prev_n,
                                       unsigned next_n) override;

  QFuture<Bookmark> getEntrypoint() override;
  QFuture<Bookmark> getPosition(ScrollbarIndex index) override;
  QFuture<Bookmark> getPositionByChunk(const ChunkID& chunk) override;

 protected:
  MockWindow* window_;
  std::shared_ptr<ChunkNode> root_;

  Bookmark entrypoint_;
};

}  // namespace mocks
}  // namespace disasm
}  // namespace ui
}  // namespace veles
