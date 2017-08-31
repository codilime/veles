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

#include <QAction>
#include <QBoxLayout>
#include <QLabel>
#include <QSplitter>
#include <QString>
#include <QToolBar>
#include <QWidget>
#include <QWidgetAction>

#include <map>

#include "ui/dockwidget.h"
#include "ui/fileblobmodel.h"
#include "ui/nodetreewidget.h"
#include "visualization/base.h"
#include "visualization/minimap_panel.h"
#include "visualization/samplingmethoddialog.h"

namespace veles {
namespace visualization {

class VisualizationPanel : public ui::View {
  Q_OBJECT

 public:
  explicit VisualizationPanel(
      ui::MainWindowWithDetachableDockWidgets* main_window,
      const QSharedPointer<ui::FileBlobModel>& data_model,
      QWidget* parent = nullptr);
  ~VisualizationPanel() override;

  void setData(const QByteArray& data);
  void setRange(size_t start, size_t end);
  bool eventFilter(QObject* watched, QEvent* event) override;

 public slots:
  void visibilityChanged(bool visibility);

 private slots:
  void setSamplingMethod(const QString& name);
  void setSampleSize(qint64 kilobytes);
  void showDigramVisualization();
  void showTrigramVisualization();
  void showLayeredDigramVisualization();
  void minimapSelectionChanged(size_t start, size_t end);
  void showMoreOptions();

 private:
  enum class ESampler { NO_SAMPLER, UNIFORM_SAMPLER };
  enum class EVisualization { DIGRAM, TRIGRAM, LAYERED_DIGRAM };

  static const std::map<QString, ESampler> k_sampler_map;
  static const ESampler k_default_sampler = ESampler::UNIFORM_SAMPLER;
  static const EVisualization k_default_visualization = EVisualization::TRIGRAM;
  static const int k_max_sample_size = 128 * 1024;
  static const int k_minimap_sample_size = 4096;

  static util::ISampler* getSampler(ESampler type, const QByteArray& data,
                                    qint64 sample_size);
  VisualizationWidget* getVisualization(EVisualization type,
                                        QWidget* parent = nullptr);
  static QString prepareAddressString(size_t start, size_t end);

  void setVisualization(EVisualization type);
  void refreshVisualization();
  void initLayout();
  void initOptionsPanel();
  void prepareVisualizationOptions();

  QByteArray data_;
  ESampler sampler_type_;
  EVisualization visualization_type_;
  int sample_size_;
  util::ISampler *sampler_, *minimap_sampler_;
  MinimapPanel* minimap_;
  VisualizationWidget* visualization_;
  QMainWindow* visualization_root_;

  QSpinBox* sample_size_box_;
  QBoxLayout *layout_, *options_layout_;
  QWidget* child_options_wrapper_;
  QAction *digram_action_, *trigram_action_, *layered_digram_action_;
  QLabel* selection_label_;

  QToolBar* tools_tool_bar_;
  QAction* show_node_tree_act_;
  QAction* show_minimap_act_;
  QToolBar* modes_tool_bar_;

  QPointer<QDockWidget> node_tree_dock_;
  QPointer<QDockWidget> minimap_dock_;

  ui::NodeTreeWidget* node_tree_widget_;
  QSharedPointer<ui::FileBlobModel> data_model_;

  SamplingMethodDialog* sampling_method_dialog_;
  ui::MainWindowWithDetachableDockWidgets* main_window_;

  bool visible_;
};

}  //  namespace visualization
}  //  namespace veles
