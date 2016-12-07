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
#include <QComboBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLayoutItem>
#include <QLabel>

#include "visualisation/panel.h"
#include "util/sampling/fake_sampler.h"
#include "util/sampling/uniform_sampler.h"
#include "visualisation/digram.h"
#include "visualisation/trigram.h"

namespace veles {
namespace visualisation {

const std::map<QString, VisualisationPanel::ESampler>
  VisualisationPanel::k_sampler_map = {
    {"No sampling", VisualisationPanel::ESampler::NO_SAMPLER},
    {"Uniform random sampling", VisualisationPanel::ESampler::UNIFORM_SAMPLER}
};

/*****************************************************************************/
/* Public methods */
/*****************************************************************************/

VisualisationPanel::VisualisationPanel(QWidget *parent) :
  sampler_type_(k_default_sampler),
  visualisation_type_(k_default_visualisation), sample_size_(1024) {
    sampler_ = getSampler(sampler_type_, data_, sample_size_);
    minimap_sampler_ = getSampler(ESampler::UNIFORM_SAMPLER,
                                  data_, k_minimap_sample_size);
    minimap_ = new MinimapPanel(this);
    minimap_->setSampler(minimap_sampler_);
    connect(minimap_, SIGNAL(selectionChanged(size_t, size_t)), this,
            SLOT(minimapSelectionChanged(size_t, size_t)));

    visualisation_ = getVisualisation(visualisation_type_, this);
    initLayout();
}

VisualisationPanel::~VisualisationPanel() {
  delete minimap_;
  if (sampler_ != nullptr) {
    delete sampler_;
  }
  if (minimap_sampler_ != nullptr) {
    delete minimap_sampler_;
  }
}

void VisualisationPanel::setData(const QByteArray &data) {
  if (sampler_ != nullptr) {
    delete sampler_;
  }
  if (minimap_sampler_ != nullptr) {
    delete minimap_sampler_;
  }
  data_ = data;
  sampler_ = getSampler(sampler_type_, data_, sample_size_);
  minimap_sampler_ = getSampler(ESampler::UNIFORM_SAMPLER,
                                data_, k_minimap_sample_size);
  minimap_->setSampler(minimap_sampler_);
  visualisation_->setSampler(sampler_);
  selection_label_->setText(prepareAddressString(0,
                            sampler_->getFileOffset(sampler_->getSampleSize() - 1)));

}

void VisualisationPanel::setRange(const size_t start, const size_t end) {
  sampler_->setRange(start, end);
  refreshVisualisation();
}

/*****************************************************************************/
/* Static factory methods */
/*****************************************************************************/

util::ISampler* VisualisationPanel::getSampler(ESampler type,
                                          const QByteArray &data,
                                          int sample_size) {
  switch (type) {
  case ESampler::NO_SAMPLER:
    return new util::FakeSampler(data);
  case ESampler::UNIFORM_SAMPLER:
    util::UniformSampler *sampler = new util::UniformSampler(data);
    sampler->setSampleSize(1024 * sample_size);
    return sampler;
  }
  return nullptr;
}

VisualisationWidget* VisualisationPanel::getVisualisation(EVisualisation type,
                                                          QWidget* parent) {
  TrigramWidget* trigram = nullptr;
  switch (type) {
  case EVisualisation::DIGRAM:
    return new DigramWidget(parent);
  case EVisualisation::TRIGRAM:
    trigram = new TrigramWidget(parent);
    trigram->setMode(TrigramWidget::EVisualisationMode::TRIGRAM);
    return trigram;
  case EVisualisation::LAYERED_DIGRAM:
    trigram = new TrigramWidget(parent);
    trigram->setMode(TrigramWidget::EVisualisationMode::LAYERED_DIGRAM, false);
    return trigram;
  }
  return nullptr;
}

QString VisualisationPanel::prepareAddressString(size_t start, size_t end) {
  auto label = QString("0x%1 : ").arg(start, 8, 16, QChar('0'));
  label.append(QString("0x%1").arg(end, 8, 16, QChar('0')));
  return label;
}

/*****************************************************************************/
/* Private slots */
/*****************************************************************************/

void VisualisationPanel::setSamplingMethod(const QString &name) {
  auto new_sampler_type = k_sampler_map.at(name);
  if (new_sampler_type == sampler_type_) return;

  auto old_sampler = sampler_;
  sampler_ = getSampler(new_sampler_type, data_, sample_size_);
  auto selection = minimap_->getSelection();
  sampler_->setRange(selection.first, selection.second);
  visualisation_->setSampler(sampler_);
  if (old_sampler != nullptr) {
    delete old_sampler;
  }
  sampler_type_ = new_sampler_type;
  sample_size_box_->setEnabled(sampler_type_ == ESampler::UNIFORM_SAMPLER);
}

void VisualisationPanel::setSampleSize(int kilobytes) {
  sample_size_ = kilobytes;
  if (sampler_type_ == ESampler::UNIFORM_SAMPLER) {
    sampler_->setSampleSize(1024 * kilobytes);
    refreshVisualisation();
  }
}

void VisualisationPanel::showDigramVisualisation() {
  setVisualisation(EVisualisation::DIGRAM);
}

void VisualisationPanel::showTrigramVisualisation() {
  if (visualisation_type_ == EVisualisation::LAYERED_DIGRAM) {
    visualisation_type_ = EVisualisation::TRIGRAM;
    auto trigram = static_cast<TrigramWidget*>(visualisation_);
    trigram->setMode(TrigramWidget::EVisualisationMode::TRIGRAM);
  } else {
    setVisualisation(EVisualisation::TRIGRAM);
  }
}

void VisualisationPanel::showLayeredDigramVisualisation() {
 if (visualisation_type_ == EVisualisation::TRIGRAM) {
    visualisation_type_ = EVisualisation::LAYERED_DIGRAM;
    auto trigram = static_cast<TrigramWidget*>(visualisation_);
    trigram->setMode(TrigramWidget::EVisualisationMode::LAYERED_DIGRAM);
  } else {
    setVisualisation(EVisualisation::LAYERED_DIGRAM);
  }
}

void VisualisationPanel::minimapSelectionChanged(size_t start, size_t end) {
  selection_label_->setText(prepareAddressString(start, end));
  sampler_->setRange(start, end);
  refreshVisualisation();
}

/*****************************************************************************/
/* Private methods */
/*****************************************************************************/

void VisualisationPanel::setVisualisation(EVisualisation type) {
  if (type != visualisation_type_) {
    VisualisationWidget *old = visualisation_;
    auto sizes = splitter_->sizes();
    visualisation_type_ = type;
    visualisation_ = getVisualisation(visualisation_type_, this);
    visualisation_->setSampler(sampler_);

    splitter_->addWidget(visualisation_);
    old->hide();

    QWidget *old_cow = child_options_wrapper_;
    child_options_wrapper_ = new QWidget;
    child_options_wrapper_->setLayout(prepareVisualisationOptions());
    child_options_wrapper_->resize(200, 480);
    QLayoutItem *cow_litem = options_layout_->replaceWidget(old_cow,
        child_options_wrapper_);

    delete cow_litem;
    delete old_cow;
    delete old;
    splitter_->setSizes(sizes);
  }
}

void VisualisationPanel::refreshVisualisation() {
  if (visualisation_ != nullptr) {
    visualisation_->refreshVisualisation();
  }
}

void VisualisationPanel::initLayout() {
  initOptionsPanel();

  splitter_ = new QSplitter(Qt::Horizontal);
  splitter_->addWidget(minimap_);
  splitter_->addWidget(visualisation_);
  splitter_->setStretchFactor(1, 5);

  layout_ = new QHBoxLayout;
  layout_->setContentsMargins(0, 0, 0, 0);
  layout_->addWidget(splitter_, 8);
  layout_->addLayout(options_layout_, 0);
  this->setLayout(layout_);
}

QBoxLayout* VisualisationPanel::prepareVisualisationOptions() {
  QBoxLayout *child_layout = new QVBoxLayout;
  child_layout->setContentsMargins(0, 0, 0, 0);
  visualisation_->prepareOptionsPanel(child_layout);
  return child_layout;
}

void VisualisationPanel::initOptionsPanel() {
  options_layout_ = new QVBoxLayout;

  digram_action_ =
      new QAction(QIcon(":/images/nginx2d_32.png"), tr("&Digram"), this);
  digram_action_->setToolTip("Digram Visualisation");
  connect(digram_action_, SIGNAL(triggered()), this,
          SLOT(showDigramVisualisation()));

  trigram_action_ =
      new QAction(QIcon(":/images/nginx3d_32.png"), tr("&Trigram"), this);
  trigram_action_->setToolTip("Trigram Visualisation");
  connect(trigram_action_, SIGNAL(triggered()), this,
          SLOT(showTrigramVisualisation()));

  layered_digram_action_ =
      new QAction(QIcon(":/images/nginx3d_32.png"), tr("&Layered Digram"), this);
  layered_digram_action_->setToolTip("Layered Digram Visualisation");
  connect(layered_digram_action_, SIGNAL(triggered()), this,
          SLOT(showLayeredDigramVisualisation()));

  visualisation_toolbar_ = new QToolBar("Visualisation Type");
  visualisation_toolbar_->addAction(digram_action_);
  visualisation_toolbar_->addAction(trigram_action_);
  visualisation_toolbar_->addAction(layered_digram_action_);
  options_layout_->addWidget(visualisation_toolbar_);

  QLabel *sampling_label = new QLabel("Sampling method:");
  sampling_label->setAlignment(Qt::AlignTop);
  options_layout_->addWidget(sampling_label);

  QComboBox *sampling_method = new QComboBox;
  sampling_method->addItem("Uniform random sampling");
  sampling_method->addItem("No sampling");
  options_layout_->addWidget(sampling_method);

  QLabel *sample_size_label = new QLabel("Sample size (KB):");
  sample_size_label->setAlignment(Qt::AlignTop);
  options_layout_->addWidget(sample_size_label);

  sample_size_box_ = new QSpinBox;
  sample_size_box_->setMinimum(256);
  sample_size_box_->setMaximum(k_max_sample_size);
  sample_size_box_->setSingleStep(1024);
  sample_size_box_->setValue(sample_size_);
  sample_size_box_->setEnabled(sampler_type_ == ESampler::UNIFORM_SAMPLER);
  options_layout_->addWidget(sample_size_box_);

  connect(sampling_method, SIGNAL(currentIndexChanged(const QString&)),
          this, SLOT(setSamplingMethod(const QString&)));
  connect(sample_size_box_, SIGNAL(valueChanged(int)),
          this, SLOT(setSampleSize(int)));

  child_options_wrapper_ = new QWidget;
  child_options_wrapper_->setLayout(prepareVisualisationOptions());
  child_options_wrapper_->resize(200, 480);
  options_layout_->addWidget(child_options_wrapper_, 0);

  options_layout_->addStretch(10);
  selection_label_ = new QLabel(prepareAddressString(0, 0));
  selection_label_->setAlignment(Qt::AlignCenter);
  selection_label_->setTextInteractionFlags(Qt::TextSelectableByMouse);
  options_layout_->addWidget(selection_label_);
}

}  // namespace visualisation
}  // namespace veles
