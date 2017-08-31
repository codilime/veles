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
#include "visualization/digram.h"

namespace veles {
namespace visualization {

DigramWidget::DigramWidget(QWidget* parent) : VisualizationWidget(parent) {}

DigramWidget::~DigramWidget() {
  if (texture_ == nullptr) {
    return;
  }
  makeCurrent();
  delete texture_;
  square_vertex_.destroy();
  doneCurrent();
}

void DigramWidget::refresh(const AdditionalResampleDataPtr& /*ad*/) {
  makeCurrent();
  delete texture_;
  initTextures();
  doneCurrent();
  update();
}

bool DigramWidget::initializeVisualizationGL() {
  if (!initializeOpenGLFunctions()) {
    return false;
  }

  glClearColor(0, 0, 0, 1);

  initShaders();
  initTextures();
  initGeometry();
  return true;
}

void DigramWidget::initShaders() {
  if (!program_.addShaderFromSourceFile(QOpenGLShader::Vertex,
                                        ":/digram/vshader.glsl")) {
    close();
  }

  // Compile fragment shader
  if (!program_.addShaderFromSourceFile(QOpenGLShader::Fragment,
                                        ":/digram/fshader.glsl")) {
    close();
  }

  // Link shader pipeline
  if (!program_.link()) {
    close();
  }

  // Bind shader pipeline for use
  if (!program_.bind()) {
    close();
  }
}

void DigramWidget::initTextures() {
  texture_ = new QOpenGLTexture(QOpenGLTexture::Target2D);
  texture_->setSize(256, 256);
  texture_->setFormat(QOpenGLTexture::RG32F);
  texture_->allocateStorage();

  // effectively arrays of size [256][256][2], represented as single blocks
  auto bigtab = new uint64_t[256 * 256 * 2];
  memset(bigtab, 0, 256 * 256 * 2 * sizeof(*bigtab));
  auto ftab = new float[256 * 256 * 2];
  const auto* rowdata = reinterpret_cast<const uint8_t*>(getData());
  for (size_t i = 0; i < getDataSize() - 1; i++) {
    size_t index = rowdata[i] * 512 + rowdata[i + 1] * 2;
    bigtab[index]++;
    bigtab[index + 1] += i;
  }
  for (int i = 0; i < 256; i++) {
    for (int j = 0; j < 256; j++) {
      int index = i * 512 + j * 2;
      ftab[index] = static_cast<float>(bigtab[index]) / getDataSize();
      ftab[index + 1] =
          static_cast<float>(bigtab[index + 1]) / getDataSize() / getDataSize();
    }
  }
  texture_->setData(QOpenGLTexture::RG, QOpenGLTexture::Float32,
                    reinterpret_cast<void*>(ftab));
  texture_->generateMipMaps();

  texture_->setMinificationFilter(QOpenGLTexture::Nearest);

  texture_->setMagnificationFilter(QOpenGLTexture::Linear);

  texture_->setWrapMode(QOpenGLTexture::ClampToEdge);

  delete[] bigtab;
  delete[] ftab;
}

void DigramWidget::initGeometry() {
  square_vertex_.create();
  QVector2D v[] = {
      {0, 0}, {0, 1}, {1, 0}, {1, 1},
  };
  square_vertex_.bind();
  square_vertex_.allocate(v, sizeof v);
  vao_.create();
}

void DigramWidget::resizeGLImpl(int /*w*/, int /*h*/) {}

void DigramWidget::paintGLImpl() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  texture_->bind();
  vao_.bind();

  square_vertex_.bind();
  int vertexLocation = program_.attributeLocation("a_position");
  program_.enableAttributeArray(vertexLocation);
  program_.setAttributeBuffer(vertexLocation, GL_FLOAT, 0, 2,
                              sizeof(QVector2D));
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  program_.setUniformValue("tx", 0);
}

}  // namespace visualization
}  // namespace veles
