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
#pragma once

#include <QWidget>
#include <QStringList>
#include <QToolBar>
#include <QToolButton>
#include <QMainWindow>
#include <QSharedPointer>
#include <QTimer>

#include "util/sampling/uniform_sampler.h"

#include "visualization/base.h"
#include "visualization/minimap_panel.h"

#include "ui/fileblobmodel.h"
#include "ui/hexedit.h"
#include "ui/searchdialog.h"
#include "ui/dockwidget.h"
#include "ui/hexeditwidget.h"
#include "ui/nodetreewidget.h"

namespace veles {
namespace ui {

class NodeWidget : public View {
  Q_OBJECT

 public:
  explicit NodeWidget(MainWindowWithDetachableDockWidgets *main_window,
      QSharedPointer<FileBlobModel>& data_model,
      QSharedPointer<QItemSelectionModel>& selection_model);
  virtual ~NodeWidget();

 public slots:
  void loadBinDataToMinimap();

 private slots:
  void updateMinimap();
  void showMinimap(bool show);

 private:
  MainWindowWithDetachableDockWidgets *main_window_;

  HexEditWidget* hex_edit_widget_;
  visualization::MinimapPanel* minimap_;

  QPointer<NodeTreeWidget> node_tree_widget_;
  QPointer<QDockWidget> node_tree_dock_;
  QPointer<QDockWidget> minimap_dock_;

  QSharedPointer<FileBlobModel> data_model_;
  QSharedPointer<QItemSelectionModel> selection_model_;

  util::UniformSampler* sampler_;
  QByteArray sampler_data_;

  // don't change minimap too often when scrolling by "normal" hex edit scroll bar
  QTimer update_minimap_timer_;
  qint64 update_minimap_start_;
  qint64 update_minimap_size_;
  bool ignore_update_minimap_;
};

}  // namespace ui
}  // namespace veles
