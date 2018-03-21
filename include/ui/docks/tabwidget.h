#pragma once

#include <QtWidgets/QTabWidget>
#include "tabbar.h"

namespace veles {
namespace ui {

class TabWidget : public QTabWidget {

  Q_OBJECT

 public:
  explicit TabWidget(QWidget *parent = nullptr);
  std::vector<std::tuple<QWidget *, QIcon, QString>> tabchildren() const;

 signals:
  void emptied();
 private:
  TabBar * tabBar_;

};

}
}