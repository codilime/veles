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
#include <iostream>

#include <QApplication>
#include <QSurfaceFormat>
#include <QTranslator>
#include <QHostAddress>

#include "ui/dockwidget.h"
#include "ui/veles_mainwindow.h"
#include "visualisation/base.h"
#include "visualisation/digram.h"
#include "visualisation/trigram.h"
#include "util/settings/network.h"
#include "util/settings/theme.h"
#include "util/concurrency/threadpool.h"
#include "util/version.h"

int main(int argc, char *argv[]) {
  Q_INIT_RESOURCE(veles);

  QSurfaceFormat format;
  format.setDepthBufferSize(24);
  format.setVersion(3, 3);
  format.setProfile(QSurfaceFormat::CoreProfile);
  QSurfaceFormat::setDefaultFormat(format);

  QApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

  QApplication app(argc, argv);
  app.setApplicationName("Veles");
  app.setOrganizationName("Codisec");
  app.setApplicationVersion(veles::util::version::string);

  app.setStyle(veles::util::settings::theme::createStyle());
  app.setPalette(veles::util::settings::theme::pallete());

  // Identify locale and load translation if available
  QString locale = QLocale::system().name();
  QTranslator translator;
  translator.load(QString("hexedit_") + locale);
  app.installTranslator(&translator);

  veles::util::threadpool::createTopic("visualisation", 3);

  qRegisterMetaType<veles::visualisation::VisualisationWidget::AdditionalResampleDataPtr>("AdditionalResampleDataPtr");

  QCommandLineParser parser;
  parser.addHelpOption();
  parser.addVersionOption();
  parser.addOptions({
      {"network", "Enable network server\n"
       "Exclusive with no-network\n"
       "Value specified will be persistent"},
      {"no-network", "Disable  network server\n"
       "Exclusive with network\n"
       "Value specified will be persistent"},
      {"ip", "IP address the server will listen on.\n"
       "Value specified will be persistent.", "ip"},
      {{"p", "port"}, "Port the server will listen on.\n"
       "Value specified will be persistent.", "port"}
  });
  parser.process(app);

  if (parser.isSet("network") || parser.isSet("no-network")) {
    bool networkEnabled = parser.isSet("network") && !parser.isSet("no-network");
    veles::util::settings::network::setEnabled(networkEnabled);
  }

  if (parser.isSet("port")) {
    QString port = parser.value("port");
    bool ok;
    uint32_t port_val = port.toUInt(&ok);
    if (ok && 1 <= port_val && port_val <= 65535) {
      veles::util::settings::network::setPort(port_val);
    } else {
      std::cerr << "Bad port value provided - ignoring." << std::endl;
    }
  }
  if (parser.isSet("ip")) {
    QHostAddress addr;
    if (addr.setAddress(parser.value("ip"))) {
      veles::util::settings::network::setIpAddress(parser.value("ip"));
    } else {
      std::cerr << "Bad ip value provided - ignoring." << std::endl;
    }
  }

  veles::ui::VelesMainWindow *mainWin = new veles::ui::VelesMainWindow;
  mainWin->showMaximized();

  auto files = parser.positionalArguments();
  for (auto file : files) {
    mainWin->addFile(file);
  }

  return app.exec();
}
