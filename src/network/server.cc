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
#include "db/handle.h"
#include "db/object.h"
#include "dbif/types.h"
#include "network/server.h"
#include "util/settings/network.h"

#include <QtNetwork/QTcpSocket>
#include <QtEndian>
#include <QDataStream>
#include <QSettings>

namespace veles {
namespace db {

NetworkServer::NetworkServer(PLocalObject root) :
  root_(root), tcp_server_(new QTcpServer(this)) {
  uint32_t port = util::settings::network::port();
  QHostAddress ip_addr(util::settings::network::ipAddress());
  if (!tcp_server_->listen(ip_addr, port)) {
    // TODO some error logging here
    return;
  }
  connect(tcp_server_, &QTcpServer::newConnection, this, &NetworkServer::handle);
}

void NetworkServer::handle() {
  QTcpSocket *client_connection = tcp_server_->nextPendingConnection();
  connect(client_connection, &QAbstractSocket::disconnected,
          client_connection, &QObject::deleteLater);
  connect(client_connection, &QIODevice::readyRead, [this, client_connection] () {
    readMessage(client_connection);
  });
}

void NetworkServer::readMessage(QTcpSocket *client_connection) {
  uint32_t msg_len;
  if (client_connection->bytesAvailable() < static_cast<int32_t>(sizeof(msg_len))) {
    return;
  }

  client_connection->peek(reinterpret_cast<char*>(&msg_len), sizeof(msg_len));
  msg_len = qFromLittleEndian(msg_len);
  if (msg_len > k_max_msg_len_) {
    network::Response response;
    response.set_ok(false);
    response.set_error_msg("Request too long - breaking conection.");
    sendResponse(client_connection, response);
    client_connection->close();
    return;
  }
  if (client_connection->bytesAvailable() >=
      static_cast<int32_t>(sizeof(msg_len)) + msg_len) {
    QScopedArrayPointer<char> message(new char[msg_len]);
    // We don't need it anymore, but we need to get rid of it either way.
    client_connection->read(reinterpret_cast<char*>(&msg_len), sizeof(msg_len));
    client_connection->read(&message[0], msg_len);
    network::Request request;
    if (!request.ParseFromArray(&message[0], msg_len)) {
      network::Response response;
      response.set_ok(false);
      response.set_error_msg("Failed to decode request.");
      sendResponse(client_connection, response);
      return;
    }
    // TODO some error handling so that we respond if we fail to process request
    handleRequest(request, client_connection);
  }
}

void NetworkServer::listChildren(PLocalObject target_object,
                                 network::Response &resp, bool list_children) {
  for (auto child : target_object->children()) {
    network::LocalObject* result = resp.add_results();
    packObject(child, result, list_children);
  }
  resp.set_ok(true);
}

void NetworkServer::createChunk(PLocalObject target_object, PLocalObject blob,
                                network::Request &req, network::Response &resp) {
  PLocalObject parent;
  if (target_object->type() == dbif::CHUNK) {
    parent = target_object;
  } else if (target_object->type() != dbif::FILE_BLOB){
    resp.set_ok(false);
    resp.set_error_msg("Bad ID provided.");
    return;
  }

  PLocalObject created = ChunkObject::create(blob, parent,
                                req.chunk_start(), req.chunk_end(),
                                QString::fromStdString(req.chunk_type()),
                                QString::fromStdString(req.name()));
  created->setComment(QString::fromStdString(req.comment()));
  network::LocalObject* result = resp.add_results();
  packObject(created, result);
  resp.set_ok(true);
}

void NetworkServer::deleteObject(PLocalObject target_object,
                                 network::Response &resp) {
  if (target_object->type() == dbif::ROOT ||
      target_object->type() == dbif::FILE_BLOB) {
    resp.set_ok(false);
    resp.set_error_msg("Unsupported object type to delete.");
    return;
  }
  target_object->kill();
  resp.set_ok(true);
}

void NetworkServer::getBlobData(PLocalObject target_object, QTcpSocket *client_connection, network::Response &resp) {
  if (target_object->type() != dbif::FILE_BLOB &&
      target_object->type() != dbif::SUB_BLOB) {
    resp.set_ok(false);
    resp.set_error_msg("Unsupported object type to get file data.");
    sendResponse(client_connection, resp);
    return;
  }
  resp.set_ok(true);
  sendResponse(client_connection, resp);

  auto blob = target_object.staticCast<DataBlobObject>();
  sendData(client_connection, reinterpret_cast<const char*>(blob->data().rawData()), blob->data().octets());
}

void NetworkServer::handleRequest(network::Request &req, QTcpSocket *client_connection) {
  network::Response resp;
  PLocalObject target_object = root_;
  PLocalObject blob;
  bool found;

  for (int j = 0; j < req.id_size(); j++) {
    found = false;
    for (auto child : target_object->children()) {
      if (child->id() == req.id(j)) {
        if (child->type() == dbif::FILE_BLOB) {
          blob = child;
        }
        found = true;
        target_object = child;
        break;
      }
    }

    if (!found) {
      resp.set_ok(false);
      resp.set_error_msg("Bad ID provided.");
      sendResponse(client_connection, resp);
      return;
    }
  }

  switch (req.type()) {
  case network::Request::LIST_CHILDREN:
    listChildren(target_object, resp);
    break;
  case network::Request::LIST_CHILDREN_RECURSIVE:
    listChildren(target_object, resp, true);
    break;
  case network::Request::ADD_CHILD_CHUNK:
    createChunk(target_object, blob, req, resp);
    break;
  case network::Request::DELETE_OBJECT:
    deleteObject(target_object, resp);
    break;
  case network::Request::GET_BLOB_DATA:
    getBlobData(target_object, client_connection, resp);
    // we already sent responses
    return;
  default:
    resp.set_ok(false);
    resp.set_error_msg("Unknown request type.");
    break;
  }

  sendResponse(client_connection, resp);
}

void NetworkServer::sendResponse(QTcpSocket *client_connection,
                                 network::Response &resp) {
  int32_t resp_len = resp.ByteSize();
  QScopedArrayPointer<char> response_content(new char[resp_len]);
  resp.SerializeToArray(&response_content[0], resp_len);
  sendData(client_connection, &response_content[0], resp_len);
}

void NetworkServer::sendData(QTcpSocket *client_connection, const char* data, int64_t length) {
  uint32_t resp_len_send = qToLittleEndian(length);
  int64_t written = 0, total_written = 0;
  while (total_written < sizeof(resp_len_send)) {
    written = client_connection->write(
          ((const char*)&resp_len_send) + total_written,
          sizeof(resp_len_send) - total_written);
    if (written == -1) {
      // TODO log some error message here
      return;
    }
    total_written += written;
  }

  total_written = 0;
  while (total_written < length) {
    written = client_connection->write(data + total_written,
        length - total_written);
    if (written == -1) {
      // TODO log some error message here
      return;
    }
    total_written += written;
  }
}

void NetworkServer::packObject(PLocalObject object,
                               network::LocalObject *result,
                               bool pack_children) {
  result->set_id(object->id());
  result->set_name(object->name().toStdString());
  result->set_comment(object->comment().toStdString());
  result->set_type(object->type());

  if (object->type() == dbif::FILE_BLOB) {
    auto file = object.dynamicCast<FileBlobObject>();
    result->set_file_blob_path(file->path().toStdString());
  } else if (object->type() == dbif::CHUNK){
    auto chunk = object.dynamicCast<ChunkObject>();
    result->set_chunk_start(chunk->start());
    result->set_chunk_end(chunk->end());
    result->set_chunk_type(chunk->chunkType().toStdString());
    for (auto item : chunk->items()) {
      if (item.type != data::ChunkDataItem::FIELD) {
        continue;
      }
      network::ChunkDataItem* packed_item = result->add_items();
      packed_item->set_start(item.start);
      packed_item->set_end(item.end);
      packed_item->set_name(item.name.toStdString());
    }
  }

  if (pack_children) {
    for (auto child : object->children()) {
      network::LocalObject* packed_child = result->add_children();
      packObject(child, packed_child, true);
    }
  }
}

}  // namespace db
}  // namespace veles
