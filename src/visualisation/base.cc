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
#include "visualisation/base.h"
#include "util/sampling/fake_sampler.h"
#include "util/sampling/uniform_sampler.h"
#include <QComboBox>
#include <QLabel>

namespace veles {
namespace visualisation {

VisualisationWidget::VisualisationWidget(QWidget *parent) :
  QOpenGLWidget(parent), initialised_(false), gl_initialised_(false),
  sampler_(nullptr) {}

void VisualisationWidget::setSampler(util::ISampler *sampler) {
  sampler_ = sampler;
  initialised_ = true;
  refreshVisualisation();
}

void VisualisationWidget::refreshVisualisation() {
  if (gl_initialised_) {
    refresh();
  }
}

void VisualisationWidget::initializeGL() {
  initializeVisualisationGL();
  gl_initialised_ = true;
}

size_t VisualisationWidget::getDataSize() {
  if (!initialised_ || sampler_->empty()) {
    return 0;
  }
  return sampler_->getSampleSize();
}

const char* VisualisationWidget::getData() {
  if (!initialised_ || sampler_->empty()) {
    return nullptr;
  }
  return sampler_->data();
}

char VisualisationWidget::getByte(size_t index) {
  return (*sampler_)[index];
}

bool VisualisationWidget::prepareOptionsPanel(QBoxLayout *layout) {
  return false;
}

}  // namespace visualisation
}  // namespace veles
