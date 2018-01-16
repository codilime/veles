#include "ui/disasm/addresscolumn.h"

namespace veles {
namespace ui {

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

}  // namespace ui
}  // namespace veles
