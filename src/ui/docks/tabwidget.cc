
#include "ui/docks/tabwidget.h"

namespace veles {
namespace ui {

TabWidget::TabWidget(QWidget *parent) : QTabWidget(parent) {
  tabBar_ = new TabBar();
  setTabBar(tabBar_);
  setTabsClosable(true);
  connect(this, &TabWidget::tabCloseRequested, this, [this](int index){
    this -> removeTab(index);
    if (this -> count() == 0)
      emit emptied();
  } );
}

std::vector<std::tuple<QWidget *, QIcon, QString>> TabWidget::tabchildren() const {
  auto res = std::vector<std::tuple<QWidget *, QIcon, QString>>();
  for (int i = 0; i < count(); ++i)
    res.push_back(std::make_tuple(widget(i), tabIcon(i), tabText(i)));
  return res;
}

void TabWidget::deleteTab(int index) {
  auto* childWidget = widget(index);
  QTabWidget::removeTab(index);
  delete childWidget;
}

void TabWidget::tabRemoved(int index) {
  if (count() == 0)
    emit emptied();
}



}}