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
#include <memory>
#include <QWidget>
#include <QtGui/QPaintEvent>
#include <QtGui/QPainter>
#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>

namespace veles {
namespace ui {
namespace disasm {


struct Arrow {
  Arrow(unsigned start_row, unsigned end_row, unsigned level = 0);

  unsigned start_row;
  unsigned end_row;
  unsigned level;
  friend std::ostream& operator<<(std::ostream& out, const Arrow& arrow);
};

/**
 * This widget is able to draw arrows pointing between rows.
 * As input (updateArrows) it takes row attach points and vector of Arrow objects.
 * Attach points pixel-level heights where arrows will originate or terminate.
 *
 * Arrow objects contain three fields: start_row and end_row are logical
 * rows where arrow will start and end, i.e. arrow will start at height
 * row_attach_points[arrow.start_row] and terminate at row_attach_points[arrow.end_row],
 * therefore start_row and end_row must be lower than size of row_attach_points.
 * Otherwise it will be skipped.
 *
 * Field level means how far from right edge the vertical part of the arrow will be placed.
 * The higher max level, the denser the arrows are packed.
 * Distance between arrows in adjacent levels is calculated automagically
 * based on requested widget width and max level.
 *
 * Doc written: 7 March 2018
 */
class Arrows : public QWidget {
  Q_OBJECT

  const unsigned ARROWHEAD_WIDTH = 10;
  const unsigned ARROWHEAD_HEIGHT = 10;
  const unsigned DEFAULT_WIDTH = 200;
  const unsigned MIN_LEVELS = 8;

  unsigned width_ = DEFAULT_WIDTH;
  unsigned height_ = 0;
  unsigned levels = MIN_LEVELS;
  unsigned points_per_level;

  std::vector<unsigned> row_attach_points;
  std::vector<Arrow> arrows;

  void paintSingleArrow(Arrow& arrow, QPainter& painter);

 public:
  Arrows(QWidget* parent);

  void updateArrows(std::vector<unsigned> row_attach_points,
                    std::vector<Arrow> arrows);

  void paintEvent(QPaintEvent* event) override;
};

}  // namespace disasm
}  // namespace ui
}  // namespace veles
