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
#ifndef FILEBLOBITEM_H
#define FILEBLOBITEM_H

#include <QList>
#include <QObject>
#include <QString>
#include <QIcon>
#include "dbif/types.h"

namespace veles {
namespace ui {

class FileBlobItem : public QObject {
  Q_OBJECT

 public:
  virtual ~FileBlobItem() {}
  explicit FileBlobItem(QString name, QString value, QString comment,
                        uint64_t start, uint64_t end, QObject *parent);

  virtual int childrenCount();
  virtual FileBlobItem *child(int index);
  virtual int childIndex(FileBlobItem *child);
  virtual QString name();
  virtual QString comment();
  virtual QString value();
  virtual bool range(uint64_t *start, uint64_t *end) const;
  virtual dbif::ObjectHandle objectHandle();
  virtual bool isRemovable();

  virtual void setComment(QString comment);

  bool operator<(const FileBlobItem &other);
  QIcon icon() {return icon_;}
  void setIcon(const QIcon icon) {icon_ = icon;}
  dbif::ObjectHandle newRoot() {return newRoot_;}
  void setNewRoot(dbif::ObjectHandle root) {newRoot_ = root;}

 protected:
  void setFields(QString name, QString comment, uint64_t start, uint64_t end);
  void addChildren(const QList<FileBlobItem *> &children);
  void removeOldChildren();

  QString name_;
  QString comment_;
  QString value_;
  uint64_t start_;
  uint64_t end_;
  QIcon icon_;
  dbif::ObjectHandle newRoot_;

  dbif::ObjectHandle dataObj_;
  QList<FileBlobItem *> children_;

 protected slots:
  virtual void insertingChildrenHandle(FileBlobItem *item, bool before,
                                       int count);
  virtual void removingChildrenHandle(FileBlobItem *item, bool before);
  virtual void dataUpdatedHandle(FileBlobItem *item);

 signals:
  void insertingChildren(FileBlobItem *item, bool before, int count);
  void removingChildren(FileBlobItem *item, bool before);
  void dataUpdated(FileBlobItem *item);
};

}  // namespace ui
}  // namespace veles

#endif  // FILEBLOBITEM_H
