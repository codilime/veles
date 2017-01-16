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
#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include "db/db.h"
#include "dbif/info.h"
#include "dbif/method.h"
#include "dbif/promise.h"
#include "dbif/error.h"
#include "dbif/universe.h"

class Aaa : public QObject {
  Q_OBJECT

public slots:
  void f(veles::dbif::PInfoReply rep) {
    auto crep = rep.dynamicCast<veles::dbif::ChildrenReply>();
    printf("List, %zu elements.\n", crep->objects.size());
  }
  void g(veles::dbif::PInfoReply rep) {
    auto drep = rep.dynamicCast<veles::dbif::DescriptionReply>();
    qDebug() << "Name: " << drep->name;
    qDebug() << "Comment: " << drep->comment;
  }
};

#include "dbif_test.moc"

int main(int argc, char **argv) {
  QCoreApplication app(argc, argv);
  veles::dbif::ObjectHandle obj = veles::db::create_db();
  veles::data::BinData vec(8, { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 });
  auto rep = obj->syncRunMethod<veles::dbif::RootCreateFileBlobFromDataRequest>(vec, "abc");
  auto blob = rep->object;
  auto drep = blob->syncGetInfo<veles::dbif::DescriptionRequest>();
  qDebug() << "Name: " << drep->name;
  qDebug() << "Comment: " << drep->comment;
  blob->syncRunMethod<veles::dbif::SetCommentRequest>("defghi");
  drep = blob->syncGetInfo<veles::dbif::DescriptionRequest>();
  qDebug() << "Name: " << drep->name;
  qDebug() << "Comment: " << drep->comment;
  auto data = blob->syncGetInfo<veles::dbif::BlobDataRequest>(2, 5)->data;
  qDebug() << "Data: " << QByteArray(reinterpret_cast<const char*>(data.rawData()), data.size());
  data = blob->syncGetInfo<veles::dbif::BlobDataRequest>(7, 11)->data;
  qDebug() << "Data: " << QByteArray(reinterpret_cast<const char*>(data.rawData()), data.size());
  return 0;
}
