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
#include <cmath>

#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QSlider>
#include <QPixmap>
#include <QBitmap>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QToolButton>

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
    VisualisationWidget(parent), texture(nullptr), databuf(nullptr), angle(0),
    c_sph(0), c_cyl(0), c_pos(0), shape_(EVisualisationShape::CUBE),
    mode_(EVisualisationMode::TRIGRAM),
    brightness_((k_maximum_brightness + k_minimum_brightness) / 2),
    pause_button_(nullptr), brightness_slider_(nullptr), is_playing_(true),
    use_brightness_heuristic_(true), show_labels_and_rf_(true) {
  manipulators_.push_back(spin_manipulator_ = new SpinManipulator(this));
  manipulators_.push_back(trackball_manipulator_ = new TrackballManipulator(this));
  manipulators_.push_back(free_manipulator_ = new FreeManipulator(this));
  current_manipulator_ = nullptr;
  setManipulator(manipulators_.front());
  time_.start();
  setFocusPolicy(Qt::StrongFocus);
}

TrigramWidget::~TrigramWidget() {
  if (texture == nullptr && databuf == nullptr) return;
  makeCurrent();
  delete texture;
  delete databuf;
  releaseLabels();
  releaseRF();
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

float TrigramWidget::vfovDeg(float min_fov_deg, float aspect_ratio) {
  if(aspect_ratio >= 1.f) {
    return min_fov_deg;
  }

  static float deg2rad = std::acos(-1.f) / 180.f;
  float min_fov = deg2rad * min_fov_deg;
  float vfov = 2.f * std::atan(std::tan(min_fov * .5f) / aspect_ratio);
  return vfov / deg2rad;
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
  prepareManipulatorToolbar(layout);

  show_labels_and_rf_checkbox_ = new QCheckBox(tr("Show reference frame"));
  show_labels_and_rf_checkbox_->setChecked(show_labels_and_rf_);
  layout->addWidget(show_labels_and_rf_checkbox_);
  connect(show_labels_and_rf_checkbox_, &QCheckBox::toggled,
      [this](bool toggled){show_labels_and_rf_ = toggled;});

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

void TrigramWidget::setManipulator(Manipulator* manipulator) {
  if(manipulator == current_manipulator_) {
    return;
  }

  for (auto m : manipulators_) {
    removeEventFilter(m);
  }

  installEventFilter(manipulator);
  if (current_manipulator_ != nullptr && manipulator != nullptr) {
    manipulator->initFromMatrix(current_manipulator_->transform());
  }
  current_manipulator_ = manipulator;

  if(!is_playing_) {
    playPause();
  }

  if (pause_button_) {
    pause_button_->setEnabled(current_manipulator_->handlesPause());
  }

  emit manipulatorChanged(current_manipulator_);
  setFocus();
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

bool TrigramWidget::event(QEvent *event) {
  if(event->type() == QEvent::MouseMove) {
    QMouseEvent* mouse_event = static_cast<QMouseEvent*>(event);
    if ((mouse_event->buttons() & Qt::LeftButton)
        && current_manipulator_ == spin_manipulator_) {
      setManipulator(trackball_manipulator_);
      trackball_manipulator_->processEvent(this, event);
    }
  } else if (event->type() == QEvent::KeyPress) {
    QKeyEvent* key_event = static_cast<QKeyEvent*>(event);
    if ((current_manipulator_ == spin_manipulator_
        || current_manipulator_ == trackball_manipulator_)
        && FreeManipulator::isCtrlButton(key_event->key())) {
      setManipulator(free_manipulator_);
      trackball_manipulator_->processEvent(this, event);
    }
  }

  return QWidget::event(event);
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

bool TrigramWidget::initializeVisualisationGL() {
  if (!initializeOpenGLFunctions()) return false;

  glClearColor(0, 0, 0, 1);
  glEnable(GL_DEPTH_TEST);

  autoSetBrightness();

  initShaders();
  initTextures();
  initGeometry();
  setBrightness(brightness_);

  initLabels();
  initRF();

  return true;
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

QAction* TrigramWidget::createAction(const QIcon& icon,
      Manipulator* manipulator, const QList<QKeySequence>& sequences) {
  QAction* action = new QAction(icon, manipulator->manipulatorName(), this);
  action->setShortcuts(sequences);
  action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
  connect(action, &QAction::triggered, std::bind(
      &TrigramWidget::setManipulator, this, manipulator));
  action->setProperty("manipulator", QVariant(qintptr(manipulator)));
  return action;
}

QAbstractButton* TrigramWidget::createActionButton(QAction* action) {
  QToolButton* button = new QToolButton;
  button->setAutoRaise(true);
  button->setIcon(action->icon());
  button->setToolTip(action->text());
  button->setCheckable(true);
  button->setIconSize(QSize(64, 64));
  button->setAutoExclusive(true);
  button->setProperty("action", QVariant(qintptr(action)));
  connect(button, &QPushButton::toggled, [button](bool toggled){
        if(toggled) {
          QAction* action =
              reinterpret_cast<QAction*>(
              qvariant_cast<qintptr>(
              button->property("action")));
          if(action) {
            action->trigger();
          }
        }
      });
  connect(this, &TrigramWidget::manipulatorChanged, [action, button](
      Manipulator* new_manipulator){
        Manipulator* manipulator =
            reinterpret_cast<Manipulator*>(
            qvariant_cast<qintptr>(
            action->property("manipulator")));
        if(manipulator == new_manipulator) {
          button->setChecked(true);
        }
      });
  return button;
}

void TrigramWidget::prepareManipulatorToolbar(QBoxLayout *layout) {
  QGroupBox* group = new QGroupBox;
  QHBoxLayout *group_layout = new QHBoxLayout;
  group->setTitle(tr("Camera manipulators"));
  group->setLayout(group_layout);

  {
    QAction* action = createAction(
        QIcon(":/images/manipulator_spin.png"), spin_manipulator_,
        {QKeySequence(Qt::CTRL + Qt::Key_1), QKeySequence(Qt::Key_Escape)});
    addAction(action);
    QAbstractButton* button = createActionButton(action);
    group_layout->addWidget(button);
    button->setChecked(true);
  }

  {
    QAction* action = createAction(
        QIcon(":/images/manipulator_trackball.png"), trackball_manipulator_,
        {QKeySequence(Qt::CTRL + Qt::Key_2)});
    addAction(action);
    QAbstractButton* button = createActionButton(action);
    group_layout->addWidget(button);
  }

  {
    QAction* action = createAction(
        QIcon(":/images/manipulator_free.png"), free_manipulator_,
        {QKeySequence(Qt::CTRL + Qt::Key_3)});
    addAction(action);
    QAbstractButton* button = createActionButton(action);
    group_layout->addWidget(button);
  }

  layout->addWidget(group);
}

void TrigramWidget::resizeGLImpl(int w, int h) {
  width = w;
  height = h;
}

void TrigramWidget::paintGLImpl() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_BLEND);
  glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ONE);
  glDepthFunc(GL_ALWAYS);
  unsigned size = getDataSize();

  program.bind();
  texture->bind();
  vao.bind();

  QMatrix4x4 mp, m;
  mp.setToIdentity();
  float aspect_ratio = static_cast<float>(width) / height;
  mp.perspective(vfovDeg(45.f, aspect_ratio), aspect_ratio, 0.01f, 100.0f);

  m.setToIdentity();
  float dt = static_cast<float>(time_.restart()) / 1000.f;
  if (current_manipulator_) {
    if (is_playing_ || !current_manipulator_->handlesPause()) {
      current_manipulator_->update(dt);
    }
    m = current_manipulator_->transform();
  }

  int loc_sz = program.uniformLocation("sz");
  program.setUniformValue("tx", 0);
  program.setUniformValue("c_cyl", c_cyl);
  program.setUniformValue("c_sph", c_sph);
  program.setUniformValue("c_pos", c_pos);
  program.setUniformValue("xfrm", mp * m);
  program.setUniformValue("c_brightness", c_brightness);
  glUniform1ui(loc_sz, size);
  glDrawArrays(GL_POINTS, 0, size - 2);

  vao.release();
  texture->release();
  program.release();

  if (show_labels_and_rf_) {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    QMatrix4x4 mvp = mp * m;
    paintRF(mvp);
    glDepthFunc(GL_LESS);
    paintLabels(mp, m);
  }
}

void TrigramWidget::paintLabels(QMatrix4x4& scene_mp, QMatrix4x4& scene_m) {
  QMatrix4x4 screen_mp;
  screen_mp.ortho(0, width, 0, height, -1.f, 1.f);

  QMatrix4x4 scene_to_screen;
  scene_to_screen.setToIdentity();
  scene_to_screen.scale(width * .5f, height * .5f, -1.f);
  scene_to_screen.translate(1.f, 1.f, 0.f);
  scene_to_screen = scene_to_screen * scene_mp * scene_m;

  label_program_.bind();
  label_vao_.bind();

  texture_0_->bind();
  paintLabel(lpm_0_, scene_to_screen, screen_mp);
  texture_1_->bind();
  paintLabel(lpm_1_, scene_to_screen, screen_mp);
  texture_2_->bind();
  paintLabel(lpm_2_, scene_to_screen, screen_mp);
  texture_3_->bind();
  paintLabel(lpm_3_, scene_to_screen, screen_mp);
  texture_pos_->bind();
  paintLabel(lpm_pos_, scene_to_screen, screen_mp);
  texture_N0_->bind();
  paintLabel(lpm_N0_, scene_to_screen, screen_mp);
  texture_N0_->release();

  label_vao_.release();
  label_program_.release();
}

void TrigramWidget::paintLabel(LabelPositionMixer& mixer,
    QMatrix4x4& scene_to_screen, QMatrix4x4& screen_mp) {
  QVector4D world_pos = mixer.mix(c_sph, c_cyl, c_pos);
  QVector3D screen_pos = calcScreenPosForLabel(
          world_pos.toVector3D(), scene_to_screen,
          texture_0_->width(), texture_0_->height());
  float scale_x = texture_0_->width() * world_pos.w();
  float scale_y = texture_0_->height() * world_pos.w();
  QMatrix4x4 mvp = screen_mp;
  mvp.translate(screen_pos);
  mvp.scale(scale_x, scale_y, 1.f);
  label_program_.setUniformValue("mvp", mvp);
  label_program_.setUniformValue("texture_sampler", 0);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

void TrigramWidget::initLabels() {
  initLabelPositionMixers();

  label_program_.create();
  label_program_.addShaderFromSourceFile(QOpenGLShader::Vertex,
      ":/trigram/label_vshader.glsl");
  label_program_.addShaderFromSourceFile(QOpenGLShader::Fragment,
      ":/trigram/label_fshader.glsl");
  label_program_.link();
  label_program_.bind();

  static const float label_vert[] = {
      0.f, 0.f, 0.f, 0.0, 0.0,
      1.f, 0.f, 0.f, 1.0, 0.0,
      1.f, 1.f, 0.f, 1.0, 1.0,
      1.f, 1.f, 0.f, 1.0, 1.0,
      0.f, 1.f, 0.f, 0.0, 1.0,
      0.f, 0.f, 0.f, 0.0, 0.0
    };

  label_vao_.create();
  label_vao_.bind();

  label_vb_ = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
  label_vb_.create();
  label_vb_.setUsagePattern(QOpenGLBuffer::StaticDraw);
  label_vb_.bind();
  label_vb_.allocate(label_vert, 36 * sizeof(float));

  label_program_.setAttributeBuffer(label_program_.attributeLocation("vert"),
      GL_FLOAT, 0, 3, 5 * sizeof(float));
  label_program_.enableAttributeArray(
      label_program_.attributeLocation("vert"));
  label_program_.setAttributeBuffer(label_program_.attributeLocation("tex_coord_in"),
      GL_FLOAT, 3 * sizeof(float), 2, 5 * sizeof(float));
  label_program_.enableAttributeArray(
      label_program_.attributeLocation("tex_coord_in"));

  label_vb_.release();
  label_vao_.release();
  label_program_.release();

  texture_0_ = new QOpenGLTexture(QImage(":/images/label_zero.png").mirrored());
  texture_1_ = new QOpenGLTexture(QImage(":/images/label_one.png").mirrored());
  texture_2_ = new QOpenGLTexture(QImage(":/images/label_two.png").mirrored());
  texture_3_ = new QOpenGLTexture(QImage(":/images/label_three.png").mirrored());
  texture_pos_ = new QOpenGLTexture(QImage(":/images/label_pos.png").mirrored());
  texture_N0_ = new QOpenGLTexture(QImage(":/images/label_N0.png").mirrored());
}

void TrigramWidget::releaseLabels() {
  label_vao_.destroy();
  label_vb_.destroy();
  delete texture_0_;
  delete texture_1_;
  delete texture_2_;
  delete texture_3_;
  delete texture_pos_;
  delete texture_N0_;
}

QVector3D TrigramWidget::calcScreenPosForLabel(QVector3D world_pos,
    QMatrix4x4& scene_to_screen, int width, int height) {
  QVector3D world_zero(0.f, 0.f, 0.f);
  QVector3D zero_on_screen = scene_to_screen.map(world_zero);
  QVector3D pos_on_screen = scene_to_screen.map(world_pos);
  QVector3D label_pos = pos_on_screen - zero_on_screen;
  label_pos.setZ(0.f);
  float length = label_pos.length();
  float factor = length >= 50.f ? 1.f : length / 50.f;
  label_pos.normalize();
  label_pos *= factor * 1.2 * std::sqrt((.5f * width) * (.5f * width)
      + (.5f * height) * (.5f * height));
  label_pos += pos_on_screen - QVector3D(width >> 1, height >> 1, 0.f);
  return label_pos;
}

void TrigramWidget::paintRF(QMatrix4x4& mvp) {
  glEnable(GL_LINE_SMOOTH);

  rf_vao_.bind();
  rf_program_.bind();
  rf_program_.setUniformValue("mvp", mvp);
  switch (shape_) {
  case EVisualisationShape::CUBE:
    if (c_cyl == 0.f && c_sph == 0.f) {
      glDrawArrays(GL_LINES, 0, 6);
    }
    break;
  case EVisualisationShape::CYLINDER:
    if (c_cyl == 1.f) {
      glDrawArrays(GL_LINES, 6, 4);
    }
    break;
  case EVisualisationShape::SPHERE:
    if (c_sph == 1.f) {
      glDrawArrays(GL_LINES, 10, 2);
    }
    break;
  default:
    break;
  }
  rf_program_.release();
  rf_vao_.release();
}

void TrigramWidget::initRF() {
  rf_program_.create();
  rf_program_.addShaderFromSourceFile(QOpenGLShader::Vertex,
      ":/trigram/rf_vshader.glsl");
  rf_program_.addShaderFromSourceFile(QOpenGLShader::Fragment,
      ":/trigram/rf_fshader.glsl");
  rf_program_.link();
  rf_program_.bind();

  static const float rf_vert[] = {
      //cube
      -1.f, -1.f, -1.f,
      1.f, -1.f, -1.f,
      -1.f, -1.f, -1.f,
      -1.f, 1.f, -1.f,
      -1.f, -1.f, -1.f,
      -1.f, -1.f, 1.f,

      //cylinder
      0.f, 0.f, -1.f,
      0.f, 0.f, 1.f,
      0.f, 0.f, -1.f,
      1.f, 0.f, -1.f,

      //sphere
      0.f, 0.f, 0.f,
      0.f, 0.f, 1.f,
    };

  rf_vao_.create();
  rf_vao_.bind();

  rf_vb_ = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
  rf_vb_.create();
  rf_vb_.setUsagePattern(QOpenGLBuffer::StaticDraw);
  rf_vb_.bind();
  rf_vb_.allocate(rf_vert, 36 * sizeof(float));

  rf_program_.setAttributeBuffer(rf_program_.attributeLocation("vert"),
  GL_FLOAT, 0, 3, 3 * sizeof(float));
  rf_program_.enableAttributeArray(rf_program_.attributeLocation("vert"));

  rf_vb_.release();
  rf_vao_.release();
  rf_program_.release();
}

void TrigramWidget::releaseRF() {
  rf_vao_.destroy();
  rf_vb_.destroy();
}

void TrigramWidget::initLabelPositionMixers() {
  lpm_0_ = LabelPositionMixer(
      QVector4D(-1.f, -1.f, -1.f, 1.f),
      QVector4D(0.f, 0.f, 0.f, 0.f),
      QVector4D(0.f, 0.f, -1.f, 1.f),
      QVector4D(0.f, 0.f, -1.f, 1.f),
      QVector4D(-1.f, -1.f, -1.f, 1.f),
      QVector4D(0.f, 0.f, 0.f, 0.f)
      );

  lpm_1_ = LabelPositionMixer(
      QVector4D(1.f, -1.f, -1.f, 1.f),
      QVector4D(0.f, 0.f, 0.f, 0.f),
      QVector4D(0.f, 0.f, 0.f, 0.f),
      QVector4D(0.f, 0.f, 0.f, 0.f),
      QVector4D(1.f, -1.f, -1.f, 1.f),
      QVector4D(0.f, 0.f, 0.f, 0.f)
      );

  lpm_2_ = LabelPositionMixer(
      QVector4D(-1.f, 1.f, -1.f, 1.f),
      QVector4D(0.f, 0.f, 0.f, 0.f),
      QVector4D(1.f, 0.f, -1.f, 1.f),
      QVector4D(0.f, 0.f, 0.f, 0.f),
      QVector4D(-1.f, 1.f, -1.f, 1.f),
      QVector4D(0.f, 0.f, 0.f, 0.f)
      );

  lpm_3_ = LabelPositionMixer(
      QVector4D(-1.f, -1.f, 1.f, 1.f),
      QVector4D(0.f, 0.f, 0.f, 0.f),
      QVector4D(0.f, 0.f, 1.f, 1.f),
      QVector4D(0.f, 0.f, 0.f, 0.f),
      QVector4D(0.f, 0.f, 0.f, 0.f),
      QVector4D(0.f, 0.f, 0.f, 0.f)
      );

  lpm_pos_ = LabelPositionMixer(
      QVector4D(0.f, 0.f, 0.f, 0.f),
      QVector4D(0.f, 0.f, 0.f, 0.f),
      QVector4D(0.f, 0.f, 0.f, 0.f),
      QVector4D(0.f, 0.f, 1.f, 1.f),
      QVector4D(-1.f, -1.f, 1.f, 1.f),
      QVector4D(0.f, 0.f, 0.f, 0.f)
      );

  lpm_N0_ = LabelPositionMixer(
      QVector4D(1.f, 1.f, 1.f, 1.f),
      QVector4D(0.f, 0.f, 1.f, 1.f),
      QVector4D(1.f, 0.f, 1.f, 1.f),
      QVector4D(1.f, 0.f, -1.f, 1.f),
      QVector4D(1.f, 1.f, -1.f, 1.f),
      QVector4D(0.f, 0.f, 0.f, 0.f)
      );
}

}  // namespace visualisation
}  // namespace veles
