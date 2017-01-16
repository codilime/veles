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
#ifndef VELES_DB_OBJECT_H
#define VELES_DB_OBJECT_H

#include <QSet>
#include <QMap>
#include <QtGlobal>
#include <QEnableSharedFromThis>
#include "dbif/universe.h"
#include "dbif/types.h"
#include "db/types.h"
#include "data/bindata.h"

namespace veles {
namespace db {

class LocalObject : public QEnableSharedFromThis<LocalObject> {
  Universe *db_;
  QString name_;
  QString comment_;
  QSet<PLocalObject> children_;
  QSet<InfoGetter *> children_watchers_;
  QSet<InfoGetter *> description_watchers_;
  void children_reply(InfoGetter *getter);
  void remove_description_watcher(InfoGetter * getter);
  void remove_children_watcher(InfoGetter * getter);

 protected:
  virtual void killed() {}
  void description_updated();
  virtual void children_updated();
  virtual void description_reply(InfoGetter *getter);
  const QSet<PLocalObject> &children() { return children_; }

 public:
  LocalObject(Universe *db, QString name) : db_(db), name_(name) {}
  virtual ~LocalObject() { Q_ASSERT(dead()); }
  virtual void getInfo(InfoGetter *getter, PInfoRequest req, bool once);
  virtual void runMethod(MethodRunner *runner, PMethodRequest req);
  bool dead() const { return db_ == nullptr; }
  Universe *db() const { return db_; }
  void kill();
  void addChild(PLocalObject obj);
  void delChild(PLocalObject obj);
  virtual dbif::ObjectType type() const = 0;
  QString name() const { return name_; }
  QString comment() const { return comment_; }
};

class RootLocalObject : public LocalObject {
  friend class QSharedPointer<RootLocalObject>;
  RootLocalObject(Universe *db) : LocalObject(db, "root") {}

 public:
  void runMethod(MethodRunner *runner, PMethodRequest req) override;
  dbif::ObjectType type() const override { return dbif::ROOT; };

  static PLocalObject create(Universe *db) {
    return QSharedPointer<RootLocalObject>::create(db);
  }
};

class DataBlobObject : public LocalObject {
  LocalObject *parent_;
  data::BinData data_;
  QMap<InfoGetter *, std::pair<uint64_t, uint64_t>> data_watchers_;

  void data_reply(InfoGetter *getter, uint64_t start, uint64_t end);
  void remove_data_watcher(InfoGetter *getter);

 protected:
  DataBlobObject(LocalObject *parent, const data::BinData &data, const QString &name) :
    LocalObject(parent->db(), name), parent_(parent), data_(data) {}
  void description_reply(InfoGetter *getter) override;
  void killed() override;

 public:
  LocalObject *parent() { return parent_; }
  void getInfo(InfoGetter *getter, PInfoRequest req, bool once) override;
  void runMethod(MethodRunner *runner, PMethodRequest req) override;
  const data::BinData &data() const { return data_; }
};

class FileBlobObject : public DataBlobObject {
  friend class QSharedPointer<FileBlobObject>;
  QString path_;

  FileBlobObject(LocalObject *parent, const data::BinData &data, const QString &path) :
    DataBlobObject(parent, data, path), path_(path) {}

 protected:
  void description_reply(InfoGetter *getter) override;

 public:
  static PLocalObject create(LocalObject *parent,
    const data::BinData &data, const QString &path) {
    PLocalObject res = QSharedPointer<FileBlobObject>::create(parent, data, path);
    parent->addChild(res);
    return res;
  }
  dbif::ObjectType type() const override { return dbif::FILE_BLOB; };
  QString path() const { return path_; }
};

class SubBlobObject : public DataBlobObject {
  friend class QSharedPointer<SubBlobObject>;

  SubBlobObject(LocalObject *parent, const data::BinData &data, const QString &name) :
    DataBlobObject(parent, data, name) {}

 protected:
  void description_reply(InfoGetter *getter) override;

 public:
  static PLocalObject create(LocalObject *parent,
    const data::BinData &data, const QString &name) {
    PLocalObject res = QSharedPointer<SubBlobObject>::create(parent, data, name);
    parent->addChild(res);
    return res;
  }
  dbif::ObjectType type() const override { return dbif::SUB_BLOB; };
};

class ChunkObject : public LocalObject {
  friend class QSharedPointer<ChunkObject>;
  PLocalObject blob_;
  PLocalObject parent_chunk_;
  uint64_t start_;
  uint64_t end_;
  QString chunk_type_;
  std::vector<data::ChunkDataItem> items_;
  std::vector<data::ChunkDataItem> parseReplyItems_;
  QSet<InfoGetter *> parse_watchers_;

  ChunkObject(PLocalObject blob, PLocalObject parent_chunk,
              uint64_t start, uint64_t end, const QString &chunk_type,
              const QString &name) :
    LocalObject(blob->db(), name), blob_(blob), parent_chunk_(parent_chunk),
    start_(start), end_(end), chunk_type_(chunk_type) {}
  void calcParseReplyItems();
  void remove_parse_watcher(InfoGetter *getter);

 protected:
  void description_reply(InfoGetter *getter) override;
  virtual void children_updated() override;
  void parse_updated();
  virtual void parse_reply(InfoGetter *getter);
  void getInfo(InfoGetter *getter, PInfoRequest req, bool once) override;
  void runMethod(MethodRunner *runner, PMethodRequest req) override;
  dbif::ObjectType type() const override { return dbif::CHUNK; };
  void killed() override;

 public:
  static PLocalObject create(PLocalObject blob, PLocalObject parent_chunk,
                             uint64_t start, uint64_t end, const QString &chunk_type,
                             const QString &name) {
    PLocalObject res = QSharedPointer<ChunkObject>::create(blob, parent_chunk,
      start, end, chunk_type, name);
    if (parent_chunk)
      parent_chunk->addChild(res);
    else
      blob->addChild(res);
    return res;
  }
};

};
};

#endif
