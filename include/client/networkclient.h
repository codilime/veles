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

#include <cstdint>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include <QSslSocket>
#include <QString>

#include "data/nodeid.h"
#include "network/msgpackwrapper.h"

namespace veles {
namespace client {

const QString SCHEME_UNIX("veles+unix");
const QString SCHEME_TCP("veles");
const QString SCHEME_SSL("veles+ssl");

class NodeTree;

typedef std::pair<bool, std::shared_ptr<std::string>> pair_str;
typedef std::shared_ptr<proto::MsgpackMsg> msg_ptr;

/*****************************************************************************/
/* NetworkClient */
/*****************************************************************************/

class NetworkClient : public QObject {
  Q_OBJECT

 public:
  enum class ConnectionStatus { NotConnected, Connecting, Connected };
  static QString connStatusStr(ConnectionStatus status);

  explicit NetworkClient(QObject* parent = nullptr);
  ~NetworkClient() override;
  ConnectionStatus connectionStatus();
  void connect(const QString& server_url, const QString& client_interface_name,
               const QString& client_name, const QString& client_version,
               const QString& client_description, const QString& client_type,
               bool quit_on_close);
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
  virtual void handleNodeTreeRelatedMessage(const msg_ptr& msg);
  virtual void handleConnectedMessage(const msg_ptr& msg);
  virtual void handleProtoErrorMessage(const msg_ptr& msg);
  virtual void handleConnectionsMessage(const msg_ptr& msg);
  virtual void handleRegistryReplyMessage(const msg_ptr& msg);
  virtual void handleMthdResMessage(const msg_ptr& msg);
  virtual void handlePluginTriggerRunMessage(const msg_ptr& msg);
  virtual void handleConnErrorMessage(const msg_ptr& msg);
  virtual void handlePluginMethodRunMessage(const msg_ptr& msg);
  virtual void handlePluginQueryGetMessage(const msg_ptr& msg);
  virtual void handleBroadcastRunMessage(const msg_ptr& msg);
  virtual void handlePluginHandlerUnregisteredMessage(const msg_ptr& msg);

  typedef void (NetworkClient::*MessageHandler)(const msg_ptr&);

 signals:
  void connectionStatusChanged(ConnectionStatus connection_status);
  void messageReceived(const msg_ptr& message);

 public slots:
  void sendMessage(const msg_ptr& msg);
  void setConnectionStatus(ConnectionStatus connection_status);
  void socketConnected();
  void socketDisconnected();
  void newDataAvailable();
  void socketError(QAbstractSocket::SocketError socketError);
  void checkFingerprint(const QList<QSslError>& errors);

 private:
  QSslSocket* client_socket_ = nullptr;
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
  QString fingerprint_;
  bool quit_on_close_;
  bool ssl_enabled_;

  messages::MsgpackWrapper msgpack_wrapper_;
  std::unordered_map<std::string, MessageHandler> message_handlers_;

  QTextStream* output_stream_ = nullptr;
  uint64_t qid_;
};

}  // namespace client
}  // namespace veles

Q_DECLARE_METATYPE(veles::client::NetworkClient::ConnectionStatus)
