#include "ui/disasm/disasmwidget.h"

DisasmWidget::DisasmWidget() : tmp("Hello disasm!", this) { tmp.move(10, 10); }

void DisasmWidget::paintEvent(QPaintEvent* event) { tmp.update(); }

DisasmWidget::~DisasmWidget() {}
