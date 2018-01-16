#pragma once

#include <QMouseEvent>
#include <QtWidgets/QStyleOptionFocusRect>
#include <QtWidgets/QStylePainter>
#include <QtWidgets/QWidget>

namespace veles {
namespace ui {

using Address = uint64_t;

class AddressColumnWidget : public QWidget {
  Q_OBJECT

  QVector<Address> addressVector;
  QVector<QRectF> renderRectVector;
  QHash<Address, unsigned> hintsDict;

  bool debugPainting = false;
  QPointF mousePosition;

  QString addressToText(Address address);

 public:
  AddressColumnWidget();
  void setAddressVector(decltype(addressVector) && addressVector);
  void setHintMap(decltype(hintsDict) && hints);

 protected:
  void paintEvent(QPaintEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void leaveEvent(QEvent* event) override;
};

}  // namespace ui
}  // namespace veles
