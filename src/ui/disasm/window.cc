#include "ui/disasm/window.h"

namespace veles {
namespace ui {
namespace disasm {

WindowCache::WindowCache(std::shared_ptr<Window> window) : window_(window) {
  connect(window_.get(), &Window::dataChanged, this, &WindowCache::newData);
}

void WindowCache::seek(const veles::ui::disasm::Bookmark& pos, unsigned prev_n,
                       unsigned next_n) {
  window_->seek(pos, prev_n, next_n);
}

Bookmark WindowCache::currentPosition() { return window_->currentPosition(); }

void WindowCache::newData() { emit dataChanged(); }

ScrollbarIndex WindowCache::currentScrollbarIndex() {
  return window_->currentScrollbarIndex();
}

ScrollbarIndex WindowCache::maxScrollbarIndex() {
  return window_->maxScrollbarIndex();
}

const std::vector<Chunk>& WindowCache::breadcrumbs() {
  return window_->breadcrumbs();
}

const std::vector<std::shared_ptr<Entry>> WindowCache::entries() {
  return window_->entries();
}

QFuture<void> WindowCache::chunkCollapseToggle(const ChunkID& id) {
  return window_->chunkCollapseToggle(id);
}
}
}
}
