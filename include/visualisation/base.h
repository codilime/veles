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
#ifndef VELES_VISUALISATION_BASE_H
#define VELES_VISUALISATION_BASE_H

#include <QString>
#include <QBoxLayout>
#include <QSpinBox>
#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_2_Core>

#include <map>
#include <memory>

#include "util/sampling/isampler.h"

namespace veles {
namespace visualisation {

class VisualisationWidget : public QOpenGLWidget,
                            protected QOpenGLFunctions_3_2_Core {
  Q_OBJECT

 public:
  explicit VisualisationWidget(QWidget *parent = 0);
  ~VisualisationWidget() {}

  void setSampler(util::ISampler *sampler);
  // This method takes a QLayout* and add any widgets necessary to manipulate
  // options of this visualisation. Return true if anything was added to
  // QLayout.
  virtual bool prepareOptionsPanel(QBoxLayout *layout);

  /**
   * Derive this if you want to produce some additional data in
   * onAsyncResample method.
   */
  struct AdditionalResampleData {
    virtual ~AdditionalResampleData() {}
  };
  typedef std::shared_ptr<AdditionalResampleData> AdditionalResampleDataPtr;

  void refreshVisualisation(AdditionalResampleDataPtr ad = AdditionalResampleDataPtr());

 signals:
  void resampled(AdditionalResampleDataPtr ad);

 protected:
  void initializeGL() override;
  void resizeGL(int w, int h) override;
  void paintGL() override;

  virtual bool initializeVisualisationGL() = 0;
  virtual void refresh(AdditionalResampleDataPtr ad) = 0;
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
  virtual AdditionalResampleData* onAsyncResample() {return nullptr;}

  size_t getDataSize();
  const char* getData();
  char getByte(size_t index);

 private:

  bool initialised_;
  bool gl_initialised_, gl_broken_, error_message_set_;
  util::ISampler *sampler_;
};

}  // namespace visualisation
}  // namespace veles

#endif  // VELES_VISUALISATION_BASE_H
