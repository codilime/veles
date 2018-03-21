
#include "ui/docks/tabwidget.h"

namespace veles {
namespace ui {

TabWidget::TabWidget(QWidget *parent) : QTabWidget(parent) {
  tabBar_ = new TabBar();
  setTabBar(tabBar_);
  setTabsClosable(true);
  connect(this, &TabWidget::tabCloseRequested, [this](int index){this -> removeTab(index);} );
}

std::vector<std::tuple<QWidget *, QIcon, QString>> TabWidget::tabchildren() const {
  auto res = std::vector<std::tuple<QWidget *, QIcon, QString>>();
  for (int i = 0; i < count(); ++i)
    res.push_back(std::make_tuple(widget(i), tabIcon(i), tabText(i)));
  return res;
}

}}