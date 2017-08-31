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

#include <QBoxLayout>
#include <QOpenGLFunctions_3_2_Core>
#include <QOpenGLWidget>
#include <QSpinBox>
#include <QString>

#include <map>
#include <memory>

#include <QMainWindow>

#include "util/sampling/isampler.h"

namespace veles {
namespace visualization {

class VisualizationWidget : public QOpenGLWidget,
                            protected QOpenGLFunctions_3_2_Core {
  Q_OBJECT

 public:
  explicit VisualizationWidget(QWidget* parent = nullptr);
  ~VisualizationWidget() override;

  void setSampler(util::ISampler* sampler);

  // This method takes a visualization window (QMainWindow*) and adds any
  // toolbars necessary to manipulate options of this visualization.
  virtual void prepareOptions(QMainWindow* visualization_window);

  /**
   * Derive this if you want to produce some additional data in
   * onAsyncResample method.
   */
  struct AdditionalResampleData {
    virtual ~AdditionalResampleData() {}
  };
  typedef std::shared_ptr<AdditionalResampleData> AdditionalResampleDataPtr;

  void refreshVisualization(
      const AdditionalResampleDataPtr& ad = AdditionalResampleDataPtr());

 signals:
  void resampled(AdditionalResampleDataPtr ad);

 protected:
  void initializeGL() override;
  void resizeGL(int w, int h) override;
  void paintGL() override;

  virtual bool initializeVisualizationGL() = 0;
  virtual void refresh(const AdditionalResampleDataPtr& ad) = 0;
  virtual void paintGLImpl() = 0;
  virtual void resizeGLImpl(int w, int h) = 0;
  void resampleCallback();

  /**
   * This will be called in worker thread when new sample is ready.
   * Derive this method to do some additional processing in worker thread.
   * Keep in mind that this method will be executed while holding sampler
   * lock, so doing very expensive stuff here might hurt your performance.
   * Return value of this method will be passed along with resampled() signal
   * and in particular will be passed to refresh().
   */
  virtual AdditionalResampleData* onAsyncResample() { return nullptr; }

  size_t getDataSize();
  const char* getData();
  char getByte(size_t index);

 private:
  bool initialized_ = false;
  bool gl_initialized_ = false;
  bool gl_broken_ = false;
  bool error_message_set_ = false;
  util::ISampler* sampler_ = nullptr;
  util::ResampleCallbackId resample_cb_id_;
};

}  // namespace visualization
}  // namespace veles
