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
#ifndef VELES_VISUALISATION_MINIMAP_PANEL_H
#define VELES_VISUALISATION_MINIMAP_PANEL_H

#include <QBoxLayout>
#include <QPair>
#include <QPushButton>
#include <QSpacerItem>
#include <QVector>

#include "util/sampling/isampler.h"
#include "visualisation/minimap.h"
#include "visualisation/selectrangedialog.h"

namespace veles {
namespace visualisation {

class MinimapPanel : public QWidget {
  Q_OBJECT

 public:
  explicit MinimapPanel(QWidget *parent = 0);
  ~MinimapPanel();

  void setSampler(util::ISampler *sampler);
  QPair<size_t, size_t> getSelection();

 signals:
  void selectionChanged(size_t start, size_t end);

 private slots:
  void addMinimap();
  void removeMinimap();
  void changeMinimapMode();
  void updateSelection(int minimap_index, size_t start, size_t end);
  void showSelectRangeDialog();
  void selectRange();

 private:
  void initLayout();
  VisualisationMinimap::MinimapColor getMinimapColor();

  util::ISampler *sampler_;
  QVector<util::ISampler*> minimap_samplers_;
  QVector<VisualisationMinimap*> minimaps_;
  QVector<QSpacerItem*> minimap_spacers_;

  QPair<size_t, size_t> selection_;

  VisualisationMinimap::MinimapMode mode_;

  QBoxLayout *layout_, *minimaps_layout_;
  QPushButton *add_minimap_button_, *remove_minimap_button_;
  QPushButton *change_mode_button_, *select_range_button_;

  SelectRangeDialog *select_range_dialog_;
};

}  //  namespace visualisation
}  //  namespace veles

#endif // VELES_VISUALISATION_MINIMAP_PANEL_H
