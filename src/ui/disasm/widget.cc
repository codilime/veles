#include "ui/disasm/widget.h"

namespace veles {
namespace ui {
namespace disasm {

Widget::Widget() {
  setupMocks();
  getEntrypoint();

  // main_layout.addWidget(&rich_text_widget);
  main_layout.addLayout(&sub_layout);
  setLayout(&main_layout);
}

void Widget::setupMocks() {
  mocks::ChunkTreeFactory ctf;

  std::unique_ptr<mocks::ChunkNode> root = ctf.generateTree(mocks::ChunkType::FILE);
  ctf.setAddresses(root.get(), 0, 0x1000);

  std::shared_ptr<mocks::ChunkNode> sroot{root.release()};

  std::unique_ptr<mocks::MockBlob> mb = std::make_unique<mocks::MockBlob>(sroot);
  blob_ = std::unique_ptr<Blob>(std::move(mb));
}

void Widget::getEntrypoint() {
  entrypoint_ = blob_.get()->getEntrypoint();

  watcher_.setFuture(entrypoint_);

  QObject::connect(&watcher_, SIGNAL(finished()), this, SLOT(getWindow()));
}

void Widget::getWindow() {
  Bookmark e = entrypoint_.result();
  window_ = blob_.get()->createWindow(e, 2, 10);

  auto entries = window_.get()->entries();

  for (auto e : entries) {
    Label *label = new Label(*(e.get()));
    sub_layout.addWidget(label);
  }
}

}  // namespace disasm
}  // namespace ui
}  // namespace veles
