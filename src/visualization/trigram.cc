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
#include "visualization/trigram.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <functional>
#include <vector>

#include <QBitmap>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QSlider>
#include <QToolButton>
#include <QWidgetAction>

#include "util/icons.h"
#include "util/settings/visualization.h"

namespace veles {
namespace visualization {

using util::settings::shortcuts::ShortcutsModel;

const int k_minimum_brightness = 25;
const int k_maximum_brightness = 103;
const double k_brightness_heuristic_threshold = 0.66;
const int k_brightness_heuristic_min = 38;
const int k_brightness_heuristic_max = 66;
// decrease this to reduce noise (but you may lose data if you overdo it)
const double k_brightness_heuristic_scaling = 2.5;

TrigramWidget::TrigramWidget(QWidget* parent) : VisualizationWidget(parent) {
  show_labels_ = util::settings::visualization::showCaptions();
  setBrightness(util::settings::visualization::brightness());
  use_brightness_heuristic_ = util::settings::visualization::autoBrightness();
  manipulators_.push_back(spin_manipulator_ = new SpinManipulator(this));
  manipulators_.push_back(trackball_manipulator_ =
                              new TrackballManipulator(this));
  manipulators_.push_back(free_manipulator_ = new FreeManipulator(this));
  current_manipulator_ = nullptr;
  setManipulator(manipulators_.front());
  time_.start();
  setFocusPolicy(Qt::StrongFocus);
}

TrigramWidget::~TrigramWidget() {
  if (texture_ == nullptr && databuf_ == nullptr) {
    return;
  }
  makeCurrent();
  delete texture_;
  delete databuf_;
  releaseLabels();
  releaseRF();
  doneCurrent();
}

void TrigramWidget::setBrightness(int value) {
  util::settings::visualization::setBrightness(value);
  brightness_ = value;
}

void TrigramWidget::setMode(EVisualizationMode mode, bool animate) {
  mode_ = mode;
  if (mode_ == EVisualizationMode::LAYERED_DIGRAM && !animate) {
    c_pos_ = 1;
  } else if (mode_ == EVisualizationMode::TRIGRAM && !animate) {
    c_pos_ = 0;
  }
}

float TrigramWidget::vfovDeg(float min_fov_deg, float aspect_ratio) {
  if (aspect_ratio >= 1.f) {
    return min_fov_deg;
  }

  static float deg2rad = std::acos(-1.f) / 180.f;
  float min_fov = deg2rad * min_fov_deg;
  float vfov = 2.f * std::atan(std::tan(min_fov * .5f) / aspect_ratio);
  return vfov / deg2rad;
}

void TrigramWidget::refresh(const AdditionalResampleDataPtr& ad) {
  if (use_brightness_heuristic_) {
    if (ad) {
      setBrightness(std::static_pointer_cast<BrightnessData>(ad)->brightness);
      if (brightness_slider_ != nullptr) {
        brightness_slider_->setValue(brightness_);
      }
    } else {
      autoSetBrightness();
    }
  }
  makeCurrent();
  delete texture_;
  delete databuf_;
  initTextures();
  doneCurrent();
}

void TrigramWidget::prepareOptions(QMainWindow* visualization_window) {
  VisualizationWidget::prepareOptions(visualization_window);

  QColor icon_color = palette().color(QPalette::WindowText);
  visualization_window->addToolBarBreak();

  /////////////////////////////////////
  // Camera manipulators
  prepareManipulatorToolbar(visualization_window);

  /////////////////////////////////////
  // Shape
  QToolBar* shape_toolbar = new QToolBar("Shape", this);
  shape_toolbar->setMovable(false);
  cube_action_ = ShortcutsModel::getShortcutsModel()->createQAction(
      util::settings::shortcuts::TRIGRAM_CUBE, this,
      util::getColoredIcon(":/images/cube.png", icon_color, false),
      Qt::WindowShortcut);
  connect(cube_action_, &QAction::triggered,
          std::bind(&TrigramWidget::setShape, this, EVisualizationShape::CUBE));
  shape_toolbar->addAction(cube_action_);

  cylinder_action_ = ShortcutsModel::getShortcutsModel()->createQAction(
      util::settings::shortcuts::TRIGRAM_CYLINDER, this,
      util::getColoredIcon(":/images/cylinder.png", icon_color, false),
      Qt::WindowShortcut);
  connect(
      cylinder_action_, &QAction::triggered,
      std::bind(&TrigramWidget::setShape, this, EVisualizationShape::CYLINDER));
  shape_toolbar->addAction(cylinder_action_);

  sphere_action_ = ShortcutsModel::getShortcutsModel()->createQAction(
      util::settings::shortcuts::TRIGRAM_SPHERE, this,
      util::getColoredIcon(":/images/sphere.png", icon_color),
      Qt::WindowShortcut);

  connect(
      sphere_action_, &QAction::triggered,
      std::bind(&TrigramWidget::setShape, this, EVisualizationShape::SPHERE));
  shape_toolbar->addAction(sphere_action_);
  shape_toolbar->addSeparator();
  shape_toolbar->setContextMenuPolicy(Qt::PreventContextMenu);
  visualization_window->addToolBar(shape_toolbar);

  /////////////////////////////////////
  // Captions
  auto* rf_widget_action = new QWidgetAction(this);
  show_labels_and_rf_checkbox_ = new QCheckBox(tr("Show captions"));
  show_labels_and_rf_checkbox_->setChecked(show_labels_);
  rf_widget_action->setDefaultWidget(show_labels_and_rf_checkbox_);
  connect(show_labels_and_rf_checkbox_, &QCheckBox::toggled,
          [this](bool toggled) {
            show_labels_ = toggled;
            util::settings::visualization::setShowCaptions(toggled);
          });
  auto* captions_toolbar = new QToolBar("Captions", this);
  captions_toolbar->setMovable(false);
  captions_toolbar->addAction(rf_widget_action);
  captions_toolbar->addSeparator();
  captions_toolbar->setContextMenuPolicy(Qt::PreventContextMenu);
  visualization_window->addToolBar(captions_toolbar);

  /////////////////////////////////////
  // Perspective
  auto* perspective_widget_action = new QWidgetAction(this);
  perspective_checkbox_ = new QCheckBox(tr("Perspective"));
  perspective_checkbox_->setChecked(perspective_);
  perspective_widget_action->setDefaultWidget(perspective_checkbox_);
  connect(perspective_checkbox_, &QCheckBox::toggled,
          [this](bool toggled) { perspective_ = toggled; });
  auto* perspective_toolbar = new QToolBar("Perspective", this);
  perspective_toolbar->setMovable(false);
  perspective_toolbar->addAction(perspective_widget_action);
  perspective_toolbar->addSeparator();
  perspective_toolbar->setContextMenuPolicy(Qt::PreventContextMenu);
  visualization_window->addToolBar(perspective_toolbar);

  /////////////////////////////////////
  // Brightness
  auto* brightness_widget_action = new QWidgetAction(this);
  auto* brightness_widget = new QWidget;
  auto* layout = new QHBoxLayout;
  brightness_widget->setLayout(layout);

  auto* brightness_label = new QLabel;
  brightness_label->setPixmap(QPixmap(":/images/brightness.png"));
  brightness_label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  layout->addWidget(brightness_label);

  if (use_brightness_heuristic_) {
    setBrightness(suggestBrightness());
  }
  brightness_slider_ = new QSlider(Qt::Horizontal);
  brightness_slider_->setMinimum(k_minimum_brightness);
  brightness_slider_->setMaximum(k_maximum_brightness);
  brightness_slider_->setMaximumWidth(200);
  brightness_slider_->setValue(brightness_);
  connect(brightness_slider_, SIGNAL(valueChanged(int)), this,
          SLOT(brightnessSliderMoved(int)));
  layout->addWidget(brightness_slider_);

  use_heuristic_checkbox_ = new QCheckBox("Auto");
  use_heuristic_checkbox_->setChecked(use_brightness_heuristic_);
  connect(use_heuristic_checkbox_, SIGNAL(stateChanged(int)), this,
          SLOT(setUseBrightnessHeuristic(Qt::CheckState)));
  layout->addWidget(use_heuristic_checkbox_);
  layout->setStretch(0, 0);
  layout->setStretch(1, 0);
  layout->setStretch(2, 1);
  layout->setMargin(1);
  brightness_widget_action->setDefaultWidget(brightness_widget);
  QToolBar* brightness_toolbar = new QToolBar("Brightness", this);
  brightness_toolbar->setMovable(false);
  brightness_toolbar->addAction(brightness_widget_action);
  brightness_toolbar->addSeparator();
  brightness_toolbar->setContextMenuPolicy(Qt::PreventContextMenu);
  visualization_window->addToolBar(brightness_toolbar);
}

int TrigramWidget::suggestBrightness() {
  size_t size = getDataSize();
  auto data = reinterpret_cast<const uint8_t*>(getData());
  if (size < 100) {
    return (k_minimum_brightness + k_maximum_brightness) / 2;
  }
  std::vector<uint64_t> counts(256, 0);
  for (size_t i = 0; i < size; ++i) {
    counts[data[i]] += 1;
  }
  std::sort(counts.begin(), counts.end());
  int offset = 0, sum = 0;
  while (offset < 255 && sum < k_brightness_heuristic_threshold * size) {
    sum += counts[255 - offset];
    offset += 1;
  }
  offset = static_cast<int>(static_cast<double>(offset) /
                            k_brightness_heuristic_scaling);
  return std::max(k_brightness_heuristic_min,
                  k_brightness_heuristic_max - offset);
}

VisualizationWidget::AdditionalResampleData* TrigramWidget::onAsyncResample() {
  if (use_brightness_heuristic_) {
    auto* res = new BrightnessData();
    res->brightness = suggestBrightness();
    return res;
  }
  return nullptr;
}

void TrigramWidget::playPause() {
// It is still unclear if current behaviour of manipulators is going to be
// subject of change.
#if 0
  QPixmap pixmap;
  QColor icon_color = palette().color(QPalette::WindowText);
  if (is_playing_) {
    pause_button_->setIcon(util::getColoredIcon(":/images/play.png",
        icon_color));
  } else {
    pause_button_->setIcon(util::getColoredIcon(":/images/pause.png",
        icon_color));
  }
#endif

  is_playing_ = !is_playing_;
}

void TrigramWidget::setShape(EVisualizationShape shape) { shape_ = shape; }

void TrigramWidget::brightnessSliderMoved(int value) {
  if (value == brightness_) {
    return;
  }
  use_brightness_heuristic_ = false;
  util::settings::visualization::setAutoBrightness(false);
  use_heuristic_checkbox_->setChecked(false);
  setBrightness(value);
}

void TrigramWidget::setUseBrightnessHeuristic(Qt::CheckState state) {
  use_brightness_heuristic_ = state != Qt::Unchecked;
  util::settings::visualization::setAutoBrightness(state != Qt::Unchecked);
  if (use_brightness_heuristic_) {
    autoSetBrightness();
  }
}

void TrigramWidget::setManipulator(Manipulator* manipulator) {
  if (manipulator == current_manipulator_) {
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

  if (!is_playing_) {
    playPause();
  }

  emit manipulatorChanged(current_manipulator_);
  setFocus();
}

void TrigramWidget::autoSetBrightness() {
  auto new_brightness = suggestBrightness();
  if (new_brightness == brightness_) {
    return;
  }
  setBrightness(new_brightness);
  if (brightness_slider_ != nullptr) {
    brightness_slider_->setValue(brightness_);
  }
}

bool TrigramWidget::event(QEvent* event) {
  if (event->type() == QEvent::MouseMove) {
    auto* mouse_event = static_cast<QMouseEvent*>(event);
    if ((mouse_event->buttons() & Qt::LeftButton) != 0 &&
        current_manipulator_ == spin_manipulator_) {
      setManipulator(trackball_manipulator_);
      trackball_manipulator_->processEvent(this, event);
    }
  } else if (event->type() == QEvent::KeyPress) {
    auto* key_event = static_cast<QKeyEvent*>(event);
    if ((current_manipulator_ == spin_manipulator_ ||
         current_manipulator_ == trackball_manipulator_) &&
        FreeManipulator::isCtrlButton(key_event->key())) {
      setManipulator(free_manipulator_);
      trackball_manipulator_->processEvent(this, event);
    }
  }

  return QWidget::event(event);
}

static void animateTo(float* thing, float target, float step) {
  if (*thing < target) {
    *thing += step;
    if (*thing > target) {
      *thing = target;
    }
  } else if (*thing > target) {
    *thing -= step;
    if (*thing < target) {
      *thing = target;
    }
  }
}

void TrigramWidget::timerEvent(QTimerEvent* /*event*/) {
  animateTo(&c_cyl_, shape_ == EVisualizationShape::CYLINDER ? 1.0 : 0.0,
            0.01f);
  animateTo(&c_sph_, shape_ == EVisualizationShape::SPHERE ? 1.0 : 0.0, 0.01f);
  animateTo(&c_pos_, mode_ == EVisualizationMode::LAYERED_DIGRAM ? 1.0 : 0.0,
            0.01f);
  animateTo(&c_ort_, perspective_ ? 0.0 : 1.0, 0.02f);
  update();
}

bool TrigramWidget::initializeVisualizationGL() {
  if (!initializeOpenGLFunctions()) {
    return false;
  }

  glClearColor(0, 0, 0, 1);
  glEnable(GL_DEPTH_TEST);

  if (use_brightness_heuristic_) {
    autoSetBrightness();
  }

  initShaders();
  initTextures();
  initGeometry();

  initLabels();
  initRF();

  return true;
}

void TrigramWidget::initShaders() {
  if (!program_.addShaderFromSourceFile(QOpenGLShader::Vertex,
                                        ":/trigram/vshader.glsl")) {
    close();
  }

  // Compile fragment shader
  if (!program_.addShaderFromSourceFile(QOpenGLShader::Fragment,
                                        ":/trigram/fshader.glsl")) {
    close();
  }

  // Link shader pipeline
  if (!program_.link()) {
    close();
  }

  timer_.start(12, this);
}

void TrigramWidget::initTextures() {
  auto size = static_cast<int>(getDataSize());
  auto* data = reinterpret_cast<const uint8_t*>(getData());

  databuf_ = new QOpenGLBuffer(QOpenGLBuffer::Type(GL_TEXTURE_BUFFER));
  databuf_->create();
  databuf_->bind();
  databuf_->allocate(data, size);
  databuf_->release();

  texture_ = new QOpenGLTexture(QOpenGLTexture::TargetBuffer);
  texture_->setSize(size);
  texture_->setFormat(QOpenGLTexture::R8U);
  texture_->create();
  texture_->bind();
  glTexBuffer(GL_TEXTURE_BUFFER, QOpenGLTexture::R8U, databuf_->bufferId());
}

void TrigramWidget::initGeometry() { vao_.create(); }

QAction* TrigramWidget::createAction(
    util::settings::shortcuts::ShortcutType type, const QIcon& icon,
    Manipulator* manipulator) {
  auto action = ShortcutsModel::getShortcutsModel()->createQAction(
      type, this, icon, Qt::WidgetWithChildrenShortcut);
  connect(action, &QAction::triggered,
          std::bind(&TrigramWidget::setManipulator, this, manipulator));
  action->setProperty("manipulator", QVariant(qintptr(manipulator)));
  return action;
}

QAbstractButton* TrigramWidget::createActionButton(QAction* action) {
  auto* button = new QToolButton;
  button->setAutoRaise(true);
  button->setIcon(action->icon());
  button->setToolTip(action->text());
  button->setCheckable(true);
  button->setIconSize(QSize(64, 64));
  button->setAutoExclusive(true);
  button->setProperty("action", QVariant(qintptr(action)));
  connect(button, &QPushButton::toggled, [button](bool toggled) {
    if (toggled) {
      QAction* action = reinterpret_cast<QAction*>(
          qvariant_cast<qintptr>(button->property("action")));
      if (action != nullptr) {
        action->trigger();
      }
    }
  });
  connect(this, &TrigramWidget::manipulatorChanged,
          [action, button](Manipulator* new_manipulator) {
            Manipulator* manipulator = reinterpret_cast<Manipulator*>(
                qvariant_cast<qintptr>(action->property("manipulator")));
            if (manipulator == new_manipulator) {
              button->setChecked(true);
            }
          });
  return button;
}

void TrigramWidget::prepareManipulatorToolbar(
    QMainWindow* visualization_window) {
  QToolBar* manipulator_toolbar = new QToolBar("Camera manipulators", this);
  manipulator_toolbar->setMovable(false);

  {
    QAction* action =
        createAction(util::settings::shortcuts::VISUALIZATION_MANIPULATOR_SPIN,
                     QIcon(":/images/manipulator_spin.png"), spin_manipulator_);
    addAction(action);
    manipulator_toolbar->addAction(action);
  }

  {
    QAction* action = createAction(
        util::settings::shortcuts::VISUALIZATION_MANIPULATOR_TRACKBALL,
        QIcon(":/images/manipulator_trackball.png"), trackball_manipulator_);
    addAction(action);
    manipulator_toolbar->addAction(action);
  }

  {
    QAction* action =
        createAction(util::settings::shortcuts::VISUALIZATION_MANIPULATOR_FREE,
                     QIcon(":/images/manipulator_free.png"), free_manipulator_);
    addAction(action);
    manipulator_toolbar->addAction(action);
  }

  manipulator_toolbar->addSeparator();
  manipulator_toolbar->setContextMenuPolicy(Qt::PreventContextMenu);
  visualization_window->addToolBar(manipulator_toolbar);
}

void TrigramWidget::resizeGLImpl(int w, int h) {
  width_ = w;
  height_ = h;
}

void TrigramWidget::paintGLImpl() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_BLEND);
  glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ONE);
  glDepthFunc(GL_ALWAYS);
  auto size = static_cast<unsigned>(getDataSize());

  program_.bind();
  texture_->bind();
  vao_.bind();

  QMatrix4x4 mp, mo, m;
  m.setToIdentity();
  float dt = static_cast<float>(time_.restart()) / 1000.f;
  if (current_manipulator_ != nullptr) {
    if (is_playing_ || !current_manipulator_->handlesPause()) {
      current_manipulator_->update(dt);
    }
    m = current_manipulator_->transform();
  }

  mp.setToIdentity();
  mo.setToIdentity();
  float aspect_ratio = static_cast<float>(width_) / height_;
  mp.perspective(vfovDeg(45.f, aspect_ratio), aspect_ratio, 0.01f, 100.0f);
  if (aspect_ratio < 1) {
    mo.ortho(-1, 1, -1 / aspect_ratio, 1 / aspect_ratio, -100.0f, 10000.0f);
  } else {
    mo.ortho(-aspect_ratio, aspect_ratio, -1, 1, -100.0f, 10000.0f);
  }
  // Extract the translation-by-z component of the modelview matrix and use it
  // to get a scale factor for the orthogonal projection (since it would
  // otherwise always be the same size on screen).
  float tz = m(2, 3);
  // If tz happens to be positive or 0 (ie. camera is at or past cube center),
  // we'd get a negative scale factor.  Cap it instead.
  tz = std::min(tz, -0.001f);
  // This factor results in the cube being approximately the same size in both
  // orthogonal and perspective projections -- the plane containing the cube
  // center is the same size on screen.
  mo.scale(-2.5f / tz);
  // The * 4 fudge factor for ortho projection doesn't affect the final result
  // (since it uniformly scales w as well as x, y, z), but improves
  // the animation between ortho and perspective.
  mp = mp * (1 - c_ort_) + mo * c_ort_ * 4;
  QMatrix4x4 mvp = mp * m;

  int loc_sz = program_.uniformLocation("sz");
  program_.setUniformValue("tx", 0);
  program_.setUniformValue("c_cyl", c_cyl_);
  program_.setUniformValue("c_sph", c_sph_);
  program_.setUniformValue("c_pos", c_pos_);
  program_.setUniformValue("xfrm", mvp);
  GLfloat c_brightness = brightness_ * brightness_ * brightness_;
  c_brightness /= getDataSize();
  program_.setUniformValue("c_brightness", c_brightness);
  glUniform1ui(loc_sz, size);
  glDrawArrays(GL_POINTS, 0, size - 2);

  vao_.release();
  texture_->release();
  program_.release();

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  paintRF(mvp);
  glDepthFunc(GL_LESS);
  if (show_labels_) {
    paintLabels(mp, m);
  }
}

void TrigramWidget::paintLabels(const QMatrix4x4& scene_mp,
                                const QMatrix4x4& scene_m) {
  QMatrix4x4 screen_mp;
  screen_mp.ortho(0, width_, 0, height_, -1.f, 1.f);

  QMatrix4x4 scene_to_screen;
  scene_to_screen.setToIdentity();
  scene_to_screen.scale(width_ * .5f, height_ * .5f, -1.f);
  scene_to_screen.translate(1.f, 1.f, 0.f);
  scene_to_screen = scene_to_screen * scene_mp * scene_m;

  label_program_.bind();
  label_vao_.bind();

  paintLabel(lpm_0_, scene_to_screen, screen_mp, texture_0_);
  paintLabel(lpm_1_, scene_to_screen, screen_mp, texture_1_);
  paintLabel(lpm_2_, scene_to_screen, screen_mp, texture_2_);
  paintLabel(lpm_3_, scene_to_screen, screen_mp, texture_3_);
  paintLabel(lpm_pos_, scene_to_screen, screen_mp, texture_pos_);
  paintLabel(lpm_N0_, scene_to_screen, screen_mp, texture_N0_);
  paintLabel(lpm_0_digram_, scene_to_screen, screen_mp, texture_0_digram_);
  paintLabel(lpm_1_digram_, scene_to_screen, screen_mp, texture_1_digram_);
  paintLabel(lpm_2_digram_, scene_to_screen, screen_mp, texture_2_digram_);
  paintLabel(lpm_N0_digram_, scene_to_screen, screen_mp, texture_N0_digram_);
  texture_N0_digram_->release();

  label_vao_.release();
  label_program_.release();
}

void TrigramWidget::paintLabel(const LabelPositionMixer& mixer,
                               const QMatrix4x4& scene_to_screen,
                               const QMatrix4x4& screen_mp,
                               QOpenGLTexture* texture) {
  texture->bind();
  QVector4D world_pos = mixer.mix(c_sph_, c_cyl_, c_pos_);
  QVector3D screen_pos =
      calcScreenPosForLabel(world_pos.toVector3D(), scene_to_screen,
                            texture->width(), texture->height());
  float scale_x = texture->width() * world_pos.w();
  float scale_y = texture->height() * world_pos.w();
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

  // clang-format off
  static const float label_vert[] = {
      0.f, 0.f, 0.f, 0.0, 0.0,
      1.f, 0.f, 0.f, 1.0, 0.0,
      1.f, 1.f, 0.f, 1.0, 1.0,
      1.f, 1.f, 0.f, 1.0, 1.0,
      0.f, 1.f, 0.f, 0.0, 1.0,
      0.f, 0.f, 0.f, 0.0, 0.0
  };
  // clang-format on

  label_vao_.create();
  label_vao_.bind();

  label_vb_ = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
  label_vb_.create();
  label_vb_.setUsagePattern(QOpenGLBuffer::StaticDraw);
  label_vb_.bind();
  label_vb_.allocate(label_vert, 36 * sizeof(float));

  label_program_.setAttributeBuffer(label_program_.attributeLocation("vert"),
                                    GL_FLOAT, 0, 3, 5 * sizeof(float));
  label_program_.enableAttributeArray(label_program_.attributeLocation("vert"));
  label_program_.setAttributeBuffer(
      label_program_.attributeLocation("tex_coord_in"), GL_FLOAT,
      3 * sizeof(float), 2, 5 * sizeof(float));
  label_program_.enableAttributeArray(
      label_program_.attributeLocation("tex_coord_in"));

  label_vb_.release();
  label_vao_.release();
  label_program_.release();

  texture_0_ = new QOpenGLTexture(QImage(":/images/label_zero.png").mirrored());
  texture_1_ = new QOpenGLTexture(QImage(":/images/label_one.png").mirrored());
  texture_2_ = new QOpenGLTexture(QImage(":/images/label_two.png").mirrored());
  texture_3_ =
      new QOpenGLTexture(QImage(":/images/label_three.png").mirrored());
  texture_pos_ =
      new QOpenGLTexture(QImage(":/images/label_pos.png").mirrored());
  texture_N0_ = new QOpenGLTexture(QImage(":/images/label_N0.png").mirrored());
  texture_0_digram_ =
      new QOpenGLTexture(QImage(":/images/label_zero_digram.png").mirrored());
  texture_1_digram_ =
      new QOpenGLTexture(QImage(":/images/label_one_digram.png").mirrored());
  texture_2_digram_ =
      new QOpenGLTexture(QImage(":/images/label_two_digram.png").mirrored());
  texture_N0_digram_ =
      new QOpenGLTexture(QImage(":/images/label_N0_digram.png").mirrored());
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
  delete texture_0_digram_;
  delete texture_1_digram_;
  delete texture_2_digram_;
  delete texture_N0_digram_;
}

QVector3D TrigramWidget::calcScreenPosForLabel(
    QVector3D world_pos, const QMatrix4x4& scene_to_screen, int width,
    int height) {
  QVector3D world_zero(0.f, 0.f, 0.f);
  QVector3D zero_on_screen = scene_to_screen.map(world_zero);
  QVector3D pos_on_screen = scene_to_screen.map(world_pos);
  QVector3D label_pos = pos_on_screen - zero_on_screen;
  label_pos.setZ(0.f);
  float length = label_pos.length();
  float factor = length >= 50.f ? 1.f : length / 50.f;
  label_pos.normalize();
  label_pos *= factor * 1.2 * std::sqrt((.5f * width) * (.5f * width) +
                                        (.5f * height) * (.5f * height));
  label_pos += pos_on_screen - QVector3D(width >> 1, height >> 1, 0.f);
  return label_pos;
}

void TrigramWidget::paintRF(const QMatrix4x4& mvp) {
  glEnable(GL_LINE_SMOOTH);

  rf_vao_.bind();
  rf_program_.bind();
  rf_program_.setUniformValue("mvp", mvp);
  switch (shape_) {
    case EVisualizationShape::CUBE:
      if (c_cyl_ == 0.f && c_sph_ == 0.f) {
        glDrawArrays(GL_LINES, 0, 6);
      }
      break;
    case EVisualizationShape::CYLINDER:
      if (c_cyl_ == 1.f) {
        glDrawArrays(GL_LINES, 6, 4);
      }
      break;
    case EVisualizationShape::SPHERE:
      if (c_sph_ == 1.f) {
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

  // clang-format off
  static const float rf_vert[] = {
      //cube
      -1.f, -1.f, -1.f, 1.f, 0.f, 0.f,
       1.f, -1.f, -1.f, 1.f, 0.f, 0.f,
      -1.f, -1.f, -1.f, 0.f, 1.f, 0.f,
      -1.f,  1.f, -1.f, 0.f, 1.f, 0.f,
      -1.f, -1.f, -1.f, 0.f, 0.f, 1.f,
      -1.f, -1.f,  1.f, 0.f, 0.f, 1.f,

      //cylinder
      0.f, 0.f, -1.f, 1.f, 0.f, 1.f,
      0.f, 0.f,  1.f, 1.f, 0.f, 1.f,
      0.f, 0.f, -1.f, 1.f, 0.f, 1.f,
      1.f, 0.f, -1.f, 1.f, 0.f, 1.f,

      //sphere
      0.f, 0.f, 0.f, 1.f, 0.f, 1.f,
      0.f, 0.f, 1.f, 1.f, 0.f, 1.f
  };
  // clang-format on

  rf_vao_.create();
  rf_vao_.bind();

  rf_vb_ = QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
  rf_vb_.create();
  rf_vb_.setUsagePattern(QOpenGLBuffer::StaticDraw);
  rf_vb_.bind();
  rf_vb_.allocate(rf_vert, sizeof rf_vert);

  rf_program_.setAttributeBuffer(rf_program_.attributeLocation("vert"),
                                 GL_FLOAT, 0, 3, 6 * sizeof(float));
  rf_program_.enableAttributeArray(rf_program_.attributeLocation("vert"));
  rf_program_.setAttributeBuffer(rf_program_.attributeLocation("color"),
                                 GL_FLOAT, 3 * sizeof(float), 3,
                                 6 * sizeof(float));
  rf_program_.enableAttributeArray(rf_program_.attributeLocation("color"));

  rf_vb_.release();
  rf_vao_.release();
  rf_program_.release();
}

void TrigramWidget::releaseRF() {
  rf_vao_.destroy();
  rf_vb_.destroy();
}

void TrigramWidget::initLabelPositionMixers() {
  // clang-format off
  lpm_0_ = LabelPositionMixer(
      QVector4D(-1.f, -1.f, -1.f, 1.f),
      QVector4D( 0.f,  0.f,  0.f, 0.f),
      QVector4D( 0.f,  0.f, -1.f, 1.f),
      QVector4D( 0.f,  0.f, -1.f, 0.f),
      QVector4D(-1.f, -1.f, -1.f, 0.f),
      QVector4D( 0.f,  0.f,  0.f, 0.f)
      );

  lpm_1_ = LabelPositionMixer(
      QVector4D(1.f, -1.f, -1.f, 1.f),
      QVector4D(0.f,  0.f,  0.f, 0.f),
      QVector4D(0.f,  0.f,  0.f, 0.f),
      QVector4D(0.f,  0.f,  0.f, 0.f),
      QVector4D(1.f, -1.f, -1.f, 0.f),
      QVector4D(0.f,  0.f,  0.f, 0.f)
      );

  lpm_2_ = LabelPositionMixer(
      QVector4D(-1.f, 1.f, -1.f, 1.f),
      QVector4D( 0.f, 0.f,  0.f, 0.f),
      QVector4D( 1.f, 0.f, -1.f, 1.f),
      QVector4D( 0.f, 0.f,  0.f, 0.f),
      QVector4D(-1.f, 1.f, -1.f, 0.f),
      QVector4D( 0.f, 0.f,  0.f, 0.f)
      );

  lpm_3_ = LabelPositionMixer(
      QVector4D(-1.f, -1.f, 1.f, 1.f),
      QVector4D( 0.f,  0.f, 0.f, 0.f),
      QVector4D( 0.f,  0.f, 1.f, 1.f),
      QVector4D( 0.f,  0.f, 0.f, 0.f),
      QVector4D( 0.f,  0.f, 0.f, 0.f),
      QVector4D( 0.f,  0.f, 0.f, 0.f)
      );

  lpm_pos_ = LabelPositionMixer(
      QVector4D( 0.f,  0.f, 0.f, 0.f),
      QVector4D( 0.f,  0.f, 0.f, 0.f),
      QVector4D( 0.f,  0.f, 0.f, 0.f),
      QVector4D( 0.f,  0.f, 1.f, 1.f),
      QVector4D(-1.f, -1.f, 1.f, 1.f),
      QVector4D( 0.f,  0.f, 0.f, 0.f)
      );

  lpm_N0_ = LabelPositionMixer(
      QVector4D(1.f, 1.f,  1.f, 1.f),
      QVector4D(0.f, 0.f,  1.f, 1.f),
      QVector4D(1.f, 0.f,  1.f, 1.f),
      QVector4D(1.f, 0.f, -1.f, 0.f),
      QVector4D(1.f, 1.f, -1.f, 0.f),
      QVector4D(0.f, 0.f,  0.f, 0.f)
      );

  lpm_0_digram_ = LabelPositionMixer(
      QVector4D(-1.f, -1.f, -1.f, 0.f),
      QVector4D( 0.f,  0.f,  0.f, 0.f),
      QVector4D( 0.f,  0.f, -1.f, 0.f),
      QVector4D( 0.f,  0.f, -1.f, 1.f),
      QVector4D(-1.f, -1.f, -1.f, 1.f),
      QVector4D( 0.f,  0.f,  0.f, 0.f)
      );

  lpm_1_digram_ = LabelPositionMixer(
      QVector4D(1.f, -1.f, -1.f, 0.f),
      QVector4D(0.f,  0.f,  0.f, 0.f),
      QVector4D(0.f,  0.f,  0.f, 0.f),
      QVector4D(0.f,  0.f,  0.f, 0.f),
      QVector4D(1.f, -1.f, -1.f, 1.f),
      QVector4D(0.f,  0.f,  0.f, 0.f)
      );

  lpm_2_digram_ = LabelPositionMixer(
      QVector4D(-1.f, 1.f, -1.f, 0.f),
      QVector4D( 0.f, 0.f,  0.f, 0.f),
      QVector4D( 1.f, 0.f, -1.f, 0.f),
      QVector4D( 0.f, 0.f,  0.f, 0.f),
      QVector4D(-1.f, 1.f, -1.f, 1.f),
      QVector4D( 0.f, 0.f,  0.f, 0.f)
      );

  lpm_N0_digram_ = LabelPositionMixer(
      QVector4D(1.f, 1.f,  1.f, 0.f),
      QVector4D(0.f, 0.f,  1.f, 0.f),
      QVector4D(1.f, 0.f,  1.f, 0.f),
      QVector4D(1.f, 0.f, -1.f, 1.f),
      QVector4D(1.f, 1.f, -1.f, 1.f),
      QVector4D(0.f, 0.f,  0.f, 0.f)
      );
  // clang-format on
}

}  // namespace visualization
}  // namespace veles
