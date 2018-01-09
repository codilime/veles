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

using Address = uint64_t;

// vector adresów
// mapa hintów

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

class DisasmWidget : public QDockWidget {
  Q_OBJECT

  QVBoxLayout main_layout_;
  AddressColumnWidget debug_widget;

 public:
  DisasmWidget();

  virtual ~DisasmWidget();
};
