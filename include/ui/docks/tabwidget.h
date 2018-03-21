#pragma once

#include <QtWidgets/QTabWidget>
#include "tabbar.h"

namespace veles {
namespace ui {

class TabWidget : public QTabWidget {

 public:
  explicit TabWidget(QWidget *parent = nullptr);
  std::vector<std::tuple<QWidget *, QIcon, QString>> tabchildren() const;

 private:
  TabBar * tabBar_;

};

}
}