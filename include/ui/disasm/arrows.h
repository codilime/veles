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

#pragma once

#include <iostream>

#include <mutex>
#include <QWidget>
#include <QtGui/QPaintEvent>
#include <QtGui/QPainter>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>

namespace veles {
namespace ui {
namespace disasm {

struct Arrow {
  Arrow(int start_row, int end_row, int level = 0);

  int start_row;
  int end_row;
  int level;
  friend std::ostream& operator<<(std::ostream& out, const Arrow& arrow);
};

/**
 * This widget is able to draw arrows pointing between rows.
 * As input (updateArrows) it takes row attach points and vector of Arrow
 * objects.
 * Attach points are pixel-level heights (increasing downwards)
 * where arrows will originate or terminate.
 *
 * Arrow objects contain three fields: start_row and end_row are logical
 * rows where arrow will start and end, i.e. arrow will start at height
 * row_attach_points[arrow.start_row] and terminate at
 * row_attach_points[arrow.end_row],
 * therefore start_row and end_row must be lower than size of row_attach_points.
 * Otherwise it will be skipped.
 *
 * Field level means how far from right edge the vertical part of the arrow will
 * be placed.
 * The higher max level, the denser the arrows are packed.
 * Distance between arrows in adjacent levels is calculated automagically
 * based on requested widget width and max level.
 *
 * Doc written: 7 March 2018, prybicki
 */
class ArrowsWidget : public QWidget {
  Q_OBJECT

 public:
  explicit ArrowsWidget(QWidget* parent);

  void updateArrows(std::vector<int> row_attach_points,
                    std::vector<Arrow> arrows);

  void paintEvent(QPaintEvent* event) override;

 private:
  static const int ARROWHEAD_WIDTH;
  static const int ARROWHEAD_HEIGHT;
  static const int DEFAULT_WIDTH;
  static const int MIN_LEVELS;

  int width_;
  int height_;
  int levels_;
  int points_per_level_;

  std::vector<int> row_attach_points_;
  std::vector<Arrow> arrows_;

  std::mutex paint_change_mutex;

  void paintSingleArrow(const Arrow& arrow, QPainter* painter);
};

}  // namespace disasm
}  // namespace ui
}  // namespace veles
