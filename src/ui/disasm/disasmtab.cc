#include "ui/disasm/disasmtab.h"
#include <include/ui/disasm/disasmtab.h>
#include <cassert>
#include <iostream>
#include <QtWidgets/QStyleOptionFocusRect>

namespace veles {
namespace ui {

DisasmWidget::DisasmWidget() {
  // main_layout_.addWidget(&debug_output_widget_);
  // setLayout(&main_layout_);

  address_column.setAddressVector({1,  2,  3,  4,  5,  6,  7,  8,
                                   9,  10, 11, 12, 13, 14, 15, 16,
                                   17, 18, 19, 20, 21, 22, 23, 24});

  address_column.setHintMap({{13, 2}, {23, 5}});
  address_column.setMaximumWidth(100);
  address_column.setMinimumHeight(800);

  main_layout.addWidget(&address_column);
  setLayout(&main_layout);
}

void AddressColumnWidget::paintEvent(QPaintEvent* event)  // override
{
  QPainter painter(this);
  QStylePainter stylePainter(this);

  painter.fillRect(rect(), palette().color(QPalette::AlternateBase));

  const auto rowHeight = (1 + 0.25) * QFontMetricsF(painter.font()).height();

  auto currentAddressPositionY = 0;

  for (int i = 0; i < addressVector.size(); i++) {
    auto hintIterator = hintsDict.find(addressVector[i]);
    auto currentAddressMultiplicity =
        (hintIterator == hintsDict.end()) ? 1 : *hintIterator;
    auto currentAddressHeight = rowHeight * currentAddressMultiplicity;

    auto boundingBox = QRectF(0, currentAddressPositionY, rect().width(),
                              currentAddressHeight);
    boundingBox.setWidth(boundingBox.width() - 1);
    boundingBox.setHeight(boundingBox.height() - 1);

    renderRectVector[i] = boundingBox;

    if (debugPainting) {
      auto old_pen = painter.pen();
      painter.setPen(palette().color(QPalette::Highlight));
      painter.drawRect(boundingBox);
      painter.setPen(old_pen);
    }

    if (underMouse() && renderRectVector[i].contains(mousePosition)) {
      auto styleOption = QStyleOptionFocusRect();
      styleOption.rect = renderRectVector[i].toRect();
      stylePainter.drawPrimitive(QStyle::PE_FrameFocusRect, styleOption);
    }

    painter.drawText(boundingBox, Qt::AlignHCenter | Qt::AlignVCenter,
                     addressToText(addressVector[i]), &boundingBox);

    currentAddressPositionY += currentAddressHeight;
  }
}

QString AddressColumnWidget::addressToText(Address address) {
  return "0x" + QString::number(address, 16).rightJustified(4 * 2, '0');
}

void AddressColumnWidget::setHintMap(decltype(hintsDict) && hintsDict) {
  this->hintsDict = hintsDict;
  update();
}

void AddressColumnWidget::setAddressVector(
    decltype(addressVector) && addressVector) {
  this->addressVector = addressVector;
  this->renderRectVector.resize(addressVector.size());
  update();
}

void AddressColumnWidget::mouseMoveEvent(QMouseEvent* event) {
  mousePosition = event->localPos();
  update();
}

AddressColumnWidget::AddressColumnWidget() { setMouseTracking(true); }

void AddressColumnWidget::leaveEvent(QEvent* event) { update(); }

DisasmTab::DisasmTab() : IconAwareView("Kotek1", "Kotek2") {
  //  auto* dock_address_column = new DockWidget;
  //  dock_address_column->setWidget(&disasm_widget);
  //  addDockWidget(Qt::LeftDockWidgetArea, dock_address_column);
  setCentralWidget(&disasm_widget);
}
}  // namespace ui
}  // namespace veles