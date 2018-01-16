#pragma once

#include <QDockWidget>
#include <QHash>
#include <QMouseEvent>
#include <QObject>
#include <QPainter>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QVector>
#include <QWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStylePainter>

#include "ui/dockwidget.h"

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

class DisasmWidget : public QWidget {
  QHBoxLayout main_layout;
  AddressColumnWidget address_column;

 public:
  DisasmWidget();
};

class DisasmTab : public IconAwareView {
  Q_OBJECT

  DisasmWidget disasm_widget;

 public:
  DisasmTab();
};

}  // namespace ui
}  // namespace veles
