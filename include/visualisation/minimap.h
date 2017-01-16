/*
 * Copyright 2016 CodiLime
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
#ifndef VELES_VISUALISATION_MINIMAP_H
#define VELES_VISUALISATION_MINIMAP_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_2_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPair>


#include "util/sampling/isampler.h"

namespace veles {
namespace visualisation {

class VisualisationMinimap : public QOpenGLWidget,
                             protected QOpenGLFunctions_3_2_Core {
  Q_OBJECT

 public:
  enum class MinimapColor {
    RED = 0,
    GREEN,
    BLUE
  };

  enum class MinimapMode {
    VALUE,
    ENTROPY
  };

  explicit VisualisationMinimap(QWidget *parent = 0);
  ~VisualisationMinimap();

  void setSampler(util::ISampler * sampler);
  void setRange(size_t start, size_t end, bool reset_selection = true);
  QPair<size_t, size_t> getSelectedRange();
  void setSelectedRange(size_t start_address, size_t end_address);
  void refresh(bool has_context = false);

  void setMinimapColor(MinimapColor color);
  void setMinimapMode(MinimapMode mode);

 signals:
  void selectionChanged(size_t start, size_t end);

 protected:
  void mouseMoveEvent(QMouseEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;

  void updateLinePositions(bool keep_selection = false,
                           bool keep_size = false);

  void initializeGL() override;

  void resizeGL(int w, int h) override;
  void paintGL() override;

  void initShaders();
  void initTextures();
  void initGeometry();

 private:
  struct ScalingInfo {
    float line_width; // line width in GL coordinates
    float scale_factor; // minimap operates on (-1, 1) coordinates, but
                        // margins (line resting points) take some space,
                        // so actual GL coordinates need to be scaled by this
    float scaled_tlp; // top_line_positon_ scaled with scale_factor
    float scaled_blp; // bottom_line_pos_ scaled  with scale_factor
  };

  ScalingInfo calculateScaledPositions();
  float clickToTextureCoord(int y);
  void centerSelectionOnCoord(float coord);
  float normaliseLinePosition(float line);

  size_t lineToOffset(float line_position);
  float offsetToLine(size_t offset);

  static float* calculateAverageValueTexture(
      const uint8_t *sample, size_t sample_size,
      size_t texture_size, double point_size);
  static float* calculateEntropyTexture(
      const uint8_t *sample, size_t sample_size,
      size_t texture_size, double point_size);

  static float* calculateEntropyTexturePerPixel(
      const uint8_t *sample, size_t sample_size,
      size_t texture_size, double point_size);
  static float* calculateEntropyTextureSlidingWindow(
      const uint8_t *sample, size_t sample_size,
      size_t texture_size, double point_size);
  static float* calculateEntropyTextureSingleWindow(
      const uint8_t *sample, size_t sample_size,
      size_t texture_size, double point_size);

  bool empty();

  const int k_px_per_point = 1;
  const int k_round_size_to = 5;
  const int k_wheel_units_per_pixel = 6;
  const MinimapColor k_default_color = MinimapColor::GREEN;
  const MinimapMode k_default_mode = MinimapMode::VALUE;
  const float k_line_selection_epsilon = 0.003;
  const float k_minimum_line_distance = 0.02;
  static const int k_minimum_entropy_window = 256;
  const int k_bar_height = 7;
  const int k_bar_texture_width = 100;
  const float k_line_comparison_epsilon = 0.1;

  enum class DragState {NO_DRAG, TOP_LINE, BOTTOM_LINE, BOX};

  bool initialised_;
  bool gl_initialised_;
  util::ISampler *sampler_;

  size_t rows_, cols_, texture_rows_, texture_cols_;
  size_t selection_start_, selection_end_;
  size_t sample_size_;
  double point_size_;

  DragState drag_state_;
  float top_line_pos_, bottom_line_pos_;
  MinimapColor color_;
  MinimapMode mode_;

  QOpenGLShaderProgram program_, lines_program_, background_program_;
  QOpenGLTexture *texture_, *lines_texture_;

  QOpenGLBuffer square_vertex_;
  QOpenGLVertexArrayObject vao_;
};


}  //  namespace visualisation
}  //  namespace veles

#endif // VELES_VISUALISATION_MINIMAP_H
