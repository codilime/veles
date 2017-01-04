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
#ifndef VELES_VISUALISATION_TRIGRAM_H
#define VELES_VISUALISATION_TRIGRAM_H

#include "visualisation/trigram.h"

#include <stdint.h>

#include <vector>

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_2_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QBasicTimer>
#include <QPushButton>
#include <QSlider>
#include <QCheckBox>
#include <QTime>
#include <QToolBar>
#include <QAction>

#include "visualisation/base.h"
#include "visualisation/manipulator.h"

namespace veles {
namespace visualisation {

/*****************************************************************************/
/* LabelPositionMixer */
/*****************************************************************************/

class LabelPositionMixer {
 public:
  LabelPositionMixer(){}
  LabelPositionMixer(
      QVector4D s0c0p0, QVector4D s1c0p0,
      QVector4D s0c1p0, QVector4D s0c1p1,
      QVector4D s0c0p1, QVector4D s1c0p1) {
    s0c0p0_ = s0c0p0;
    s1c0p0_ = s1c0p0;
    s0c1p0_ = s0c1p0;
    s0c1p1_ = s0c1p1;
    s0c0p1_ = s0c0p1;
    s1c0p1_ = s1c0p1;
  }

  QVector4D interpolate(QVector4D a, QVector4D b, float f) {
    return (1.f - f) * a + f * b;
  }

  QVector4D mix(float sph, float cyl, float pos) {
    QVector4D sp0 = interpolate(s0c0p0_, s1c0p0_, sph);
    QVector4D cp0 = interpolate(s0c0p0_, s0c1p0_, cyl);
    QVector4D sp1 = interpolate(s0c0p1_, s1c0p1_, sph);
    QVector4D cp1 = interpolate(s0c0p1_, s0c1p1_, cyl);

    float f = 0.f;
    if(sph + cyl != 0.f) {
      f = cyl / (sph + cyl);
    }

    QVector4D p0 = interpolate(sp0, cp0, f);
    QVector4D p1 = interpolate(sp1, cp1, f);
    return interpolate(p0, p1, pos);
  }

 private:
  QVector4D s0c0p0_;
  QVector4D s1c0p0_;
  QVector4D s0c1p0_;
  QVector4D s0c1p1_;
  QVector4D s0c0p1_;
  QVector4D s1c0p1_;
};

/*****************************************************************************/
/* TrigramWidget */
/*****************************************************************************/

class TrigramWidget : public VisualisationWidget {
  Q_OBJECT

 public:
  enum class EVisualisationShape {CUBE, CYLINDER, SPHERE};
  enum class EVisualisationMode {TRIGRAM, LAYERED_DIGRAM};

  explicit TrigramWidget(QWidget *parent = 0);
  ~TrigramWidget();

  bool prepareOptionsPanel(QBoxLayout *layout) override;
  void setMode(EVisualisationMode mode, bool animate = true);

  static float vfovDeg(float min_fov_deg, float aspect_ratio);

 public slots:
  void brightnessSliderMoved(int value);

 protected:
  void refresh() override;
  bool initializeVisualisationGL() override;

  bool event(QEvent *event) override;
  void timerEvent(QTimerEvent *e) override;

  void resizeGLImpl(int w, int h) override;
  void paintGLImpl() override;

  void paintLabels(QMatrix4x4& scene_mp, QMatrix4x4& scene_m);
  void paintLabel(LabelPositionMixer& mixer, QMatrix4x4& scene_to_screen,
      QMatrix4x4& screen_mp);
  void initLabels();
  void releaseLabels();
  QVector3D calcScreenPosForLabel(QVector3D world_pos,
      QMatrix4x4& scene_to_screen, int width, int height);
  void paintRF(QMatrix4x4& mvp);
  void initRF();
  void releaseRF();
  void initLabelPositionMixers();

  void initShaders();
  void initTextures();
  void initGeometry();

  QAction* createAction(const QIcon& icon, Manipulator* manipulator,
      const QList<QKeySequence>& sequences);
  QAbstractButton* createActionButton(QAction* action);
  virtual void prepareManipulatorToolbar(QBoxLayout *layout);

 signals:
  void manipulatorChanged(Manipulator* manipulator);

 private slots:
  void playPause();
  void setShape(EVisualisationShape shape);
  void setUseBrightnessHeuristic(int state);
  void setManipulator(Manipulator* manipulator);

 private:
  void setBrightness(int value);
  int suggestBrightness(); // heuristic
  void autoSetBrightness();
  QIcon getColoredIcon(QString path, bool black_only = true);

  QBasicTimer timer;
  QOpenGLShaderProgram program;
  QOpenGLTexture *texture;
  QOpenGLBuffer *databuf;

  QOpenGLVertexArrayObject vao;
  float angle;
  float c_sph, c_cyl, c_pos, c_brightness;
  int width, height;
  EVisualisationShape shape_;
  EVisualisationMode mode_;
  int brightness_;

  QPushButton *pause_button_, *cube_button_, *cylinder_button_, *sphere_button_;
  QSlider *brightness_slider_;
  QCheckBox *use_heuristic_checkbox_;
  bool is_playing_, use_brightness_heuristic_;
  QCheckBox* show_labels_and_rf_checkbox_;
  bool show_labels_and_rf_;

  QList<Manipulator*> manipulators_;
  Manipulator* current_manipulator_;
  SpinManipulator* spin_manipulator_;
  TrackballManipulator* trackball_manipulator_;
  FreeManipulator* free_manipulator_;
  QTime time_;

  QOpenGLShaderProgram label_program_;
  QOpenGLVertexArrayObject label_vao_;
  QOpenGLBuffer label_vb_;
  QOpenGLShaderProgram rf_program_;
  QOpenGLVertexArrayObject rf_vao_;
  QOpenGLBuffer rf_vb_;
  QOpenGLTexture* texture_0_;
  QOpenGLTexture* texture_1_;
  QOpenGLTexture* texture_2_;
  QOpenGLTexture* texture_3_;
  QOpenGLTexture* texture_pos_;
  QOpenGLTexture* texture_N0_;

  LabelPositionMixer lpm_0_, lpm_1_, lpm_2_, lpm_3_, lpm_pos_, lpm_N0_;
};

}  // namespace visualisation
}  // namespace veles

#endif // VELES_VISUALISATION_TRIGRAM_H
