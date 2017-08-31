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
#include <QFont>
#include <QSettings>
#include <QStyleFactory>

#include "util/settings/theme.h"

namespace veles {
namespace util {
namespace settings {
namespace theme {

static bool isDark_ = false;
static bool isDarkCached_ = false;

static bool isDark();
static QColor colorInvertedIfDark(const QColor& color);

static QVector<QColor> chunkBackgroundColors_ = {
    QColor("#FFEB3B"), QColor("#FFAB91"), QColor("#FFC107"),
    QColor("#81C784"), QColor("#B2FF59"), QColor("#A7FFEB")};

QString currentTheme() {
  QSettings settings;
  return settings.value("theme", "normal").toString();
}

void setCurrentTheme(const QString& theme) {
  QSettings settings;
  settings.setValue("theme", theme);
}

QStringList availableThemes() { return {"normal", "dark"}; }

QPalette pallete() {
  QPalette pallete;
  if (isDark()) {
    pallete.setColor(QPalette::Window, QColor(53, 53, 53));
    pallete.setColor(QPalette::WindowText, Qt::white);
    pallete.setColor(QPalette::Base, QColor(25, 25, 25));
    pallete.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    pallete.setColor(QPalette::ToolTipBase, QColor(255, 255, 225));
    pallete.setColor(QPalette::ToolTipText, Qt::black);
    pallete.setColor(QPalette::Text, Qt::white);
    pallete.setColor(QPalette::Button, QColor(53, 53, 53));
    pallete.setColor(QPalette::Light, QColor(25, 25, 25));
    pallete.setColor(QPalette::ButtonText, Qt::white);
    pallete.setColor(QPalette::BrightText, Qt::red);
    pallete.setColor(QPalette::Link, QColor(42, 130, 218));
    pallete.setColor(QPalette::Highlight, QColor(42, 130, 218));
    pallete.setColor(QPalette::HighlightedText, Qt::black);

    pallete.setColor(QPalette::Disabled, QPalette::Text, Qt::gray);
    pallete.setColor(QPalette::Disabled, QPalette::ButtonText, Qt::gray);
  }
  return pallete;
}

QStyle* createStyle() {
  if (currentTheme() == "dark") {
    return QStyleFactory::create("Fusion");
  }
  return nullptr;
}

QColor highlightingColor() {
  return colorInvertedIfDark(QColor(0xff, 0xff, 0x99, 0xff));
}

QColor chunkBackground(int color_index) {
  if (color_index < 0) {
    return {"#FFFFFF"};
  }
  QColor ret =
      chunkBackgroundColors_[color_index % chunkBackgroundColors_.size()];
  if (isDark()) {
    ret = QColor(ret.red() ^ 0xff, ret.green() ^ 0xff, ret.blue() ^ 0xff);
  }

  return ret;
}

QColor byteColor(uint8_t byte) {
  if (isDark()) {
    byte ^= 0xFF;
  }

  int red, blue, green = 0;
  if (byte <= 0x80) {
    blue = 0xff;
    red = qMin(0xff, byte * 2);
  } else {
    blue = qMin(0xff, (0xff - byte) * 2);
    red = 0xff;
  }

  return colorInvertedIfDark(QColor(red, green, blue));
}

QFont font() {
#ifdef Q_OS_WIN32
  return QFont("Courier", 10);
#else
  return QFont("Monospace", 10);
#endif
}

static bool isDark() {
  if (!isDarkCached_) {
    isDark_ = currentTheme() == "dark";
    isDarkCached_ = true;
  }
  return isDark_;
}

static QColor colorInvertedIfDark(const QColor& color) {
  if (isDark()) {
    return {color.red() ^ 0xFF, color.green() ^ 0xFF, color.blue() ^ 0xFF};
  }
  return color;
}

QColor editedBackground() { return colorInvertedIfDark(QColor("#bc7874")); }

}  // namespace theme
}  // namespace settings
}  // namespace util
}  // namespace veles
