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
#pragma once

#include <QEvent>
#include <QMatrix4x4>
#include <QMouseEvent>
#include <QObject>
#include <QQuaternion>
#include <QVector3D>
#include <QWheelEvent>

namespace veles {
namespace visualization {

/*****************************************************************************/
/* Manipulator */
/*****************************************************************************/

class Manipulator : public QObject {
  Q_OBJECT

 protected:
  static bool isMouseEvent(QEvent* event) {
    return event->type() == QEvent::MouseButtonDblClick ||
           event->type() == QEvent::MouseButtonPress ||
           event->type() == QEvent::MouseButtonRelease ||
           event->type() == QEvent::MouseMove;
  }

  static bool isKeyEvent(QEvent* event) {
    return event->type() == QEvent::KeyPress ||
           event->type() == QEvent::KeyRelease;
  }

  static bool isWheelEvent(QEvent* event) {
    return event->type() == QEvent::Wheel;
  }

 public:
  explicit Manipulator(QObject* parent = nullptr);
  virtual QMatrix4x4 transform() = 0;
  virtual void initFromMatrix(const QMatrix4x4& /*m*/) {}
  virtual void update(float /*dt*/) {}
  virtual QString manipulatorName() = 0;
  virtual bool processEvent(QObject* watched, QEvent* event);
  virtual bool handlesPause();
};

/*****************************************************************************/
/* TrackballManipulator */
/*****************************************************************************/

class TrackballManipulator : public Manipulator {
  Q_OBJECT

 public:
  explicit TrackballManipulator(QObject* parent = nullptr);
  QMatrix4x4 transform() override;
  void initFromMatrix(const QMatrix4x4& m) override;
  void update(float dt) override;
  QString manipulatorName() override;

 protected:
  bool eventFilter(QObject* watched, QEvent* event) override;
  virtual bool mouseEvent(QWidget* widget, QMouseEvent* event);
  virtual bool wheelEvent(QWidget* widget, QWheelEvent* event);

  float distance_ = 5.f;
  float min_distance_ = 1.5f;
  float max_distance_ = 6.5f;
  QVector3D init_position_;
  QQuaternion rotation_;

  bool lmb_drag_;
  QPoint prev_pos_;

  float factor_;
  float time_;
};

/*****************************************************************************/
/* FreeManipulator */
/*****************************************************************************/

class FreeManipulator : public Manipulator {
  Q_OBJECT

 public:
  explicit FreeManipulator(QObject* parent = nullptr);
  QMatrix4x4 transform() override;
  void initFromMatrix(const QMatrix4x4& m) override;
  void update(float dt) override;
  QString manipulatorName() override;

  static bool isCtrlButton(int key);

 protected:
  bool eventFilter(QObject* watched, QEvent* event) override;
  virtual bool keyboardEvent(QKeyEvent* event);
  virtual bool mouseEvent(QObject* watched, QMouseEvent* event);
  virtual bool wheelEvent(QObject* watched, QWheelEvent* event);

  static const int key_left = Qt::Key_A;
  static const int key_right = Qt::Key_D;
  static const int key_up = Qt::Key_Q;
  static const int key_down = Qt::Key_Z;
  static const int key_forward = Qt::Key_W;
  static const int key_back = Qt::Key_S;

  QVector3D position_;
  QQuaternion eye_rotation_;
  QQuaternion cube_rotation_;

  bool lmb_drag_;
  QPoint prev_pos_;

  bool left_, right_, up_, down_, forward_, back_;
};

/*****************************************************************************/
/* SpinManipulator */
/*****************************************************************************/

class SpinManipulator : public Manipulator {
  Q_OBJECT

 public:
  explicit SpinManipulator(QObject* parent = nullptr);
  QMatrix4x4 transform() override;
  void initFromMatrix(const QMatrix4x4& m) override;
  void update(float dt) override;
  QString manipulatorName() override;
  bool handlesPause() override;

 protected:
  float distance_;
  QQuaternion init_rotation_;
  QVector3D init_position_;
  float angle_;
  float factor_;
  float time_;
  QQuaternion base_rotation_;

  const float angular_speed_ = 30.f;  // deg/s
};

}  // namespace visualization
}  // namespace veles
