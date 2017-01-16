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
#include "visualisation/minimap.h"
#include <QImage>
#include <cstdlib>
#include <cmath>
#include <assert.h>


namespace veles {
namespace visualisation {

VisualisationMinimap::VisualisationMinimap(QWidget *parent) :
  QOpenGLWidget(parent), initialised_(false), gl_initialised_(false),
  sampler_(nullptr), rows_(0), cols_(0), selection_start_(0),
  selection_end_(0), top_line_pos_(1.0), bottom_line_pos_(-1.0),
  color_(k_default_color), mode_(k_default_mode), texture_(nullptr),
  lines_texture_(nullptr) {}

VisualisationMinimap::~VisualisationMinimap() {
  if (gl_initialised_) {
    makeCurrent();
    delete texture_;
    delete lines_texture_;
    square_vertex_.destroy();
    doneCurrent();
  }
}

void VisualisationMinimap::setSampler(util::ISampler *sampler) {
  sampler_ = sampler;
  selection_start_ = 0;
  selection_end_ = (empty()) ? 0 : sampler_->getSampleSize() - 1;
  initialised_ = true;
  refresh();
}

QPair<size_t, size_t> VisualisationMinimap::getSelectedRange() {
  if (empty()) return qMakePair(0, 0);
  size_t start = sampler_->getFileOffset(selection_start_);
  size_t end = sampler_->getFileOffset(selection_end_);
  return qMakePair(start, end);
}

void VisualisationMinimap::setSelectedRange(size_t start_address, size_t end_address) {
  selection_start_ = sampler_->getSampleOffset(start_address);
  selection_end_ = sampler_->getSampleOffset(end_address - 1);
  if (gl_initialised_) {
    top_line_pos_ = normaliseLinePosition(offsetToLine(selection_start_));
    bottom_line_pos_ = normaliseLinePosition(offsetToLine(selection_end_));
    update();
  }
  emit selectionChanged(start_address, end_address);
}

void VisualisationMinimap::refresh(bool has_context) {
  if (!initialised_ || !gl_initialised_) return;
  if (!has_context) makeCurrent();
  delete lines_texture_;
  delete texture_;
  initTextures();
  if (!has_context) doneCurrent();
  update();
}

void VisualisationMinimap::setRange(size_t start, size_t end,
                                    bool reset_selection) {
  assert(!empty());
  sampler_->setRange(start, end);
  sample_size_ = sampler_->getSampleSize();
  if (reset_selection) {
    selection_start_ = 0;
    selection_end_ = sample_size_ - 1;
    top_line_pos_ = 1.0;
    bottom_line_pos_ = -1.0;
    refresh();
  } else {
    refresh();
    selection_start_ = lineToOffset(top_line_pos_);
    selection_end_ = lineToOffset(bottom_line_pos_);
    top_line_pos_ = normaliseLinePosition(top_line_pos_);
    bottom_line_pos_ = normaliseLinePosition(bottom_line_pos_);
    updateLinePositions();
  }
  emit selectionChanged(sampler_->getFileOffset(selection_start_),
                        sampler_->getFileOffset(selection_end_));
}

void VisualisationMinimap::setMinimapColor(MinimapColor color) {
  color_ = color;
  update();
}

void VisualisationMinimap::setMinimapMode(MinimapMode mode) {
  mode_ = mode;
  refresh();
}

/*****************************************************************************/
/* calculate minimap texture methods */
/*****************************************************************************/

float* VisualisationMinimap::calculateAverageValueTexture(
              const uint8_t *sample, size_t sample_size,
              size_t texture_size, double point_size) {
  auto bigtab = new float[texture_size];
  memset(bigtab, 0, texture_size * sizeof(*bigtab));
  uint64_t point_sum = 0, point_count = 0;

  size_t index = 0;
  for (size_t i = 0; i < sample_size; ++i) {
    point_sum += sample[i];
    point_count += 1;
    if (static_cast<double>(i) / point_size >= index + 1) {
      if (index == texture_size - 1 && i < sample_size - 1) continue;
      uint8_t result = (point_count == 0) ? 0 : point_sum / point_count;
      bigtab[index] = static_cast<float>(result); // HAX
      index += 1;
      point_sum = 0;
      point_count = 0;
    }
  }
  return bigtab;
}

float* VisualisationMinimap::calculateEntropyTexture(
              const uint8_t *sample, size_t sample_size,
              size_t texture_size, double point_size) {
  if (point_size > k_minimum_entropy_window) {
    return calculateEntropyTexturePerPixel(sample, sample_size,
                                           texture_size, point_size);
  }
  if (sample_size < 2 * k_minimum_entropy_window) {
    return calculateEntropyTextureSingleWindow(sample, sample_size,
                                               texture_size, point_size);
  }
  return calculateEntropyTextureSlidingWindow(sample, sample_size,
                                              texture_size, point_size);
}

float* VisualisationMinimap::calculateEntropyTexturePerPixel(
              const uint8_t *sample, size_t sample_size,
              size_t texture_size, double point_size) {
  auto bigtab = new float[texture_size];
  memset(bigtab, 0, texture_size * sizeof(*bigtab));

  auto counts = new uint64_t[256]; // assume 8-bit bytes
  memset(counts, 0, 256 * sizeof(*counts));

  size_t index = 0, point_count = 0;

  for (size_t i = 0; i < sample_size; ++i) {
    counts[sample[i]] += 1;
    point_count += 1;
    if (static_cast<double>(i) / point_size >= index + 1) {
      if (index == texture_size - 1 && i < sample_size - 1) continue;
      float entropy = 0.0f;
      for (int i = 0; i < 256; ++i) {
        if (counts[i] > 0) {
          float fcounts = static_cast<float>(counts[i]) / point_count;
          entropy -= fcounts * log2(fcounts);
        }
      }
      entropy *= 32; // 256 / 8 (entropy will be in range [0,8], scale it
      bigtab[index] = entropy;
      index += 1;
      point_count = 0;
      memset(counts, 0, 256 * sizeof(*counts));
    }
  }
  delete[] counts;
  return bigtab;
}

float* VisualisationMinimap::calculateEntropyTextureSlidingWindow(
              const uint8_t *sample, size_t sample_size,
              size_t texture_size, double point_size) {
  auto bigtab = new float[texture_size];
  memset(bigtab, 0, texture_size * sizeof(*bigtab));

  auto counts = new uint64_t[256]; // assume 8-bit bytes
  memset(counts, 0, 256 * sizeof(*counts));

  size_t start = 0, end = 0;
  while (start < sample_size) {
    size_t mid = (start + end) / 2;
    if (mid > 0 && std::floor(mid / point_size) != std::floor((mid - 1) / point_size)) {
      float entropy = 0.0f;
      size_t point_count = end - start;
      for (int i = 0; i < 256; ++i) {
        if (counts[i] > 0) {
          float fcounts = static_cast<float>(counts[i]) / point_count;
          entropy -= fcounts * log2(fcounts);
        }
      }
      entropy *= 32; // 256 / 8 (entropy will be in range [0,8], scale it
      bigtab[static_cast<size_t>(mid / point_size)] = entropy;
    }

    if (end > k_minimum_entropy_window || end >= sample_size) {
      counts[sample[start++]] -= 1;
    }
    if (end < sample_size) {
      counts[sample[end++]] += 1;
    }
  }
  delete[] counts;
  return bigtab;
}

float* VisualisationMinimap::calculateEntropyTextureSingleWindow(
              const uint8_t *sample, size_t sample_size,
              size_t texture_size, double point_size) {
  auto bigtab = new float[texture_size];
  memset(bigtab, 0, texture_size * sizeof(*bigtab));

  auto counts = new uint64_t[256]; // assume 8-bit bytes
  memset(counts, 0, 256 * sizeof(*counts));

  for (size_t i = 0; i < sample_size; ++i) {
    counts[sample[i]] += 1;
  }

  uint64_t point_count = 0;
  float point_sum = 0;

  size_t index = 0;
  for (size_t i = 0; i < sample_size; ++i) {
    point_sum -= log2(static_cast<float>(counts[sample[i]]) / sample_size);
    point_count += 1;
    if (static_cast<double>(i) / point_size >= index + 1) {
      if (index == texture_size - 1 && i < sample_size - 1) continue;
      float result = (point_count == 0) ? 0.0f : point_sum / point_count;
      bigtab[index] = static_cast<float>(result) * 32;  // Normalise to 0-256
      index += 1;
      point_sum = 0;
      point_count = 0;
    }
  }
  delete[] counts;
  return bigtab;
}

/*****************************************************************************/
/* OpenGL methods */
/*****************************************************************************/

void VisualisationMinimap::initializeGL() {
  if (!gl_initialised_) {
    initializeOpenGLFunctions();
    glClearColor(0, 0, 0, 1);
    initShaders();
    initGeometry();
    gl_initialised_ = true;
  }
}

void VisualisationMinimap::initShaders() {
  program_.addShaderFromSourceFile(QOpenGLShader::Vertex,
                                   ":/minimap/vshader.glsl");
  program_.addShaderFromSourceFile(QOpenGLShader::Fragment,
                                   ":/minimap/fshader.glsl");
  program_.link();

  lines_program_.addShaderFromSourceFile(QOpenGLShader::Vertex,
                                         ":/minimap/lines_vshader.glsl");
  lines_program_.addShaderFromSourceFile(QOpenGLShader::Fragment,
                                         ":/minimap/lines_fshader.glsl");
  lines_program_.link();

  background_program_.addShaderFromSourceFile(QOpenGLShader::Vertex,
                                         ":/minimap/background_vshader.glsl");
  background_program_.addShaderFromSourceFile(QOpenGLShader::Fragment,
                                         ":/minimap/background_fshader.glsl");
  background_program_.link();
}

void VisualisationMinimap::initGeometry() {
  square_vertex_.create();
  QVector2D v[] = {
      {0, 0}, {0, 1}, {1, 0}, {1, 1},
  };
  square_vertex_.bind();
  square_vertex_.allocate(v, sizeof v);
  vao_.create();
}

void VisualisationMinimap::initTextures() {
  if (!initialised_) return;

  lines_texture_ = new QOpenGLTexture(QImage(":/images/bar.png"));
  lines_texture_->setWrapMode(QOpenGLTexture::ClampToEdge);

  if (empty()) return;

  // calculate texture size
  texture_rows_ = std::max(static_cast<size_t>(1), rows_);
  texture_cols_ = std::max(static_cast<size_t>(1), cols_);
  sample_size_ = sampler_->getSampleSize();
  size_t texture_size = texture_rows_ * texture_cols_;

  if (sample_size_ < texture_size) {
    float scale_factor = std::sqrt(static_cast<float>(sample_size_) /
                                   static_cast<float>(texture_size));
    texture_rows_ = std::max(static_cast<size_t>(1),
                             static_cast<size_t>(rows_ * scale_factor));
    texture_cols_ = std::max(static_cast<size_t>(1),
                             static_cast<size_t>(cols_ * scale_factor));
    texture_size = texture_rows_ * texture_cols_;
  }

  texture_ = new QOpenGLTexture(QOpenGLTexture::Target2D);
  texture_->setSize(texture_cols_, texture_rows_);
  // TODO(Maciek): WTF HAX
  // I really want to use GL_R8UI here, but for some reasone it doesn't work
  // so using float32 as workaround
  // texture_->setFormat(QOpenGLTexture::R8U);
  texture_->setFormat(QOpenGLTexture::R32F);
  texture_->allocateStorage();

  point_size_ = std::max(1.0, static_cast<double>(sample_size_) / texture_size);
  const uint8_t *rowdata = reinterpret_cast<const uint8_t *>(sampler_->data());

  float* bigtab;
  if (mode_ == MinimapMode::VALUE) {
    bigtab = calculateAverageValueTexture(rowdata, sample_size_,
                                          texture_size, point_size_);
  } else {
    bigtab = calculateEntropyTexture(rowdata, sample_size_,
                                     texture_size, point_size_);
  }

  texture_->setData(QOpenGLTexture::Red, QOpenGLTexture::Float32,
                    reinterpret_cast<void *>(bigtab));
  texture_->generateMipMaps();
  texture_->setMinificationFilter(QOpenGLTexture::Nearest);
  texture_->setMagnificationFilter(QOpenGLTexture::Nearest);
  texture_->setWrapMode(QOpenGLTexture::ClampToEdge);

  delete[] bigtab;
}

void VisualisationMinimap::resizeGL(int w, int h) {
  if (empty()) return;
  size_t rows = (h / k_round_size_to) * k_round_size_to / k_px_per_point;
  size_t cols = (w / k_round_size_to) * k_round_size_to / k_px_per_point;
  if (rows != rows_ || cols != cols_) {
    rows_ = rows;
    cols_ = cols;
    refresh(true);
    top_line_pos_ = normaliseLinePosition(offsetToLine(selection_start_));
    bottom_line_pos_ = normaliseLinePosition(offsetToLine(selection_end_));
    updateLinePositions(true);
  }
}

void VisualisationMinimap::paintGL() {
  if (!initialised_ || !gl_initialised_ || empty()) return;

  auto pos_info = calculateScaledPositions();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  vao_.bind();

  // Render top / bottom margins (bar "resting points")
  background_program_.bind();
  QVector2D top_margin_coords[] = {
    {-1.0f, 1.0f}, {1.0f, 1.0f},
    {-1.0f, 1.0f - pos_info.line_width}, {1.0f, 1.0f - pos_info.line_width}
  };
  QVector2D bottom_margin_coords[] = {
    {-1.0f, -1.0f}, {1.0f, -1.0f},
    {-1.0f, -1.0f + pos_info.line_width}, {1.0f, -1.0f + pos_info.line_width}
  };

  int bg_red, bg_green, bg_blue;
  palette().color(QPalette::Button).getRgb(&bg_red, &bg_green, &bg_blue);
  background_program_.setUniformValue("bg_red", bg_red);
  background_program_.setUniformValue("bg_green", bg_green);
  background_program_.setUniformValue("bg_blue", bg_blue);
  background_program_.setUniformValueArray("coords", top_margin_coords, 4);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  background_program_.setUniformValueArray("coords", bottom_margin_coords, 4);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  // Render minimap
  program_.bind();
  texture_->bind();

  square_vertex_.bind();
  int vertexLocation = program_.attributeLocation("a_position");
  program_.enableAttributeArray(vertexLocation);
  program_.setAttributeBuffer(vertexLocation, GL_FLOAT, 0, 2,
                              sizeof(QVector2D));
  program_.setUniformValue("scale_factor", pos_info.scale_factor);
  program_.setUniformValue("tx", 0);
  program_.setUniformValue("top_line_pos", (1 + (-1.0f * top_line_pos_)) / 2);
  program_.setUniformValue("bottom_line_pos", (1 + (-1.0f * bottom_line_pos_)) / 2);
  program_.setUniformValue("channel", static_cast<GLuint>(color_));
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

  // Render selection bars
  lines_program_.bind();
  lines_texture_->bind();
  lines_program_.setUniformValue("tx", 0);
  float l_texture_scaling = (static_cast<float>(cols_) / k_bar_texture_width) / 2;
  QVector2D texture_coords[] = {
    {0.5f - l_texture_scaling, 0.0f}, {0.5f + l_texture_scaling, 0.0f},
    {0.5f - l_texture_scaling, 1.0f}, {0.5f + l_texture_scaling, 1.0f}
  };
  QVector2D top_line_coords[] = {
    {-1.0f, pos_info.scaled_tlp + pos_info.line_width},
    {1.0f, pos_info.scaled_tlp + pos_info.line_width},
    {-1.0f, pos_info.scaled_tlp},
    {1.0f, pos_info.scaled_tlp}
  };
  QVector2D bottom_line_coords[] = {
    {-1.0f, pos_info.scaled_blp},
    {1.0f, pos_info.scaled_blp},
    {-1.0f, pos_info.scaled_blp - pos_info.line_width},
    {1.0f, pos_info.scaled_blp - pos_info.line_width}
  };

  lines_program_.setUniformValueArray("coords", top_line_coords, 4);
  lines_program_.setUniformValueArray("tcoords", texture_coords, 4);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  lines_program_.setUniformValueArray("coords", bottom_line_coords, 4);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

/*****************************************************************************/
/* Drag&drop methods */
/*****************************************************************************/

float VisualisationMinimap::clickToTextureCoord(int y) {
  return -1 * (static_cast<float>(y) / rows_ * 2 - 1);
}

void VisualisationMinimap::centerSelectionOnCoord(float coord) {
  float window_size = top_line_pos_ - bottom_line_pos_;
  top_line_pos_ = normaliseLinePosition(coord + window_size / 2);
  bottom_line_pos_ = top_line_pos_ - window_size;
  if (top_line_pos_ > 1) {
    bottom_line_pos_ -= top_line_pos_ - 1;
    top_line_pos_ = 1;
  }
  if (bottom_line_pos_ < -1) {
    top_line_pos_ -= bottom_line_pos_ + 1;
    bottom_line_pos_ = -1;
  }
}

void VisualisationMinimap::mouseMoveEvent(QMouseEvent *event){
  if (empty()) return;
  if (!event->buttons().testFlag(Qt::LeftButton)) return;
  float position = clickToTextureCoord(event->y());
  switch (drag_state_) {
  case DragState::NO_DRAG:
    return;
  case DragState::TOP_LINE:
    top_line_pos_ = normaliseLinePosition(position);
    break;
  case DragState::BOTTOM_LINE:
    bottom_line_pos_ = normaliseLinePosition(position);
    break;
  case DragState::BOX:
    centerSelectionOnCoord(position);
    updateLinePositions(false, true);
    return;
  }
  updateLinePositions();
}

void VisualisationMinimap::mousePressEvent(QMouseEvent *event) {
  if (empty()) return;
  if (event->button() != Qt::LeftButton) return;
  float position = clickToTextureCoord(event->y());
  auto pos_info = calculateScaledPositions();
  // make bar hitbox slightly larger than bar itself
  float tlp_bottom = pos_info.scaled_tlp - k_line_selection_epsilon,
        tlp_top = pos_info.scaled_tlp + pos_info.line_width
                  + k_line_selection_epsilon,
        blp_top = pos_info.scaled_blp + k_line_selection_epsilon,
        blp_bottom = pos_info.scaled_blp - pos_info.line_width
                     - k_line_selection_epsilon;
  bool grab_top = position >= tlp_bottom && position <= tlp_top,
    grab_bot = position >= blp_bottom && position <= blp_top,
    between = position < tlp_bottom && position > blp_top;

  if (grab_top) {
    drag_state_ = DragState::TOP_LINE;
    return;
  }
  if (grab_bot) {
    drag_state_ = DragState::BOTTOM_LINE;
    return;
  }
  if (between) {
    drag_state_ = DragState::BOX;
    return;
  }
  centerSelectionOnCoord(position);
  drag_state_ = DragState::NO_DRAG;
  updateLinePositions(false, true);
}

void VisualisationMinimap::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() != Qt::LeftButton) return;
  drag_state_ = DragState::NO_DRAG;
}

void VisualisationMinimap::wheelEvent(QWheelEvent *event) {
  if (empty()) return;
  if (drag_state_ != DragState::NO_DRAG) {
    // we're already dragging the box, let's ignore the wheel
    event->accept();
    return;
  }
  auto pixel_delta = event->pixelDelta();
  auto angle_delta = event->angleDelta();
  int pixels = 0;
  if (!pixel_delta.isNull()) {
    pixels = pixel_delta.y();
  } else if (!angle_delta.isNull()) {
    pixels = angle_delta.y() / k_wheel_units_per_pixel;
  }

  size_t abs_pixels = static_cast<size_t>(std::abs(pixels));
  if (abs_pixels > 0 && abs_pixels < rows_ / texture_rows_) {
    pixels = std::copysign(rows_ / texture_rows_, pixels);
  }
  float position = (top_line_pos_ + bottom_line_pos_) / 2;
  float position_delta = static_cast<float>(2 * pixels) / rows_;

  centerSelectionOnCoord(position + position_delta);
  updateLinePositions(false, true);

  event->accept();
}

void VisualisationMinimap::updateLinePositions(bool keep_selection,
                                               bool keep_size) {
  assert(!empty());
  float row_size = 2.0 / texture_rows_;
  float minimum_distance = std::max(k_minimum_line_distance, row_size);

  top_line_pos_ = std::max(top_line_pos_,
                           bottom_line_pos_ + minimum_distance);
  top_line_pos_ = std::min(top_line_pos_, 1.0f);

  bottom_line_pos_ = std::min(bottom_line_pos_,
                              top_line_pos_ - minimum_distance);
  bottom_line_pos_ = std::max(bottom_line_pos_, -1.0f);

  update();

  // this is rather heuristic, but try to avoid changing selection
  // if we can do it without getting too far out of touch with line positions
  if (keep_selection) {
    float raw_top_position = offsetToLine(selection_start_);
    float raw_bot_position = offsetToLine(selection_end_);
    if (selection_end_ > selection_start_ &&
        raw_top_position + k_line_comparison_epsilon > top_line_pos_ &&
        raw_top_position - k_line_comparison_epsilon < top_line_pos_ &&
        raw_bot_position + k_line_comparison_epsilon > bottom_line_pos_ &&
        raw_bot_position - k_line_comparison_epsilon < bottom_line_pos_) {
      return;
    }
  }

  size_t start = lineToOffset(top_line_pos_);
  size_t end = lineToOffset(bottom_line_pos_);

  if (start != selection_start_ || end != selection_end_) {

    // when moving a window we want to keep constant selection size,
    // even if it doesn't exactly fit to where line is
    if (keep_size) {
      size_t size = selection_end_ - selection_start_;
      assert(size <= sample_size_);
      size_t middle = (start + end) / 2;

      if (size / 2 > middle) {
        // oops, it's unsigned, we don't want to overflow it
        start = 0;
      } else {
        start = middle - (size / 2);
      }
      end = start + size;
      if (end >= sample_size_) {
        end = sample_size_ - 1;
        start = end - size;
      }
    }

    // this has been checked before, but may no longer be true if we got into
    // keep_size logic
    if (start != selection_start_ || end != selection_end_) {
      selection_start_ = start;
      selection_end_ = end;

      emit selectionChanged(sampler_->getFileOffset(start),
                            sampler_->getFileOffset(end));
    }
  }
}

/*****************************************************************************/
/* Helper methods */
/*****************************************************************************/

VisualisationMinimap::ScalingInfo
VisualisationMinimap::calculateScaledPositions() {
  VisualisationMinimap::ScalingInfo info;
  info.line_width = (2.0 / rows_) * k_bar_height;
  info.scale_factor = 1.0 - info.line_width;
  info.scaled_tlp = info.scale_factor * top_line_pos_;
  info.scaled_blp = info.scale_factor * bottom_line_pos_;
  return info;
}

float VisualisationMinimap::normaliseLinePosition(float line) {
  float row_size = 2.0 / texture_rows_;
  float result = row_size * std::round((line + 1.0f) / row_size) - 1;
  return std::max(std::min(1.0f, result), -1.0f);
}

size_t VisualisationMinimap::lineToOffset(float line_position) {
  assert(line_position >= -1.0);
  assert(line_position <= 1.0);
  float row_size = 2.0 / texture_rows_;
  size_t row = texture_rows_ - ((line_position + 1) / row_size);
  if (row == texture_rows_) {
    return sample_size_ - 1;
  }
  size_t row_bytes = point_size_ * texture_cols_;
  return std::min(sample_size_ - 1, row_bytes * row);
}

float VisualisationMinimap::offsetToLine(size_t offset) {
  float row_size = 2.0 / texture_rows_;
  size_t row_bytes = point_size_ * texture_cols_;
  float position = -1.0 * (((offset / row_bytes) * row_size) - 1);
  return std::min(1.0f, std::max(-1.0f, position));
}

bool VisualisationMinimap::empty() {
  return (sampler_ == nullptr || sampler_->empty());
}

}  // namespace visualisation
}  // namespace veles
