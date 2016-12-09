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
#include "visualisation/trigram.h"

#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <functional>
#include <vector>

#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QSlider>
#include <QPixmap>
#include <QBitmap>
#include <QHBoxLayout>


namespace veles {
namespace visualisation {

const int k_minimum_brightness = 25;
const int k_maximum_brightness = 103;
const double k_brightness_heuristic_threshold = 0.66;
const int k_brightness_heuristic_min = 38;
const int k_brightness_heuristic_max = 66;
// decrease this to reduce noise (but you may lose data if you overdo it)
const double k_brightness_heuristic_scaling = 2.5;

TrigramWidget::TrigramWidget(QWidget *parent) :
  VisualisationWidget(parent), angle(0), c_sph(0), c_cyl(0), c_pos(0),
  shape_(EVisualisationShape::CUBE), mode_(EVisualisationMode::TRIGRAM),
  brightness_((k_maximum_brightness + k_minimum_brightness) / 2),
  brightness_slider_(nullptr), is_playing_(true),
  use_brightness_heuristic_(true) {}

TrigramWidget::~TrigramWidget() {
  makeCurrent();
  delete texture;
  delete databuf;
  doneCurrent();
}

void TrigramWidget::setBrightness(const int value) {
  brightness_ = value;
  c_brightness = static_cast<float>(value) * value * value;
  c_brightness /= getDataSize();
}

void TrigramWidget::setMode(EVisualisationMode mode, bool animate) {
  mode_ = mode;
  if (mode_ == EVisualisationMode::LAYERED_DIGRAM && !animate) {
    c_pos = 1;
  } else if (mode_ == EVisualisationMode::TRIGRAM && !animate) {
    c_pos = 0;
  }
}

void TrigramWidget::refresh() {
  if (use_brightness_heuristic_) {
    autoSetBrightness();
  }
  setBrightness(brightness_);
  makeCurrent();
  delete texture;
  delete databuf;
  initTextures();
  doneCurrent();
}

QIcon TrigramWidget::getColoredIcon(QString path, bool black_only) {
  QPixmap pixmap(path);
  QPixmap mask;
  if (black_only) {
    mask = pixmap.createMaskFromColor(QColor("black"), Qt::MaskOutColor);
  } else {
    mask = pixmap.createMaskFromColor(QColor("white"), Qt::MaskInColor);
  }
  pixmap.fill(palette().color(QPalette::WindowText));
  pixmap.setMask(mask);
  return QIcon(pixmap);
}

bool TrigramWidget::prepareOptionsPanel(QBoxLayout *layout) {
  VisualisationWidget::prepareOptionsPanel(layout);

  QLabel *brightness_label = new QLabel("Brightness: ");
  brightness_label->setAlignment(Qt::AlignTop);
  layout->addWidget(brightness_label);

  brightness_ = suggestBrightness();
  brightness_slider_ = new QSlider(Qt::Horizontal);
  brightness_slider_->setMinimum(k_minimum_brightness);
  brightness_slider_->setMaximum(k_maximum_brightness);
  brightness_slider_->setValue(brightness_);
  connect(brightness_slider_, SIGNAL(valueChanged(int)), this,
          SLOT(brightnessSliderMoved(int)));
  layout->addWidget(brightness_slider_);

  use_heuristic_checkbox_ = new QCheckBox(
    "Automatically adjust brightness");
  use_heuristic_checkbox_->setChecked(use_brightness_heuristic_);
  connect(use_heuristic_checkbox_, SIGNAL(stateChanged(int)),
          this, SLOT(setUseBrightnessHeuristic(int)));
  layout->addWidget(use_heuristic_checkbox_);

  pause_button_ = new QPushButton();
  pause_button_->setIcon(getColoredIcon(":/images/pause.png"));
  layout->addWidget(pause_button_);
  connect(pause_button_, SIGNAL(released()),
          this, SLOT(playPause()));

  QHBoxLayout *shape_box = new QHBoxLayout();
  cube_button_ = new QPushButton();
  cube_button_->setIcon(getColoredIcon(":/images/cube.png", false));
  cube_button_->setIconSize(QSize(32, 32));
  connect(cube_button_, &QPushButton::released,
          std::bind(&TrigramWidget::setShape, this,
                    EVisualisationShape::CUBE));
  shape_box->addWidget(cube_button_);

  cylinder_button_ = new QPushButton();
  cylinder_button_->setIcon(getColoredIcon(":/images/cylinder.png", false));
  cylinder_button_->setIconSize(QSize(32, 32));
  connect(cylinder_button_, &QPushButton::released,
          std::bind(&TrigramWidget::setShape, this,
                    EVisualisationShape::CYLINDER));
  shape_box->addWidget(cylinder_button_);

  sphere_button_ = new QPushButton();
  sphere_button_->setIcon(getColoredIcon(":/images/sphere.png"));
  sphere_button_->setIconSize(QSize(32, 32));

  connect(sphere_button_, &QPushButton::released,
          std::bind(&TrigramWidget::setShape, this,
                    EVisualisationShape::SPHERE));
  shape_box->addWidget(sphere_button_);

  layout->addLayout(shape_box);

  return true;
}

int TrigramWidget::suggestBrightness() {
  int size = getDataSize();
  auto data = reinterpret_cast<const uint8_t*>(getData());
  if (size < 100) {
    return (k_minimum_brightness + k_maximum_brightness) / 2;
  }
  std::vector<uint64_t> counts(256, 0);
  for (int i = 0; i < size; ++i) {
    counts[data[i]] += 1;
  }
  std::sort(counts.begin(), counts.end());
  int offset = 0, sum = 0;
  while (offset < 255 && sum < k_brightness_heuristic_threshold * size) {
    sum += counts[255 - offset];
    offset += 1;
  }
  offset = static_cast<int>(static_cast<double>(offset)
                            / k_brightness_heuristic_scaling);
  return std::max(k_brightness_heuristic_min,
                  k_brightness_heuristic_max - offset);
}

void TrigramWidget::playPause() {
  QPixmap pixmap;
  if (is_playing_) {
    pause_button_->setIcon(getColoredIcon(":/images/play.png"));
  } else {
    pause_button_->setIcon(getColoredIcon(":/images/pause.png"));
  }
  is_playing_ = !is_playing_;
}

void TrigramWidget::setShape(EVisualisationShape shape) {
  shape_ = shape;
}

void TrigramWidget::brightnessSliderMoved(int value) {
  if (value == brightness_) return;
  use_brightness_heuristic_ = false;
  use_heuristic_checkbox_->setChecked(false);
  setBrightness(value);
}

void TrigramWidget::setUseBrightnessHeuristic(int state) {
  use_brightness_heuristic_ = state;
  if (use_brightness_heuristic_) {
    autoSetBrightness();
  }
}

void TrigramWidget::autoSetBrightness() {
  auto new_brightness = suggestBrightness();
  if (new_brightness == brightness_) return;
  brightness_ = new_brightness;
  if (brightness_slider_ != nullptr) {
    brightness_slider_->setValue(brightness_);
  }
  setBrightness(brightness_);
}

void TrigramWidget::timerEvent(QTimerEvent *e) {
  if (is_playing_) {
    angle += 0.5f;
  }

  if (shape_ == EVisualisationShape::CYLINDER) {
    c_cyl += 0.01;
  } else {
    c_cyl -= 0.01;
  }
  if (shape_ == EVisualisationShape::SPHERE) {
    c_sph += 0.01;
  } else {
    c_sph -= 0.01;
  }
  if (c_cyl > 1) c_cyl = 1;
  if (c_cyl < 0) c_cyl = 0;
  if (c_sph > 1) c_sph = 1;
  if (c_sph < 0) c_sph = 0;

  if (mode_ == EVisualisationMode::LAYERED_DIGRAM && c_pos < 1) {
    c_pos += 0.01;
    if (c_pos > 1) c_pos = 1;
  }
  if (mode_ != EVisualisationMode::LAYERED_DIGRAM && c_pos) {
    c_pos -= 0.01;
    if (c_pos < 0) c_pos = 0;
  }
  update();
}

void TrigramWidget::initializeVisualisationGL() {
  initializeOpenGLFunctions();

  glClearColor(0, 0, 0, 1);

  autoSetBrightness();

  initShaders();
  initTextures();
  initGeometry();
  setBrightness(brightness_);
}

void TrigramWidget::initShaders() {
  if (!program.addShaderFromSourceFile(QOpenGLShader::Vertex,
                                       ":/trigram/vshader.glsl"))
    close();

  // Compile fragment shader
  if (!program.addShaderFromSourceFile(QOpenGLShader::Fragment,
                                       ":/trigram/fshader.glsl"))
    close();

  // Link shader pipeline
  if (!program.link()) close();

  timer.start(12, this);
}

void TrigramWidget::initTextures() {
  int size = getDataSize();
  const uint8_t *data = reinterpret_cast<const uint8_t*>(getData());

  databuf = new QOpenGLBuffer(QOpenGLBuffer::Type(GL_TEXTURE_BUFFER));
  databuf->create();
  databuf->bind();
  databuf->allocate(data, size);
  databuf->release();

  texture = new QOpenGLTexture(QOpenGLTexture::TargetBuffer);
  texture->setSize(size);
  texture->setFormat(QOpenGLTexture::R8U);
  texture->create();
  texture->bind();
  glTexBuffer(GL_TEXTURE_BUFFER, QOpenGLTexture::R8U, databuf->bufferId());
}

void TrigramWidget::initGeometry() {
  vao.create();
}

void TrigramWidget::resizeGL(int w, int h) {
  width = w;
  height = h;
}

void TrigramWidget::paintGL() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_BLEND);
  glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ONE);
  unsigned size = getDataSize();

  program.bind();
  texture->bind();
  vao.bind();

  QMatrix4x4 mp, m;
  mp.setToIdentity();
  if (width > height) {
    mp.perspective(45, static_cast<double>(width) / height, 0.01f, 100.0f);
  } else {
    // wtf h4x.  gluPerspective fixes the viewport wrt y field of view, and
    // we need to fix it to x instead.  So, rotate the world, fix to new y,
    // rotate it back.
    mp.rotate(90, 0, 0, 1);
    mp.perspective(45, static_cast<double>(height) / width, 0.01f, 100.0f);
    mp.rotate(-90, 0, 0, 1);
  }
  m.setToIdentity();
  m.translate(0.0f, 0.0f, -5.0f);
  m.rotate(90, 0, 0, 1);
  m.rotate(angle, 0.5, 1, 0);
  int loc_sz = program.uniformLocation("sz");
  program.setUniformValue("tx", 0);
  program.setUniformValue("c_cyl", c_cyl);
  program.setUniformValue("c_sph", c_sph);
  program.setUniformValue("c_pos", c_pos);
  program.setUniformValue("xfrm", mp * m);
  program.setUniformValue("c_brightness", c_brightness);
  glUniform1ui(loc_sz, size);
  glDrawArrays(GL_POINTS, 0, size - 2);
}

}  // namespace visualisation
}  // namespace veles
