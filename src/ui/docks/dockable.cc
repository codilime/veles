#include "ui/docks/dockable.h"

namespace veles {
namespace ui {

Dockable::Dockable(QWidget *widget, QIcon &icon, QString &label) : widget(widget), icon(icon), title(label) {}
Dockable::Dockable(QWidget *widget, QString &label): widget(widget), title(label) {}

}
}