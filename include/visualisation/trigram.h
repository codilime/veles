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
#ifndef TRIGRAM_H
#define TRIGRAM_H

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

#include "visualisation/base.h"

namespace veles {
namespace visualisation {

class TrigramWidget : public VisualisationWidget {
  Q_OBJECT

 public:
  enum class EVisualisationShape {CUBE, CYLINDER, SPHERE};
  enum class EVisualisationMode {TRIGRAM, LAYERED_DIGRAM};

  explicit TrigramWidget(QWidget *parent = 0);
  ~TrigramWidget();

  bool prepareOptionsPanel(QBoxLayout *layout) override;
  void setMode(EVisualisationMode mode, bool animate = true);

 public slots:
  void brightnessSliderMoved(int value);

 protected:
  void refresh() override;
  void initializeVisualisationGL() override;

  void timerEvent(QTimerEvent *e) override;

  void resizeGL(int w, int h) override;
  void paintGL() override;

  void initShaders();
  void initTextures();
  void initGeometry();

 private slots:
  void playPause();
  void setShape(EVisualisationShape shape);
  void setUseBrightnessHeuristic(int state);

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
};

}  // namespace visualisation
}  // namespace veles

#endif
