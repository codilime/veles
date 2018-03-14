/*
 * Copyright 2018 CodiLime
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "ui/disasm/arrows.h"
#include <QtWidgets/QLabel>

namespace veles {
namespace ui {
namespace disasm {

const int ArrowsWidget::ARROWHEAD_WIDTH = 10;
const int ArrowsWidget::ARROWHEAD_HEIGHT = 10;
const int ArrowsWidget::DEFAULT_WIDTH = 200;
const int ArrowsWidget::MIN_LEVELS = 8;

Arrow::Arrow(int start_row, int end_row, int level)
    : start_row(start_row), end_row(end_row), level(level) {}

std::ostream& operator<<(std::ostream& out, const Arrow& arrow) {
  out << "(" << arrow.start_row << ", " << arrow.end_row << ") @ "
      << arrow.level;
  return out;
}

ArrowsWidget::ArrowsWidget(QWidget* parent)
    : QWidget(parent), width_(DEFAULT_WIDTH), height_(0), levels_(MIN_LEVELS) {
  setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
  setFixedSize(width_, height_);
}

void ArrowsWidget::paintEvent(QPaintEvent* event) {
  std::lock_guard<std::mutex> lock(paint_change_mutex);
  QPainter painter(this);

  auto brush = painter.brush();
  brush.setStyle(Qt::BrushStyle::SolidPattern);
  brush.setColor(palette().color(QPalette::Text));
  painter.setBrush(brush);

  auto pen = painter.pen();
  pen.setColor(palette().color(QPalette::Text));
  painter.setPen(pen);

  painter.fillRect(event->rect(), palette().color(QPalette::AlternateBase));

  for (const auto& arrow : arrows_) {
    if (arrow.level == 0) {
      std::cerr << "Warning: unspecified level of an arrow! " << arrow
                << std::endl;
      continue;
    }
    if (arrow.start_row >= static_cast<int>(row_attach_points_.size()) ||
        arrow.end_row >= static_cast<int>(row_attach_points_.size())) {
      std::cerr << "Warning: attach point not specified for start/end row! "
                << arrow << std::endl;
      continue;
    }

    paintSingleArrow(arrow, painter);
  }
}

void ArrowsWidget::updateArrows(std::vector<int> _row_attach_points,
                                std::vector<Arrow> _arrows) {
  std::lock_guard<std::mutex> lock(paint_change_mutex);

  this->arrows_ = std::move(_arrows);
  this->row_attach_points_ = std::move(_row_attach_points);

  int max_level =
      std::max_element(arrows_.begin(), arrows_.end(), [](const Arrow& lhs,
                                                          const Arrow& rhs) {
        return lhs.level < rhs.level;
      })->level;
  levels_ = std::max(MIN_LEVELS, max_level);

  int max_attach_point =
      *std::max_element(row_attach_points_.begin(), row_attach_points_.end());
  height_ = max_attach_point + ARROWHEAD_HEIGHT + 1;

  width_ = width();
  points_per_level_ = width_ / (levels_ + 1);
  setFixedSize(width_, height_);
  update();
}

void ArrowsWidget::paintSingleArrow(const Arrow& arrow, QPainter& painter) {
  QPoint start_point = QPoint(width_, row_attach_points_[arrow.start_row]);

  QPoint first_turn = QPoint(width_ - (arrow.level * points_per_level_),
                             row_attach_points_[arrow.start_row]);

  QPoint second_turn = QPoint(width_ - (arrow.level * points_per_level_),
                              row_attach_points_[arrow.end_row]);

  QPoint end_point = QPoint(width_, row_attach_points_[arrow.end_row]);

  painter.drawLine(start_point, first_turn);
  painter.drawLine(first_turn, second_turn);
  painter.drawLine(second_turn, end_point);

  QPoint arrowhead_points[] = {end_point,
                               QPoint(end_point.x() - ARROWHEAD_WIDTH,
                                      end_point.y() + ARROWHEAD_HEIGHT / 2),
                               QPoint(end_point.x() - ARROWHEAD_WIDTH,
                                      end_point.y() - ARROWHEAD_HEIGHT / 2)};

  painter.drawConvexPolygon(arrowhead_points, 3);
}

}  // namespace disasm
}  // namespace ui
}  // namespace veles
