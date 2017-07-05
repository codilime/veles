/*
 * Copyright 2017 CodiLime
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
#include <QAction>
#include <QColorDialog>
#include <QFileDialog>
#include <QGroupBox>
#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardItemModel>
#include <QToolBar>
#include <QTreeView>
#include <QVBoxLayout>

#include "dbif/info.h"
#include "dbif/types.h"
#include "dbif/universe.h"

#include "ui/hexeditwidget.h"
#include "ui/nodetreewidget.h"
#include "ui/nodewidget.h"
#include "ui/veles_mainwindow.h"

#include "util/settings/hexedit.h"
#include "util/icons.h"

#include "visualization/panel.h"

namespace veles {
namespace ui {

const float k_minimap_selection_factor = 10;

/*****************************************************************************/
/* Public methods */
/*****************************************************************************/

NodeWidget::NodeWidget(MainWindowWithDetachableDockWidgets *main_window,
    QSharedPointer<FileBlobModel>& data_model,
    QSharedPointer<QItemSelectionModel>& selection_model)
    : View("Hex editor", ":/images/show_hex_edit.png"),
      main_window_(main_window), minimap_(nullptr),
      minimap_dock_(nullptr), data_model_(data_model),
      selection_model_(selection_model), sampler_(nullptr),
      update_minimap_start_(0), update_minimap_size_(0),
      ignore_update_minimap_(false) {
  hex_edit_widget_ = new HexEditWidget(
      main_window, data_model, selection_model);
  addAction(hex_edit_widget_->uploadAction());
  addAction(hex_edit_widget_->undoAction());
  addAction(hex_edit_widget_->findAction());
  addAction(hex_edit_widget_->findNextAction());
  addAction(hex_edit_widget_->showVisualizationAction());
  addAction(hex_edit_widget_->showHexEditAction());
  addAction(hex_edit_widget_->showNodeTreeAction());
  setCentralWidget(hex_edit_widget_);
  setFocusProxy(hex_edit_widget_);

  node_tree_dock_ = new QDockWidget;
  new DockWidgetVisibilityGuard(node_tree_dock_);
  node_tree_dock_->setWindowTitle("Node tree");
  node_tree_widget_ = new NodeTreeWidget(main_window, data_model,
      selection_model);
  node_tree_dock_->setWidget(node_tree_widget_);
  node_tree_dock_->setContextMenuPolicy(Qt::PreventContextMenu);
  node_tree_dock_->setAllowedAreas(
      Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
  setDockNestingEnabled(true);
  addDockWidget(Qt::LeftDockWidgetArea, node_tree_dock_);

  connect(hex_edit_widget_, &HexEditWidget::showNodeTree,
      node_tree_dock_, &QDockWidget::setVisible);
  connect(node_tree_dock_, &QDockWidget::visibilityChanged,
        hex_edit_widget_, &HexEditWidget::nodeTreeVisibilityChanged);

  connect(hex_edit_widget_, &HexEditWidget::showMinimap, this, &NodeWidget::showMinimap);
  connect(hex_edit_widget_, &HexEditWidget::updateMinimap, [this](qint64 start, qint64 size) {
    if (!ignore_update_minimap_) {
      update_minimap_start_ = start;
      update_minimap_size_ = size;
      update_minimap_timer_.start(400);
    }
  });

  update_minimap_timer_.setSingleShot(true);
  connect(&update_minimap_timer_, &QTimer::timeout, this, &NodeWidget::updateMinimap);
}

NodeWidget::~NodeWidget() {
  delete sampler_;
}

void NodeWidget::showMinimap(bool show) {
  if (minimap_dock_ == nullptr) {
    if (!show) {
      return;
    }
    minimap_dock_ = new QDockWidget;
    new DockWidgetVisibilityGuard(minimap_dock_);
    minimap_dock_->setWindowTitle("Minimap");
    minimap_ = new visualization::MinimapPanel(this, /*size_control=*/false);

    if (data_model_->binData().size() > 0) {
      loadBinDataToMinimap();
    } else {
      sampler_data_ = QByteArray("");
      sampler_ = new util::UniformSampler(sampler_data_);
      sampler_->setSampleSize(4096 * 1024);
      minimap_->setSampler(sampler_);
    }

    minimap_dock_->setWidget(minimap_);
    minimap_dock_->setContextMenuPolicy(Qt::PreventContextMenu);
    minimap_dock_->setAllowedAreas(
        Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, minimap_dock_);
    MainWindowWithDetachableDockWidgets::splitDockWidget2(this, node_tree_dock_,
        minimap_dock_, Qt::Horizontal);

    connect(data_model_.data(), &FileBlobModel::newBinData,
        this, &NodeWidget::loadBinDataToMinimap);
    connect(minimap_dock_, &QDockWidget::visibilityChanged,
        hex_edit_widget_, &HexEditWidget::minimapVisibilityChanged);
    connect(minimap_, &visualization::MinimapPanel::selectionChanged, [this](size_t start, size_t end) {
      if (!ignore_update_minimap_) {
        ignore_update_minimap_ = true;
        hex_edit_widget_->minimapSelectionChanged(start, end);
        ignore_update_minimap_ = false;
      }
    });
  }
  minimap_dock_->setVisible(show);

}

void NodeWidget::updateMinimap() {
  if (minimap_dock_ == nullptr || sampler_->empty()) {
    return;
  }
  ignore_update_minimap_ = true;
  minimap_->adjustMinimaps(update_minimap_size_, k_minimap_selection_factor, update_minimap_start_);
  ignore_update_minimap_ = false;
}

void NodeWidget::loadBinDataToMinimap() {
  delete sampler_;

  sampler_data_ = QByteArray((const char *)data_model_->binData().rawData(),
          static_cast<int>(data_model_->binData().octets()));
  sampler_ = new util::UniformSampler(sampler_data_);
  sampler_->setSampleSize(4096 * 1024);
  minimap_->setSampler(sampler_);
  updateMinimap();
}

}  // namespace ui
}  // namespace veles
