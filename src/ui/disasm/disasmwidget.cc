#include "ui/disasm/disasmwidget.h"

static QString lorem_ipsum =
    "Lorem ipsum dolor sit amet, solet abhorreant qui ex, mea tantas officiis "
    "intellegam te, eu mea harum volutpat percipitur. Moderatius dissentiet "
    "sed ex, qui illum evertitur ut. Habeo possim accusamus ea mea. Est no "
    "brute inermis referrentur. Utroque ponderum cu duo, vel volumus mediocrem "
    "id. Ut eos dolores efficiantur, vim eu nemore viderer torquatos.\n"
    "\n"
    "Ex cum duis lucilius. Mucius facete comprehensam eu mea. Laudem legendos "
    "nam in. Tantas persius nominati vis ea, tollit postea docendi vim te. Ut "
    "hinc integre voluptaria eam, magna iisque constituam ea vel. Nam iuvaret "
    "scribentur ad, mutat summo torquatos vix ne, per adhuc molestie ei.\n"
    "\n"
    "Putent perpetua cotidieque eu vis, saepe verear nostrud vis ex, veritus "
    "omittam mnesarchum ut sit. Ad graeci scaevola recusabo nam, scribentur "
    "reformidans ullamcorper no pro, ad mel liber ceteros. Ullum dictas "
    "vivendo sed ut. Mea no deterruisset interpretaris, qui prima detraxit "
    "sadipscing ei. Diam homero aperiri ea mei.\n"
    "\n"
    "Ei his dictas veritus, an partem graeci sea, solet iudico fabellas te "
    "sit. Ut ferri velit dicta per, duo ea pertinax signiferumque. No cum "
    "dictas legendos omittantur. Putant deserunt assueverit has in, nam error "
    "regione constituam eu. Labore delicatissimi eos in, at usu lorem ancillae "
    "disputationi, partem elaboraret at sed.\n"
    "\n"
    "In mea facer partem, vel bonorum singulis disputando ne. Pro erant "
    "senserit consulatu id. Quot dicunt oportere usu ea. Quo falli euismod "
    "legendos in, est causae scribentur no, duo minim offendit reprehendunt "
    "at. No agam eius consectetuer mel, ad sea oratio dicant scriptorem.";

DisasmWidget::DisasmWidget() : main_layout_(this), debug_output_widget_(this) {
  setDebugText(lorem_ipsum);
  main_layout_.setSizeConstraint(QLayout::SizeConstraint::SetMaximumSize);

  main_layout_.addWidget(&debug_output_widget_);
  setLayout(&main_layout_);
  setWidget(&debug_output_widget_);
}

DisasmWidget::~DisasmWidget() {}

void DisasmWidget::setDebugText(QString text) {
  debug_output_widget_.setText(lorem_ipsum);
}
