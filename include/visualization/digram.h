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

#include "visualization/digram.h"

#include <cstdint>

#include <vector>

#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_3_2_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>

#include "visualization/base.h"

namespace veles {
namespace visualization {

class DigramWidget : public VisualizationWidget {
  Q_OBJECT

 public:
  explicit DigramWidget(QWidget* parent = nullptr);
  ~DigramWidget() override;

 protected:
  void refresh(const AdditionalResampleDataPtr& ad) override;
  bool initializeVisualizationGL() override;

  void resizeGLImpl(int w, int h) override;
  void paintGLImpl() override;

  void initShaders();
  void initTextures();
  void initGeometry();

 private:
  QOpenGLShaderProgram program_;
  QOpenGLTexture* texture_ = nullptr;

  QOpenGLBuffer square_vertex_;
  QOpenGLVertexArrayObject vao_;
};

}  // namespace visualization
}  // namespace veles
