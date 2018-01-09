#include "ui/disasm/disasmwidget.h"
#include <cassert>
#include <iostream>
#include <QtWidgets/QStyleOptionFocusRect>

static QString lorem_ipsum =
    "Lorem ipsum dolor sit amet, solet abhorreant qui ex, mea tantas officiis "
    "intellegam te, eu mea harum volutpat percipitur. Moderatius dissentiet "
    "sed ex, qui illum evertitur ut. Habeo possim accusamus ea mea. Est no "
    "brute inermis referrentur. Utroque ponderum cu duo, vel volumus mediocrem "
    "id. Ut eos dolores efficiantur, vim eu nemore viderer torquatos.\n"
    "\n"
    "Ex cum duis lucilius. Mucius facete comprehensam eu mea. Laudem legendos "
    "nam in. Tantas persius nominati vis ea, tollit postea docendi vim te. Ut "
    "hinc integre voluptaria eam, magna iisque constituam ea vel. Nam iuvaret "
    "scribentur ad, mutat summo torquatos vix ne, per adhuc molestie ei.\n"
    "\n"
    "Putent perpetua cotidieque eu vis, saepe verear nostrud vis ex, veritus "
    "omittam mnesarchum ut sit. Ad graeci scaevola recusabo nam, scribentur "
    "reformidans ullamcorper no pro, ad mel liber ceteros. Ullum dictas "
    "vivendo sed ut. Mea no deterruisset interpretaris, qui prima detraxit "
    "sadipscing ei. Diam homero aperiri ea mei.\n"
    "\n"
    "Ei his dictas veritus, an partem graeci sea, solet iudico fabellas te "
    "sit. Ut ferri velit dicta per, duo ea pertinax signiferumque. No cum "
    "dictas legendos omittantur. Putant deserunt assueverit has in, nam error "
    "regione constituam eu. Labore delicatissimi eos in, at usu lorem ancillae "
    "disputationi, partem elaboraret at sed.\n"
    "\n"
    "In mea facer partem, vel bonorum singulis disputando ne. Pro erant "
    "senserit consulatu id. Quot dicunt oportere usu ea. Quo falli euismod "
    "legendos in, est causae scribentur no, duo minim offendit reprehendunt "
    "at. No agam eius consectetuer mel, ad sea oratio dicant scriptorem.";

DisasmWidget::DisasmWidget() : main_layout_(this) {
  main_layout_.setSizeConstraint(QLayout::SizeConstraint::SetMaximumSize);

  // main_layout_.addWidget(&debug_output_widget_);
  // setLayout(&main_layout_);
  setWidget(&debug_widget);

  debug_widget.setAddressVector({1,  2,  3,  4,  5,  6,  7,  8,
                                 9,  10, 11, 12, 13, 14, 15, 16,
                                 17, 18, 19, 20, 21, 22, 23, 24});

  debug_widget.setHintMap({{13, 2}, {23, 5}});
  debug_widget.setMaximumWidth(100);
  debug_widget.setMinimumHeight(800);
}

DisasmWidget::~DisasmWidget() {}

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
