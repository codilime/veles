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
#ifndef VELES_VISUALISATION_PANEL_H
#define VELES_VISUALISATION_PANEL_H

#include <QWidget>
#include <QBoxLayout>
#include <QAction>
#include <QToolBar>
#include <QSplitter>
#include <QLabel>
#include <QString>

#include <map>

#include "visualisation/base.h"
#include "visualisation/minimap_panel.h"

namespace veles {
namespace visualisation {

class VisualisationPanel : public QWidget {
  Q_OBJECT

 public:
  explicit VisualisationPanel(QWidget *parent = 0);
  ~VisualisationPanel();

  void setData(const QByteArray &data);
  void setRange(const size_t start, const size_t end);

 private slots:
  void setSamplingMethod(const QString &name);
  void setSampleSize(int kilobytes);
  void showDigramVisualisation();
  void showTrigramVisualisation();
  void showLayeredDigramVisualisation();
  void minimapSelectionChanged(size_t start, size_t end);

 private:
  enum class ESampler {NO_SAMPLER, UNIFORM_SAMPLER};
  enum class EVisualisation {DIGRAM, TRIGRAM, LAYERED_DIGRAM};

  static const std::map<QString, ESampler> k_sampler_map;
  static const ESampler k_default_sampler = ESampler::UNIFORM_SAMPLER;
  static const EVisualisation k_default_visualisation = EVisualisation::TRIGRAM;
  static const int k_max_sample_size = 128 * 1024;
  static const int k_minimap_sample_size = 4096;

  static util::ISampler* getSampler(ESampler type,
                                    const QByteArray &data,
                                    int sample_size);
  static VisualisationWidget* getVisualisation(EVisualisation type,
                                               QWidget *parent = 0);
  static QString prepareAddressString(size_t start, size_t end);

  void setVisualisation(EVisualisation type);
  void refreshVisualisation();
  void initLayout();
  void initOptionsPanel();
  QBoxLayout* prepareVisualisationOptions();

  QByteArray data_;
  ESampler sampler_type_;
  EVisualisation visualisation_type_;
  int sample_size_;
  util::ISampler *sampler_, *minimap_sampler_;
  MinimapPanel *minimap_;
  VisualisationWidget *visualisation_;

  QSpinBox *sample_size_box_;
  QBoxLayout *layout_, *options_layout_;
  QSplitter *splitter_;
  QWidget *child_options_wrapper_;
  QAction *digram_action_, *trigram_action_, *layered_digram_action_;
  QToolBar *visualisation_toolbar_;
  QLabel *selection_label_;
};

}  //  namespace visualisation
}  //  namespace veles

#endif  //  VELES_VISUALISATION_PANEL_H
