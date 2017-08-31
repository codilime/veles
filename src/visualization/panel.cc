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
#include <QHBoxLayout>
#include <QLabel>
#include <QLayoutItem>
#include <QVBoxLayout>

#include "util/icons.h"
#include "util/sampling/fake_sampler.h"
#include "util/sampling/uniform_sampler.h"
#include "util/settings/shortcuts.h"
#include "visualization/digram.h"
#include "visualization/panel.h"
#include "visualization/trigram.h"

namespace veles {
namespace visualization {

using util::settings::shortcuts::ShortcutsModel;

const std::map<QString, VisualizationPanel::ESampler>
    VisualizationPanel::k_sampler_map = {
        {"No sampling", VisualizationPanel::ESampler::NO_SAMPLER},
        {"Uniform random sampling",
         VisualizationPanel::ESampler::UNIFORM_SAMPLER}};

/*****************************************************************************/
/* Public methods */
/*****************************************************************************/

VisualizationPanel::VisualizationPanel(
    ui::MainWindowWithDetachableDockWidgets* main_window,
    const QSharedPointer<ui::FileBlobModel>& data_model, QWidget* parent)
    : veles::ui::View("Visualization", ":/images/trigram_icon.png"),
      sampler_type_(k_default_sampler),
      visualization_type_(k_default_visualization),
      sample_size_(1024),
      data_model_(data_model),
      main_window_(main_window),
      visible_(true) {
  sampler_ = getSampler(sampler_type_, data_, sample_size_);
  sampler_->allowAsynchronousResampling(true);
  minimap_sampler_ =
      getSampler(ESampler::UNIFORM_SAMPLER, data_, k_minimap_sample_size);
  minimap_ = new MinimapPanel(this);
  minimap_->setSampler(minimap_sampler_);
  connect(minimap_, SIGNAL(selectionChanged(size_t, size_t)), this,
          SLOT(minimapSelectionChanged(size_t, size_t)));

  visualization_ = getVisualization(visualization_type_, this);
  visualization_root_ = new QMainWindow;
  visualization_root_->setCentralWidget(visualization_);
  setFocusProxy(visualization_);

  sampling_method_dialog_ = new SamplingMethodDialog(this);
  sampling_method_dialog_->setSampleSize(sample_size_);
  sampling_method_dialog_->setMaximumSampleSize(k_max_sample_size);

  connect(sampling_method_dialog_,
          SIGNAL(samplingMethodChanged(const QString&)), this,
          SLOT(setSamplingMethod(const QString&)));
  connect(sampling_method_dialog_, SIGNAL(sampleSizeChanged(int)), this,
          SLOT(setSampleSize(int)));

  initLayout();
}

VisualizationPanel::~VisualizationPanel() {
  delete visualization_;
  delete minimap_;
  delete sampler_;
  delete minimap_sampler_;
}

void VisualizationPanel::setData(const QByteArray& data) {
  delete sampler_;
  delete minimap_sampler_;
  data_ = data;
  sampler_ = getSampler(sampler_type_, data_, sample_size_);
  sampler_->allowAsynchronousResampling(true);
  minimap_sampler_ =
      getSampler(ESampler::UNIFORM_SAMPLER, data_, k_minimap_sample_size);
  minimap_->setSampler(minimap_sampler_);
  visualization_->setSampler(sampler_);
  selection_label_->setText(prepareAddressString(
      0, sampler_->getFileOffset(sampler_->getSampleSize())));
}

void VisualizationPanel::setRange(size_t start, size_t end) {
  sampler_->setRange(start, end);
}

bool VisualizationPanel::eventFilter(QObject* /*watched*/, QEvent* event) {
  // filter out timer events for not visible visualisation so that we don't
  // waste resources on rotating something that isn't visible.
  return !visible_ && event->type() == QEvent::Timer;
}

void VisualizationPanel::visibilityChanged(bool visibility) {
  visible_ = visibility;
}

/*****************************************************************************/
/* Static factory methods */
/*****************************************************************************/

util::ISampler* VisualizationPanel::getSampler(ESampler type,
                                               const QByteArray& data,
                                               qint64 sample_size) {
  switch (type) {
    case ESampler::NO_SAMPLER:
      return new util::FakeSampler(data);
    case ESampler::UNIFORM_SAMPLER:
      auto* sampler = new util::UniformSampler(data);
      sampler->setSampleSize(1024 * sample_size);
      return sampler;
  }
  return nullptr;
}

VisualizationWidget* VisualizationPanel::getVisualization(EVisualization type,
                                                          QWidget* parent) {
  TrigramWidget* trigram = nullptr;
  switch (type) {
    case EVisualization::DIGRAM:
      return new DigramWidget(parent);
    case EVisualization::TRIGRAM:
      trigram = new TrigramWidget(parent);
      trigram->setMode(TrigramWidget::EVisualizationMode::TRIGRAM);
      break;
    case EVisualization::LAYERED_DIGRAM:
      trigram = new TrigramWidget(parent);
      trigram->setMode(TrigramWidget::EVisualizationMode::LAYERED_DIGRAM,
                       false);
      break;
  }
  if (trigram != nullptr) {
    trigram->installEventFilter(this);
    return trigram;
  }
  return nullptr;
}

QString VisualizationPanel::prepareAddressString(size_t start, size_t end) {
  auto label = QString("0x%1 : ").arg(start, 8, 16, QChar('0'));
  label.append(QString("0x%1 ").arg(end, 8, 16, QChar('0')));
  label.append(QString("(%1 bytes)").arg(end - start));
  return label;
}

/*****************************************************************************/
/* Private slots */
/*****************************************************************************/

void VisualizationPanel::setSamplingMethod(const QString& name) {
  auto new_sampler_type = k_sampler_map.at(name);
  if (new_sampler_type == sampler_type_) {
    return;
  }

  auto old_sampler = sampler_;
  sampler_ = getSampler(new_sampler_type, data_, sample_size_);
  sampler_->allowAsynchronousResampling(true);
  auto selection = minimap_->getSelection();
  sampler_->setRange(selection.first, selection.second);
  visualization_->setSampler(sampler_);
  delete old_sampler;
  sampler_type_ = new_sampler_type;
}

void VisualizationPanel::setSampleSize(qint64 kilobytes) {
  sample_size_ = kilobytes;
  if (sampler_type_ == ESampler::UNIFORM_SAMPLER) {
    sampler_->setSampleSize(1024 * kilobytes);
  }
}

void VisualizationPanel::showDigramVisualization() {
  setVisualization(EVisualization::DIGRAM);
}

void VisualizationPanel::showTrigramVisualization() {
  if (visualization_type_ == EVisualization::LAYERED_DIGRAM) {
    visualization_type_ = EVisualization::TRIGRAM;
    auto trigram = static_cast<TrigramWidget*>(visualization_);
    trigram->setMode(TrigramWidget::EVisualizationMode::TRIGRAM);
  } else {
    setVisualization(EVisualization::TRIGRAM);
  }
}

void VisualizationPanel::showLayeredDigramVisualization() {
  if (visualization_type_ == EVisualization::TRIGRAM) {
    visualization_type_ = EVisualization::LAYERED_DIGRAM;
    auto trigram = static_cast<TrigramWidget*>(visualization_);
    trigram->setMode(TrigramWidget::EVisualizationMode::LAYERED_DIGRAM);
  } else {
    setVisualization(EVisualization::LAYERED_DIGRAM);
  }
}

void VisualizationPanel::minimapSelectionChanged(size_t start, size_t end) {
  selection_label_->setText(prepareAddressString(start, end));
  sampler_->setRange(start, end);
}

void VisualizationPanel::showMoreOptions() { sampling_method_dialog_->show(); }

/*****************************************************************************/
/* Private methods */
/*****************************************************************************/

void VisualizationPanel::setVisualization(EVisualization type) {
  if (type != visualization_type_) {
    auto toolbars = visualization_root_->findChildren<QToolBar*>();
    for (auto toolbar : toolbars) {
      if (toolbar->actions().empty() || !toolbar->property("common").toBool()) {
        toolbar->deleteLater();
      }
    }
    VisualizationWidget* old = visualization_;
    visualization_type_ = type;
    visualization_ = getVisualization(visualization_type_, this);
    visualization_->setSampler(sampler_);
    visualization_root_->setCentralWidget(visualization_);
    prepareVisualizationOptions();
    visualization_root_->setFocus();
    delete old;
  }
}

void VisualizationPanel::refreshVisualization() {
  if (visualization_ != nullptr) {
    visualization_->refreshVisualization();
  }
}

void VisualizationPanel::initLayout() {
  initOptionsPanel();

  setCentralWidget(visualization_root_);
  setDockNestingEnabled(true);

  node_tree_dock_ = new QDockWidget;
  node_tree_dock_->setWindowTitle("Node tree");
  QSharedPointer<QItemSelectionModel> new_selection_model(
      new QItemSelectionModel(data_model_.data()));
  node_tree_widget_ =
      new ui::NodeTreeWidget(main_window_, data_model_, new_selection_model);
  node_tree_dock_->setWidget(node_tree_widget_);
  node_tree_dock_->setContextMenuPolicy(Qt::PreventContextMenu);
  node_tree_dock_->setAllowedAreas(Qt::LeftDockWidgetArea |
                                   Qt::RightDockWidgetArea);
  node_tree_dock_->hide();
  addDockWidget(Qt::LeftDockWidgetArea, node_tree_dock_);

  minimap_dock_ = new QDockWidget;
  minimap_dock_->setWindowTitle("Minimap");
  minimap_dock_->setWidget(minimap_);
  minimap_dock_->setContextMenuPolicy(Qt::PreventContextMenu);
  minimap_dock_->setAllowedAreas(Qt::LeftDockWidgetArea |
                                 Qt::RightDockWidgetArea);
  tabifyDockWidget(node_tree_dock_, minimap_dock_);
  ui::MainWindowWithDetachableDockWidgets::splitDockWidget2(
      this, node_tree_dock_, minimap_dock_, Qt::Horizontal);

  //  connect(show_node_tree_act_, &QAction::toggled,
  //      node_tree_dock_, &QDockWidget::setVisible);
  connect(show_minimap_act_, &QAction::toggled, minimap_dock_,
          &QDockWidget::setVisible);
}

void VisualizationPanel::prepareVisualizationOptions() {
  visualization_->prepareOptions(visualization_root_);
}

void VisualizationPanel::initOptionsPanel() {
  /////////////////////////////////////
  // Node tree / minimap
  //  show_node_tree_act_ = new QAction(QIcon(":/images/show_node_tree.png"),
  //      tr("&Node tree"), this);
  //  show_node_tree_act_->setToolTip(tr("Node tree"));
  //  show_node_tree_act_->setEnabled(true);
  //  show_node_tree_act_->setCheckable(true);
  //  show_node_tree_act_->setChecked(false);
  show_minimap_act_ = ShortcutsModel::getShortcutsModel()->createQAction(
      util::settings::shortcuts::SHOW_MINIMAP, this,
      QIcon(":/images/show_minimap.png"), Qt::WidgetWithChildrenShortcut);
  show_minimap_act_->setToolTip(tr("Minimap"));
  show_minimap_act_->setEnabled(true);
  show_minimap_act_->setCheckable(true);
  show_minimap_act_->setChecked(true);

  tools_tool_bar_ = new QToolBar(tr("Tools"));
  tools_tool_bar_->setMovable(false);
  addAction(show_minimap_act_);
  // addAction(show_node_tree_act_);
  // tools_tool_bar_->addAction(show_node_tree_act_);
  tools_tool_bar_->addAction(show_minimap_act_);
  tools_tool_bar_->setContextMenuPolicy(Qt::PreventContextMenu);
  tools_tool_bar_->setProperty("common", true);
  tools_tool_bar_->addSeparator();
  visualization_root_->addToolBar(tools_tool_bar_);

  /////////////////////////////////////
  // Modes: digram / trigrams
  QColor icon_color = palette().color(QPalette::WindowText);
  digram_action_ = ShortcutsModel::getShortcutsModel()->createQAction(
      util::settings::shortcuts::VISUALIZATION_DIGRAM, this,
      util::getColoredIcon(":/images/digram_icon.png", icon_color),
      Qt::WidgetWithChildrenShortcut);
  digram_action_->setToolTip("Digram Visualization");
  connect(digram_action_, SIGNAL(triggered()), this,
          SLOT(showDigramVisualization()));

  trigram_action_ = ShortcutsModel::getShortcutsModel()->createQAction(
      util::settings::shortcuts::VISUALIZATION_TRIGRAM, this,
      util::getColoredIcon(":/images/trigram_icon.png", icon_color),
      Qt::WidgetWithChildrenShortcut);
  trigram_action_->setToolTip("Trigram Visualization");
  connect(trigram_action_, SIGNAL(triggered()), this,
          SLOT(showTrigramVisualization()));

  layered_digram_action_ = ShortcutsModel::getShortcutsModel()->createQAction(
      util::settings::shortcuts::VISUALIZATION_LAYERED_DIGRAM, this,
      util::getColoredIcon(":/images/layered_digram_icon.png", icon_color,
                           false),
      Qt::WidgetWithChildrenShortcut);
  layered_digram_action_->setToolTip("Layered Digram Visualization");
  connect(layered_digram_action_, SIGNAL(triggered()), this,
          SLOT(showLayeredDigramVisualization()));

  modes_tool_bar_ = new QToolBar(tr("Modes"));
  modes_tool_bar_->setMovable(false);
  addAction(digram_action_);
  addAction(trigram_action_);
  addAction(layered_digram_action_);
  modes_tool_bar_->addAction(digram_action_);
  modes_tool_bar_->addAction(trigram_action_);
  modes_tool_bar_->addAction(layered_digram_action_);
  modes_tool_bar_->setContextMenuPolicy(Qt::PreventContextMenu);
  modes_tool_bar_->addSeparator();
  modes_tool_bar_->setProperty("common", true);
  visualization_root_->addToolBar(modes_tool_bar_);

  /////////////////////////////////////
  // Selection
  auto* selection_widget_action = new QWidgetAction(this);
  selection_label_ = new QLabel(prepareAddressString(0, 0));
  selection_widget_action->setDefaultWidget(selection_label_);
  selection_label_->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
  selection_label_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
  selection_label_->setTextInteractionFlags(Qt::TextSelectableByMouse);
  auto* selection_toolbar = new QToolBar;
  selection_toolbar->setMovable(false);
  selection_toolbar->addAction(selection_widget_action);
  selection_toolbar->setContextMenuPolicy(Qt::PreventContextMenu);
  selection_toolbar->setProperty("common", true);
  visualization_root_->addToolBar(selection_toolbar);

  /////////////////////////////////////
  // Sampling

  QAction* show_more_options_action =
      ShortcutsModel::getShortcutsModel()->createQAction(
          util::settings::shortcuts::VISUALIZATION_OPTIONS, this,
          QIcon(":/images/more.png"), Qt::WidgetWithChildrenShortcut);
  connect(show_more_options_action, &QAction::triggered, this,
          &VisualizationPanel::showMoreOptions);
  addAction(show_more_options_action);
  selection_toolbar->addAction(show_more_options_action);

  /////////////////////////////////////
  // Additional toolbars specific for current visualization
  prepareVisualizationOptions();

  // TODO(mkow): considering that there is already some code duplication between
  // here and HexEditWidget we should consider refactoring some parts of those
  // classes into common base.
  QAction* open_hex = ShortcutsModel::getShortcutsModel()->createQAction(
      util::settings::shortcuts::OPEN_HEX, this,
      Qt::WidgetWithChildrenShortcut);

  connect(open_hex, &QAction::triggered,
          [this]() { createHexEditor(main_window_, data_model_); });
  addAction(open_hex);

  QAction* open_visualization =
      ShortcutsModel::getShortcutsModel()->createQAction(
          util::settings::shortcuts::OPEN_VISUALIZATION, this,
          Qt::WidgetWithChildrenShortcut);

  connect(open_visualization, &QAction::triggered,
          [this]() { createVisualization(main_window_, data_model_); });
  addAction(open_visualization);
}

}  // namespace visualization
}  // namespace veles
