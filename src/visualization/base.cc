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

#include "visualization/base.h"
#include <QComboBox>
#include <QLabel>
#include <functional>
#include "util/sampling/fake_sampler.h"
#include "util/sampling/uniform_sampler.h"

namespace veles {
namespace visualization {

VisualizationWidget::VisualizationWidget(QWidget* parent)
    : QOpenGLWidget(parent) {
  connect(this, &VisualizationWidget::resampled, this,
          &VisualizationWidget::refreshVisualization);
}

VisualizationWidget::~VisualizationWidget() {
  if (sampler_ != nullptr) {
    sampler_->removeResampleCallback(resample_cb_id_);
  }
}

void VisualizationWidget::setSampler(util::ISampler* sampler) {
  if (sampler_ != nullptr) {
    sampler_->removeResampleCallback(resample_cb_id_);
  }
  sampler_ = sampler;
  resample_cb_id_ = sampler_->registerResampleCallback(std::function<void()>(
      std::bind(&VisualizationWidget::resampleCallback, this)));
  initialized_ = true;
  refreshVisualization();
}

void VisualizationWidget::refreshVisualization(
    const AdditionalResampleDataPtr& ad) {
  if (gl_initialized_ && !error_message_set_) {
    auto lc = sampler_->lock();
    refresh(ad);
  }
}

void VisualizationWidget::paintGL() {
  if (error_message_set_) {
    return;
  }
  if (gl_broken_) {
    auto* layout = new QVBoxLayout();
    auto error_label = new QLabel(tr(
        "Failed to initialize OpenGL functions. Veles visualization requires "
        "OpenGL 3.3 core profile support. Please check if this is supported "
        "by your graphic card and drivers."));
    error_label->setAlignment(Qt::AlignCenter);
    error_label->setWordWrap(true);
    layout->addWidget(error_label);
    setLayout(layout);
    error_message_set_ = true;
  } else if (gl_initialized_) {
    paintGLImpl();
  }
}

void VisualizationWidget::resizeGL(int w, int h) {
  if (!gl_initialized_ || gl_broken_) {
    return;
  }
  resizeGLImpl(w, h);
}

void VisualizationWidget::initializeGL() {
  if (gl_initialized_ || gl_broken_) {
    return;
  }
  if (initializeVisualizationGL()) {
    gl_initialized_ = true;
  } else {
    gl_broken_ = true;
  }
}

size_t VisualizationWidget::getDataSize() {
  if (!initialized_ || sampler_->empty()) {
    return 0;
  }
  return sampler_->getSampleSize();
}

const char* VisualizationWidget::getData() {
  if (!initialized_ || sampler_->empty()) {
    return nullptr;
  }
  return sampler_->data();
}

char VisualizationWidget::getByte(size_t index) { return (*sampler_)[index]; }

void VisualizationWidget::prepareOptions(
    QMainWindow* /*visualization_window*/) {}

void VisualizationWidget::resampleCallback() {
  AdditionalResampleDataPtr additionalData(onAsyncResample());
  emit resampled(additionalData);
}

}  // namespace visualization
}  // namespace veles
