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
#include "util/settings/theme.h"

#include <QFont>
#include <QFontDatabase>
#include <QSettings>
#include <QStyleFactory>

#include "ui/velesapplication.h"

namespace veles {
namespace util {
namespace settings {
namespace theme {

static const char g_dark_theme_str[] = "dark";

static bool isDark_ = false;
static bool isDarkCached_ = false;

static bool isDark();
static QColor colorInvertedIfDark(const QColor& color);

static QVector<QColor> chunkBackgroundColors_ = {
    QColor("#FFEB3B"), QColor("#FFAB91"), QColor("#FFC107"),
    QColor("#81C784"), QColor("#B2FF59"), QColor("#A7FFEB")};

QStringList availableThemes() { return {"normal", g_dark_theme_str}; }

const QString& defaultTheme() {
  static const QString default_theme{g_dark_theme_str};
  return default_theme;
}

QString theme() {
  QSettings settings;
  return settings.value("theme", defaultTheme()).toString();
}

void setTheme(const QString& theme) {
  QSettings settings;
  settings.setValue("theme", theme);
}

QPalette palette() {
  QPalette palette;
  if (isDark()) {
    palette.setColor(QPalette::Window, QColor(53, 53, 53));
    palette.setColor(QPalette::WindowText, Qt::white);
    palette.setColor(QPalette::Base, QColor(25, 25, 25));
    palette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    palette.setColor(QPalette::ToolTipBase, QColor(255, 255, 225));
    palette.setColor(QPalette::ToolTipText, Qt::black);
    palette.setColor(QPalette::Text, Qt::white);
    palette.setColor(QPalette::Button, QColor(53, 53, 53));
    palette.setColor(QPalette::Light, QColor(25, 25, 25));
    palette.setColor(QPalette::ButtonText, Qt::white);
    palette.setColor(QPalette::BrightText, Qt::red);
    palette.setColor(QPalette::Link, QColor(42, 130, 218));
    palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    palette.setColor(QPalette::HighlightedText, Qt::black);

    palette.setColor(QPalette::Disabled, QPalette::Text, Qt::gray);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, Qt::gray);
  }
  return palette;
}

QStyle* createStyle() {
  if (theme() == g_dark_theme_str) {
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

QFont defaultFont() {
  return QFontDatabase::systemFont(QFontDatabase::GeneralFont);
}

QFont font() {
  QSettings settings;
  QFont res;
  res.fromString(settings.value("font", defaultFont()).toString());
  return res;
}

void setFont(const QFont& font) {
  QSettings settings;
  settings.setValue("font", font.toString());
  veles::ui::VelesApplication::setFont(font);
}

QFont defaultFixedFont() {
#if defined(Q_OS_WIN32)
  return QFont("Courier", 10);
#elif defined(Q_OS_MAC)
  // For some reason font with size=10 looks too small on macOS.
  return QFont("Monaco", 12);
#else
  return QFont("Monospace", 10);
#endif
}

QFont fixedFont() {
  QSettings settings;
  QFont res;
  res.fromString(settings.value("fixedFont", defaultFixedFont()).toString());
  return res;
}

void setFixedFont(const QFont& font) {
  QSettings settings;
  settings.setValue("fixedFont", font.toString());
}

static bool isDark() {
  if (!isDarkCached_) {
    isDark_ = theme() == g_dark_theme_str;
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
