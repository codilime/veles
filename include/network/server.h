/*
 * Copyright 2016-2017 CodiLime
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

#ifndef VELES_NETWORK_SERVER_H
#define VELES_NETWORK_SERVER_H

#include <QtNetwork/QTcpServer>

#include "db/types.h"
#include "network.pb.h"

namespace veles {
namespace db {

class NetworkServer : public QObject {
  Q_OBJECT

  void handleRequest(network::Request &req, QTcpSocket *client_connection);
  void sendResponse(QTcpSocket *client_connection, network::Response &resp);
  void sendData(QTcpSocket *client_connection, const char *data, uint64_t length);
  void packObject(PLocalObject object, network::LocalObject* result,
                  bool pack_children = false);

public:
  NetworkServer(PLocalObject root);

private slots:
  void handle();
  void readMessage(QTcpSocket *client_connection);

private:
  PLocalObject root_;
  QTcpServer *tcp_server_;

  static const uint32_t k_max_msg_len_ = 1024*1024*16;

  void listChildren(PLocalObject target_object, network::Response &resp,
                    bool list_children = false);
  void createChunk(PLocalObject target_object, PLocalObject blob,
                   network::Request &req, network::Response &resp);
  void deleteObject(PLocalObject target_object, network::Response &resp);
  void getBlobData(PLocalObject target_object, QTcpSocket *client_connection,
                   network::Response &resp);
};

}  // namespace db
}  // namespace veles

#endif // VELES_NETWORK_SERVER_H
