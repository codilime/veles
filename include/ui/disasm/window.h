#ifndef VELES_WINDOW_H
#define VELES_WINDOW_H

#include <vector>

#include "ui/disasm/disasm.h"

namespace veles {
namespace ui {
namespace disasm {

class WindowCache : public Window {
  Q_OBJECT

 public:
  explicit WindowCache(std::shared_ptr<Window> window);

  void seek(const Bookmark& pos, unsigned prev_n, unsigned next_n) override;

  Bookmark currentPosition() override;
  ScrollbarIndex currentScrollbarIndex() override;
  ScrollbarIndex maxScrollbarIndex() override;
  const std::vector<Chunk>& breadcrumbs() override;
  const std::vector<std::shared_ptr<Entry>> entries() override;
  QFuture<void> chunkCollapseToggle(const ChunkID& id) override;

 private slots:
  void newData();
  void setScrollBarIndex(int index);

 private:
  int entries_prefetch_factor_;

  int scroll_bar_index_;
  int scroll_bar_window_index_;

  std::vector<std::shared_ptr<Entry>> entries_;
  std::shared_ptr<Window> window_;
};

}  // namespace disasm
}  // namespace ui
}  // namespace veles

#endif  // VELES_WINDOW_H
