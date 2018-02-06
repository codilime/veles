#include "ui/disasm/disasmwidget.h"

namespace veles {
namespace ui {

DisasmWidget::DisasmWidget() {
  setupMocks();
  getEntrypoint();

  address_column.setAddressVector({1,  2,  3,  4,  5,  6,  7,  8,
                                   9,  10, 11, 12, 13, 14, 15, 16,
                                   17, 18, 19, 20, 21, 22, 23, 24});

  address_column.setHintMap({{13, 2}, {23, 5}});
  address_column.setFixedWidth(100);  // todo(zpp) fix it somewhen

  main_layout.addWidget(&address_column);
  // main_layout.addWidget(&rich_text_widget);
  main_layout.addLayout(&sub_layout);
  setLayout(&main_layout);
}

void DisasmWidget::setupMocks() {
  disasm::mocks::ChunkTreeFactory ctf;

  std::unique_ptr<disasm::mocks::ChunkNode> root = ctf.generateTree(disasm::mocks::ChunkType::FILE);
  ctf.setAddresses(root.get(), 0, 0x1000);

  std::shared_ptr<disasm::mocks::ChunkNode> sroot{root.release()};

  std::unique_ptr<disasm::mocks::MockBlob> mb = std::make_unique<disasm::mocks::MockBlob>(sroot);
  blob_ = std::unique_ptr<disasm::Blob>(std::move(mb));
}

void DisasmWidget::getEntrypoint() {
  entrypoint_ = blob_.get()->getEntrypoint();

  watcher_.setFuture(entrypoint_);

  QObject::connect(&watcher_, SIGNAL(finished()),
          this, SLOT(getWindow()));
}

void DisasmWidget::getWindow() {
  disasm::Bookmark e = entrypoint_.result();
  window_ = blob_.get()->createWindow(e, 2, 10);

  auto entries = window_.get()->entries();

  for (auto e : entries) {
    DisasmLabel* label = new DisasmLabel(*(e.get()));
    sub_layout.addWidget(label);
  }
}

}  // namespace ui
}  // namespace veles
