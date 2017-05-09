/*
 * Copyright 2017 CodiLime
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
#pragma once

#include <set>
#include <map>
#include <random>
#include <memory>
#include <cstdint>
#include <iostream>
#include <unordered_map>
#include <unordered_set>

#include <QString>
#include <QTcpSocket>

#include "network/msgpackwrapper.h"
#include "data/nodeid.h"

namespace veles {
namespace client {

class NodeTree;

typedef std::pair<bool, std::shared_ptr<std::string>> pair_str;
typedef std::shared_ptr<proto::MsgpackMsg> msg_ptr;

/*****************************************************************************/
/* NetworkClient */
/*****************************************************************************/

class NetworkClient : public QObject {
  Q_OBJECT

 public:
  enum class ConnectionStatus{NotConnected, Connecting, Connected};
  static QString connStatusStr(ConnectionStatus status);

  NetworkClient(QObject* parent = nullptr);
  virtual ~NetworkClient();
  ConnectionStatus connectionStatus();
  void connect(
      QString server_name,
      int server_port,
      QString client_interface_name,
      QString client_name,
      QString client_version,
      QString client_description,
      QString client_type,
      QByteArray authentication_key,
      bool shut_down_on_disconnect);
  void disconnect();
  std::unique_ptr<NodeTree> const& nodeTree();
  uint64_t nextQid();

  QString serverHostName();
  int serverPort();
  QString clientInterfaceName();
  unsigned int protocolVersion();
  QString clientName();
  QString clientVersion();
  QString clientDescription();
  QString clientType();
  QString authenticationKey();

  QTextStream* output();
  void setOutput(QTextStream* stream);

  void sendMsgConnect();
  virtual void registerMessageHandlers();
  virtual void handleNodeTreeRelatedMessage(msg_ptr msg);
  virtual void handleConnectedMessage(msg_ptr msg);
  virtual void handleProtoErrorMessage(msg_ptr msg);
  virtual void handleConnectionsMessage(msg_ptr msg);
  virtual void handleRegistryReplyMessage(msg_ptr msg);
  virtual void handleMthdResMessage(msg_ptr msg);
  virtual void handlePluginTriggerRunMessage(msg_ptr msg);
  virtual void handleConnErrorMessage(msg_ptr msg);
  virtual void handlePluginMethodRunMessage(msg_ptr msg);
  virtual void handlePluginQueryGetMessage(msg_ptr msg);
  virtual void handleBroadcastRunMessage(msg_ptr msg);
  virtual void handlePluginHandlerUnregisteredMessage(msg_ptr msg);

  typedef void (NetworkClient::*MessageHandler)(msg_ptr);

signals:
  void connectionStatusChanged(ConnectionStatus connection_status);
  void messageReceived(msg_ptr message);

public slots:
  void sendMessage(msg_ptr msg);
  void setConnectionStatus(ConnectionStatus connection_status);
  void socketConnected();
  void socketDisconnected();
  void newDataAvailable();
  void socketError(QAbstractSocket::SocketError socketError);

 private:
  QTcpSocket* client_socket_;
  std::unique_ptr<NodeTree> node_tree_;
  ConnectionStatus status_;

  QString server_name_;
  uint16_t server_port_;
  QString client_interface_name_;

  unsigned int protocol_version_;
  QString client_name_;
  QString client_version_;
  QString client_description_;
  QString client_type_;
  QByteArray authentication_key_;
  bool shut_down_on_disconnect_;

  messages::MsgpackWrapper msgpack_wrapper_;
  std::unordered_map<std::string, MessageHandler> message_handlers_;

  QTextStream* output_stream_;
  uint64_t qid_;
};

}  // namespace client
}  // namespace veles
