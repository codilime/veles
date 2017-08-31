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
#include "visualization/manipulator.h"

#include <cmath>

#include <QWidget>

namespace veles {
namespace visualization {

float len(QPoint p) { return std::sqrt(p.x() * p.x() + p.y() * p.y()); }

// It is assumed that transformation described by m consists of
// translation and rotation only.
void decompose(const QMatrix4x4& m, QVector3D* v, QQuaternion* r) {
  *v = m.column(3).toVector3D();
  float data[] = {m.data()[0], m.data()[1], m.data()[2],
                  m.data()[4], m.data()[5], m.data()[6],
                  m.data()[8], m.data()[9], m.data()[10]};
  *r = QQuaternion::fromRotationMatrix(QMatrix3x3(data));
}

/*****************************************************************************/
/* Manipulator */
/*****************************************************************************/

Manipulator::Manipulator(QObject* parent) : QObject(parent) {}

bool Manipulator::processEvent(QObject* watched, QEvent* event) {
  return eventFilter(watched, event);
}

bool Manipulator::handlesPause() { return false; }

/*****************************************************************************/
/* TrackballManipulator */
/*****************************************************************************/

TrackballManipulator::TrackballManipulator(QObject* parent)
    : Manipulator(parent) {
  rotation_ = QQuaternion();
  lmb_drag_ = false;
  prev_pos_ = QPoint();

  init_position_ = QVector3D();
  factor_ = 1.f;
  time_ = 2.f;
}

QMatrix4x4 TrackballManipulator::transform() {
  QVector3D dest_translation(0.f, 0.f, -distance_);
  QMatrix4x4 m;
  m.setToIdentity();
  m.translate(factor_ * dest_translation + (1.f - factor_) * init_position_);
  m *= QMatrix4x4(rotation_.toRotationMatrix());

  return m;
}

void TrackballManipulator::initFromMatrix(const QMatrix4x4& m) {
  decompose(m, &init_position_, &rotation_);
  rotation_ = rotation_.conjugated();
  distance_ = 5.f;
  factor_ = 0.f;
}

void TrackballManipulator::update(float dt) {
  factor_ += dt / time_;
  if (factor_ > 1.f) {
    factor_ = 1.f;
  }
}

QString TrackballManipulator::manipulatorName() {
  return tr("Trackball manipulator");
}

bool TrackballManipulator::eventFilter(QObject* watched, QEvent* event) {
  auto* widget = dynamic_cast<QWidget*>(watched);
  if (widget == nullptr) {
    return false;
  }

  if (isMouseEvent(event)) {
    return mouseEvent(widget, static_cast<QMouseEvent*>(event));
  }
  if (isWheelEvent(event)) {
    return wheelEvent(widget, static_cast<QWheelEvent*>(event));
  }

  return false;
}

bool TrackballManipulator::mouseEvent(QWidget* widget, QMouseEvent* event) {
  if (event->type() == QEvent::MouseButtonPress &&
      event->button() == Qt::LeftButton) {
    lmb_drag_ = true;
    prev_pos_ = event->pos();
  } else if (event->type() == QEvent::MouseButtonRelease &&
             event->button() == Qt::LeftButton) {
    lmb_drag_ = false;
  } else if (event->type() == QEvent::MouseMove) {
    if ((event->buttons() & Qt::LeftButton) != 0) {
      if (!lmb_drag_) {
        lmb_drag_ = true;
        prev_pos_ = event->pos();
      } else {
        QPoint delta = event->pos() - prev_pos_;
        rotation_ = QQuaternion::fromAxisAndAngle(
                        delta.y(), delta.x(), 0.f,
                        280.f * len(delta) /
                            std::min(widget->width(), widget->height())) *
                    rotation_;
        prev_pos_ = event->pos();
      }
    } else {
      lmb_drag_ = false;
    }
  }

  // Currently return value doesn't matter.
  return true;
}

bool TrackballManipulator::wheelEvent(QWidget* /*widget*/, QWheelEvent* event) {
  distance_ -= static_cast<float>(event->angleDelta().y()) * (1.f / 8.f / 90.f);
  if (distance_ < min_distance_) {
    distance_ = min_distance_;
  } else if (distance_ > max_distance_) {
    distance_ = max_distance_;
  }
  return true;
}

/*****************************************************************************/
/* FreeManipulator */
/*****************************************************************************/

FreeManipulator::FreeManipulator(QObject* parent)
    : Manipulator(parent), position_(0.f, 0.f, -5.f) {
  eye_rotation_ = QQuaternion();
  cube_rotation_ = QQuaternion();
  lmb_drag_ = false;
  prev_pos_ = QPoint();

  left_ = right_ = up_ = down_ = forward_ = back_ = false;
}

QMatrix4x4 FreeManipulator::transform() {
  QMatrix4x4 m = QMatrix4x4(eye_rotation_.toRotationMatrix());
  m.translate(position_);
  m.rotate(cube_rotation_);
  return m;
}

void FreeManipulator::initFromMatrix(const QMatrix4x4& m) {
  decompose(m, &position_, &eye_rotation_);
  position_ = QMatrix4x4(eye_rotation_.toRotationMatrix()).mapVector(position_);
  eye_rotation_ = eye_rotation_.conjugated();
  cube_rotation_ = QQuaternion();
}

void FreeManipulator::update(float dt) {
  QVector3D dpos_local(0.f, 0.f, 0.f);
  if (left_) dpos_local += QVector3D(-1.f, 0.f, 0.f);
  if (right_) dpos_local += QVector3D(1.f, 0.f, 0.f);
  if (up_) dpos_local += QVector3D(0.f, 1.f, 0.f);
  if (down_) dpos_local += QVector3D(0.f, -1.f, 0.f);
  if (forward_) dpos_local += QVector3D(0.f, 0.f, -1.f);
  if (back_) dpos_local += QVector3D(0.f, 0.f, 1.f);
  dpos_local *= 1.f * dt;

  position_ += QMatrix4x4(eye_rotation_.conjugated().toRotationMatrix())
                   .mapVector(-dpos_local);
}

QString FreeManipulator::manipulatorName() { return tr("Free manipulator"); }

bool FreeManipulator::isCtrlButton(int key) {
  return key == key_left || key == key_right || key == key_up ||
         key == key_down || key == key_forward || key == key_back;
}

bool FreeManipulator::eventFilter(QObject* watched, QEvent* event) {
  if (isKeyEvent(event)) {
    return keyboardEvent(static_cast<QKeyEvent*>(event));
  }
  if (isMouseEvent(event)) {
    return mouseEvent(watched, static_cast<QMouseEvent*>(event));
  }
  if (isWheelEvent(event)) {
    return wheelEvent(watched, static_cast<QWheelEvent*>(event));
  }
  return false;
}

bool FreeManipulator::keyboardEvent(QKeyEvent* event) {
  int key = event->key();

  if (event->type() == QEvent::KeyPress) {
    if (key == key_left) left_ = true;
    if (key == key_right) right_ = true;
    if (key == key_up) up_ = true;
    if (key == key_down) down_ = true;
    if (key == key_forward) forward_ = true;
    if (key == key_back) back_ = true;
    return true;
  }
  if (event->type() == QEvent::KeyRelease) {
    if (key == key_left) left_ = false;
    if (key == key_right) right_ = false;
    if (key == key_up) up_ = false;
    if (key == key_down) down_ = false;
    if (key == key_forward) forward_ = false;
    if (key == key_back) back_ = false;
    return true;
  }

  return false;
}

bool FreeManipulator::mouseEvent(QObject* watched, QMouseEvent* event) {
  auto* widget = dynamic_cast<QWidget*>(watched);
  if (widget == nullptr) {
    return false;
  }

  if (event->type() == QEvent::MouseButtonPress &&
      event->button() == Qt::LeftButton) {
    lmb_drag_ = true;
    prev_pos_ = event->pos();
  } else if (event->type() == QEvent::MouseButtonRelease &&
             event->button() == Qt::LeftButton) {
    lmb_drag_ = false;
  } else if (event->type() == QEvent::MouseMove) {
    if ((event->buttons() & Qt::LeftButton) != 0) {
      if (!lmb_drag_) {
        lmb_drag_ = true;
        prev_pos_ = event->pos();
      } else {
        QPoint delta = event->pos() - prev_pos_;
        if ((event->modifiers() & Qt::ControlModifier) != 0) {
          cube_rotation_ =
              eye_rotation_.conjugated() *
              QQuaternion::fromAxisAndAngle(
                  delta.y(), delta.x(), 0.f,
                  280.f * len(delta) /
                      std::min(widget->width(), widget->height())) *
              eye_rotation_ * cube_rotation_;
        } else {
          eye_rotation_ = QQuaternion::fromAxisAndAngle(
                              delta.y(), delta.x(), 0.f,
                              90.f * len(delta) /
                                  std::min(widget->width(), widget->height())) *
                          eye_rotation_;
        }
        prev_pos_ = event->pos();
      }
    } else {
      lmb_drag_ = false;
    }
  }

  // Currently return value doesn't matter.
  return true;
}

bool FreeManipulator::wheelEvent(QObject* /*watched*/, QWheelEvent* event) {
  position_ += QMatrix4x4(eye_rotation_.conjugated().toRotationMatrix())
                   .mapVector(QVector3D(0.f, 0.f, 1.f) * (1.f / 8.f / 90.f) *
                              event->angleDelta().y());
  return true;
}

/*****************************************************************************/
/* SpinManipulator */
/*****************************************************************************/

SpinManipulator::SpinManipulator(QObject* parent) : Manipulator(parent) {
  distance_ = 5.f;
  angle_ = 0.f;
  factor_ = 1.f;
  time_ = 2.f;
  base_rotation_ =
      QQuaternion::fromAxisAndAngle(QVector3D(0.f, 0.f, 1.f), 90.f);
}

QMatrix4x4 SpinManipulator::transform() {
  QMatrix4x4 m;
  QVector3D dest_translation(0.f, 0.f, -distance_);

  m.setToIdentity();
  m.translate(factor_ * dest_translation + (1.f - factor_) * init_position_);
  m.rotate(QQuaternion::slerp(init_rotation_, base_rotation_, factor_));
  m.rotate(angle_, .5f, 1.f, 0.f);

  return m;
}

void SpinManipulator::initFromMatrix(const QMatrix4x4& m) {
  decompose(m, &init_position_, &init_rotation_);
  init_rotation_ = init_rotation_.conjugated();
  factor_ = 0.f;
  angle_ = 0.f;

  static float rad2deg = 180.f / std::acos(-1.f);
  float d_angle_deg = QQuaternion::dotProduct(init_rotation_, base_rotation_);
  d_angle_deg = d_angle_deg < 0.f ? -d_angle_deg : d_angle_deg;
  d_angle_deg = rad2deg * acos(d_angle_deg);
  time_ = std::min(d_angle_deg / angular_speed_, 2.f);
}

void SpinManipulator::update(float dt) {
  angle_ += angular_speed_ * dt;
  factor_ += dt / time_;
  if (factor_ > 1.f) {
    factor_ = 1.f;
  }
}

QString SpinManipulator::manipulatorName() { return tr("Spin manipulator"); }

bool SpinManipulator::handlesPause() { return true; }

}  // namespace visualization
}  // namespace veles
