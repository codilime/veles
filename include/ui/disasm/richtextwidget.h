#pragma once

#include <QtWidgets/QWidget>
namespace veles {
namespace ui {

class RichTextWidget : public QWidget {
  Q_OBJECT

  void paintEvent(QPaintEvent* event) override;
};

}  // namespace ui
}  // namespace veles
