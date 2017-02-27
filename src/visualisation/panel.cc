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
#include "util/icons.h"
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

VisualisationPanel::VisualisationPanel(
    ui::MainWindowWithDetachableDockWidgets* main_window,
    QSharedPointer<ui::FileBlobModel>& data_model, QWidget *parent) :
    veles::ui::View("Visualization", ":/images/trigram_icon.png"),
    sampler_type_(k_default_sampler),
    visualisation_type_(k_default_visualisation), sample_size_(1024),
    data_model_(data_model), main_window_(main_window) {
  sampler_ = getSampler(sampler_type_, data_, sample_size_);
  sampler_->allowAsynchronousResampling(true);
  minimap_sampler_ = getSampler(ESampler::UNIFORM_SAMPLER, data_,
      k_minimap_sample_size);
  minimap_ = new MinimapPanel(this);
  minimap_->setSampler(minimap_sampler_);
  connect(minimap_, SIGNAL(selectionChanged(size_t, size_t)), this,
      SLOT(minimapSelectionChanged(size_t, size_t)));

  visualisation_ = getVisualisation(visualisation_type_, this);
  visualisation_root_ = new QMainWindow;
  visualisation_root_->setCentralWidget(visualisation_);

  sampling_method_dialog_ = new SamplingMethodDialog(this);
  sampling_method_dialog_->setSampleSize(sample_size_);
  sampling_method_dialog_->setMaximumSampleSize(k_max_sample_size);

  connect(sampling_method_dialog_,
      SIGNAL(samplingMethodChanged(const QString&)),
      this, SLOT(setSamplingMethod(const QString&)));
  connect(sampling_method_dialog_, SIGNAL(sampleSizeChanged(int)),
      this, SLOT(setSampleSize(int)));

  initLayout();
}

VisualisationPanel::~VisualisationPanel() {
  delete visualisation_;
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
  sampler_->allowAsynchronousResampling(true);
  minimap_sampler_ = getSampler(ESampler::UNIFORM_SAMPLER,
                                data_, k_minimap_sample_size);
  minimap_->setSampler(minimap_sampler_);
  visualisation_->setSampler(sampler_);
  selection_label_->setText(prepareAddressString(0,
                            sampler_->getFileOffset(sampler_->getSampleSize())));

}

void VisualisationPanel::setRange(const size_t start, const size_t end) {
  sampler_->setRange(start, end);
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
  label.append(QString("0x%1 ").arg(end, 8, 16, QChar('0')));
  label.append(QString("(%1 bytes)").arg(end - start));
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
  sampler_->allowAsynchronousResampling(true);
  auto selection = minimap_->getSelection();
  sampler_->setRange(selection.first, selection.second);
  visualisation_->setSampler(sampler_);
  if (old_sampler != nullptr) {
    delete old_sampler;
  }
  sampler_type_ = new_sampler_type;
}

void VisualisationPanel::setSampleSize(int kilobytes) {
  sample_size_ = kilobytes;
  if (sampler_type_ == ESampler::UNIFORM_SAMPLER) {
    sampler_->setSampleSize(1024 * kilobytes);
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
}

void VisualisationPanel::showMoreOptions() {
  sampling_method_dialog_->show();
}

/*****************************************************************************/
/* Private methods */
/*****************************************************************************/

void VisualisationPanel::setVisualisation(EVisualisation type) {
  if (type != visualisation_type_) {
    auto toolbars = visualisation_root_->findChildren<QToolBar*>();
    for (auto toolbar : toolbars) {
      if (toolbar->actions().empty() || !toolbar->property("common").toBool()) {
        toolbar->deleteLater();
      }
    }
    VisualisationWidget *old = visualisation_;
    visualisation_type_ = type;
    visualisation_ = getVisualisation(visualisation_type_, this);
    visualisation_->setSampler(sampler_);
    visualisation_root_->setCentralWidget(visualisation_);
    prepareVisualisationOptions();
    delete old;
  }
}

void VisualisationPanel::refreshVisualisation() {
  if (visualisation_ != nullptr) {
    visualisation_->refreshVisualisation();
  }
}

void VisualisationPanel::initLayout() {
  initOptionsPanel();

  setCentralWidget(visualisation_root_);
  setDockNestingEnabled(true);

  node_tree_dock_ = new QDockWidget;
  new ui::DockWidgetVisibilityGuard(node_tree_dock_);
  node_tree_dock_->setWindowTitle("Node tree");
  QSharedPointer<QItemSelectionModel> new_selection_model(
            new QItemSelectionModel(data_model_.data()));
  node_tree_widget_ = new ui::NodeTreeWidget(main_window_, data_model_,
      new_selection_model);
  node_tree_dock_->setWidget(node_tree_widget_);
  node_tree_dock_->setContextMenuPolicy(Qt::PreventContextMenu);
  node_tree_dock_->setAllowedAreas(
      Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
  node_tree_dock_->hide();
  addDockWidget(Qt::LeftDockWidgetArea, node_tree_dock_);

  minimap_dock_ = new QDockWidget;
  new ui::DockWidgetVisibilityGuard(minimap_dock_);
  minimap_dock_->setWindowTitle("Minimap");
  minimap_dock_->setWidget(minimap_);
  minimap_dock_->setContextMenuPolicy(Qt::PreventContextMenu);
  minimap_dock_->setAllowedAreas(
      Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
  ui::MainWindowWithDetachableDockWidgets::splitDockWidget2(this,
      node_tree_dock_, minimap_dock_, Qt::Horizontal);

  connect(show_node_tree_act_, &QAction::toggled,
      node_tree_dock_, &QDockWidget::setVisible);
  connect(show_minimap_act_, &QAction::toggled,
      minimap_dock_, &QDockWidget::setVisible);
}

void VisualisationPanel::prepareVisualisationOptions() {
  visualisation_->prepareOptions(visualisation_root_);
}

void VisualisationPanel::initOptionsPanel() {
  /////////////////////////////////////
  // Node tree / minimap
  show_node_tree_act_ = new QAction(QIcon(":/images/show_node_tree.png"),
      tr("&Node tree"), this);
  show_node_tree_act_->setToolTip(tr("Node tree"));
  show_node_tree_act_->setEnabled(true);
  show_node_tree_act_->setCheckable(true);
  show_node_tree_act_->setChecked(false);
  show_minimap_act_ = new QAction(QIcon(":/images/show_minimap.png"),
      tr("&Minimap"), this);
  show_minimap_act_->setToolTip(tr("Minimap"));
  show_minimap_act_->setEnabled(true);
  show_minimap_act_->setCheckable(true);
  show_minimap_act_->setChecked(true);

  tools_tool_bar_ = new QToolBar(tr("Tools"));
  tools_tool_bar_->setMovable(false);
  //tools_tool_bar_->addAction(show_node_tree_act_);
  tools_tool_bar_->addAction(show_minimap_act_);
  tools_tool_bar_->setContextMenuPolicy(Qt::PreventContextMenu);
  tools_tool_bar_->setProperty("common", true);
  tools_tool_bar_->addSeparator();
  visualisation_root_->addToolBar(tools_tool_bar_);

  /////////////////////////////////////
  // Modes: digram / trigrams
  QColor icon_color = palette().color(QPalette::WindowText);
  digram_action_ = new QAction(util::getColoredIcon(
      ":/images/digram_icon.png", icon_color), tr("&Digram"), this);
  digram_action_->setToolTip("Digram Visualisation");
  connect(digram_action_, SIGNAL(triggered()), this,
          SLOT(showDigramVisualisation()));

  trigram_action_ = new QAction(util::getColoredIcon(
      ":/images/trigram_icon.png", icon_color), tr("&Trigram"), this);
  trigram_action_->setToolTip("Trigram Visualisation");
  connect(trigram_action_, SIGNAL(triggered()), this,
          SLOT(showTrigramVisualisation()));

  layered_digram_action_ = new QAction(util::getColoredIcon(
      ":/images/layered_digram_icon.png", icon_color, false),
      tr("&Layered Digram"), this);
  layered_digram_action_->setToolTip("Layered Digram Visualisation");
  connect(layered_digram_action_, SIGNAL(triggered()), this,
          SLOT(showLayeredDigramVisualisation()));

  modes_tool_bar_ = new QToolBar(tr("Modes"));
  modes_tool_bar_->setMovable(false);
  modes_tool_bar_->addAction(digram_action_);
  modes_tool_bar_->addAction(trigram_action_);
  modes_tool_bar_->addAction(layered_digram_action_);
  modes_tool_bar_->setContextMenuPolicy(Qt::PreventContextMenu);
  modes_tool_bar_->addSeparator();
  modes_tool_bar_->setProperty("common", true);
  visualisation_root_->addToolBar(modes_tool_bar_);

  /////////////////////////////////////
  // Selection
  QWidgetAction* selection_widget_action = new QWidgetAction(this);
  selection_label_ = new QLabel(prepareAddressString(0, 0));
  selection_widget_action->setDefaultWidget(selection_label_);
  selection_label_->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  selection_label_->setSizePolicy(
      QSizePolicy::Expanding, QSizePolicy::Minimum);
  selection_label_->setTextInteractionFlags(Qt::TextSelectableByMouse);
  QToolBar* selection_toolbar = new QToolBar;
  selection_toolbar->setMovable(false);
  selection_toolbar->addAction(selection_widget_action);
  selection_toolbar->setContextMenuPolicy(Qt::PreventContextMenu);
  selection_toolbar->setProperty("common", true);
  visualisation_root_->addToolBar(selection_toolbar);

  /////////////////////////////////////
  // Sampling
  QAction* show_more_options_action = new QAction(QIcon(":/images/more.png"),
      tr("More options"), this);
  connect(show_more_options_action, &QAction::triggered,
      this, &VisualisationPanel::showMoreOptions);
  selection_toolbar->addAction(show_more_options_action);

  /////////////////////////////////////
  // Additional toolbars specific for current visualisation
  prepareVisualisationOptions();
}

}  // namespace visualisation
}  // namespace veles
