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

#include <cstring>
#include <functional>
#include <memory>
#include <string>

#include <QHostAddress>
#include <QSslConfiguration>
#include <QSslSocket>

#include "client/networkclient.h"
#include "client/node.h"
#include "client/nodetree.h"
#include "proto/exceptions.h"

namespace veles {
namespace client {

/*****************************************************************************/
/* NetworkClient */
/*****************************************************************************/

QString NetworkClient::connStatusStr(ConnectionStatus status) {
  switch (status) {
    case ConnectionStatus::Connected:
      return QString("Connected");
      break;
    case ConnectionStatus::Connecting:
      return QString("Connecting");
      break;
    case ConnectionStatus::NotConnected:
      return QString("Not Connected");
      break;
    default:
      return QString("Unknown status");
      break;
  }
}

NetworkClient::NetworkClient(QObject* parent)
    : QObject(parent),
      node_tree_(nullptr),
      status_(ConnectionStatus::NotConnected),
      server_name_("127.0.0.1"),
      server_port_(3135),
      client_interface_name_("127.0.0.1"),
      protocol_version_(1),
      client_name_(""),
      client_version_("[unspecified version]"),
      client_description_(""),
      client_type_(""),
      authentication_key_(""),
      fingerprint_(""),
      quit_on_close_(false),
      ssl_enabled_(true),
      qid_(0) {
  NetworkClient::NetworkClient::registerMessageHandlers();
}

NetworkClient::~NetworkClient() {}

NetworkClient::ConnectionStatus NetworkClient::connectionStatus() {
  return status_;
}

void NetworkClient::connect(const QString& server_url,
                            const QString& client_interface_name,
                            const QString& client_name,
                            const QString& client_version,
                            const QString& client_description,
                            const QString& client_type, bool quit_on_close) {
  QString scheme = server_url.section("://", 0, 0).toLower();
  if (scheme == SCHEME_SSL) {
    if (QSslSocket::supportsSsl()) {
      ssl_enabled_ = true;
    } else {
      if (output() != nullptr) {
        *output() << "NetworkClient:: SSL error - check if "
                     "OpenSSL is available"
                  << endl;
      }
      return;
    }
  } else if (scheme == SCHEME_TCP) {
    ssl_enabled_ = false;
  } else if (scheme == SCHEME_UNIX) {
    if (output() != nullptr) {
      *output() << "NetworkClient:: Unix sockets are "
                   "currently unsupported"
                << endl;
    }
    return;
  } else {
    if (output() != nullptr) {
      *output() << "NetworkClient:: ERROR: unknown scheme provided!" << endl;
    }
    return;
  }
  QString url = server_url.section("://", 1);
  QString auth = url.section("@", 0, 0);
  QString loc = url.section("@", 1);

  server_name_ = loc.section(":", 0, -2);
  server_port_ = loc.section(":", -1, -1).toInt();
  client_interface_name_ = client_interface_name;
  client_name_ = client_name;
  client_version_ = client_version;
  client_description_ = client_description;
  client_type_ = client_type;
  quit_on_close_ = quit_on_close;

  authentication_key_ = QByteArray::fromHex(auth.section(":", 0, 0).toUtf8());
  fingerprint_ = auth.section(":", 1).replace(":", "").toLower();
  const int target_size = 64;
  int key_size = authentication_key_.size();
  authentication_key_.resize(target_size);
  for (int i = key_size; i < target_size; ++i) {
    authentication_key_[i] = 0;
  }

  if (status_ != ConnectionStatus::Connected &&
      status_ != ConnectionStatus::Connecting) {
    client_socket_ = new QSslSocket(this);
    if (ssl_enabled_) {
      QObject::connect(client_socket_, &QSslSocket::encrypted, this,
                       &NetworkClient::socketConnected, Qt::QueuedConnection);
      QObject::connect(
          client_socket_,
          static_cast<void (QSslSocket::*)(const QList<QSslError>&)>(
              &QSslSocket::sslErrors),
          this, &NetworkClient::checkFingerprint);
    } else {
      QObject::connect(client_socket_, &QAbstractSocket::connected, this,
                       &NetworkClient::socketConnected, Qt::QueuedConnection);
    }
    QObject::connect(client_socket_, &QAbstractSocket::disconnected, this,
                     &NetworkClient::socketDisconnected, Qt::QueuedConnection);
    QObject::connect(client_socket_, &QIODevice::readyRead, this,
                     &NetworkClient::newDataAvailable);
    QObject::connect(
        client_socket_,
        static_cast<void (QAbstractSocket::*)(QAbstractSocket::SocketError)>(
            &QAbstractSocket::error),
        this, &NetworkClient::socketError);

    if (output() != nullptr) {
      *output() << "NetworkClient::connect" << endl
                << "    client interface: " << client_interface_name_ << endl
                << "    server host: " << server_name_ << endl
                << "    server port: " << server_port_ << endl;
    }

    // TODO(altran01): Why is bind causing a problem here?
    // if (client_socket_->bind(QHostAddress(client_interface_name))) {
    if (output() != nullptr) {
      *output() << "NetworkClient: bind successful." << endl;
    }
    if (ssl_enabled_) {
      client_socket_->connectToHostEncrypted(server_name_, server_port_);
    } else {
      client_socket_->connectToHost(server_name_, server_port_);
    }
    setConnectionStatus(ConnectionStatus::Connecting);
    /*} else {
      if (output() != nullptr) {
        *output() << "NetworkClient: bind failed." << endl;
      }
    }*/
  }
}

void NetworkClient::disconnect() {
  if (output() != nullptr) {
    *output() << "NetworkClient: Disconnect." << endl;
  }

  setConnectionStatus(ConnectionStatus::NotConnected);

  if (client_socket_ != nullptr) {
    client_socket_->disconnectFromHost();
  }
}

std::unique_ptr<NodeTree> const& NetworkClient::nodeTree() {
  return node_tree_;
}

uint64_t NetworkClient::nextQid() { return ++qid_; }

unsigned int NetworkClient::protocolVersion() { return protocol_version_; }

QString NetworkClient::clientName() { return client_name_; }

QString NetworkClient::clientVersion() { return client_version_; }

QString NetworkClient::clientDescription() { return client_description_; }

QString NetworkClient::clientType() { return client_type_; }

QString NetworkClient::authenticationKey() { return authentication_key_; }

QTextStream* NetworkClient::output() { return output_stream_; }

void NetworkClient::setOutput(QTextStream* stream) { output_stream_ = stream; }

void NetworkClient::sendMsgConnect() {
  std::shared_ptr<std::string> client_name_ptr(
      new std::string(client_name_.toStdString()));
  std::shared_ptr<std::string> client_version_ptr(
      new std::string(client_version_.toStdString()));
  std::shared_ptr<std::string> client_description_ptr(
      new std::string(client_description_.toStdString()));
  std::shared_ptr<std::string> client_type_ptr(
      new std::string(client_type_.toStdString()));

  msg_ptr msg(new proto::MsgConnect(
      1, pair_str(true, client_name_ptr), pair_str(true, client_version_ptr),
      pair_str(true, client_description_ptr), pair_str(true, client_type_ptr),
      quit_on_close_));

  sendMessage(msg);
}

void NetworkClient::registerMessageHandlers() {
  message_handlers_["subscription_cancelled"] =
      &NetworkClient::handleNodeTreeRelatedMessage;
  message_handlers_["get_reply"] = &NetworkClient::handleNodeTreeRelatedMessage;
  message_handlers_["connected"] = &NetworkClient::handleConnectedMessage;
  message_handlers_["proto_error"] = &NetworkClient::handleProtoErrorMessage;
  message_handlers_["connections_reply"] =
      &NetworkClient::handleConnectionsMessage;
  message_handlers_["registry_reply"] =
      &NetworkClient::handleRegistryReplyMessage;
  message_handlers_["method_result"] = &NetworkClient::handleMthdResMessage;
  message_handlers_["method_error"] = &NetworkClient::handleMthdResMessage;
  message_handlers_["broadcast_result"] = &NetworkClient::handleMthdResMessage;
  message_handlers_["plugin_trigger_run"] =
      &NetworkClient::handlePluginTriggerRunMessage;
  message_handlers_["request_error"] =
      &NetworkClient::handleNodeTreeRelatedMessage;
  message_handlers_["get_list_reply"] =
      &NetworkClient::handleNodeTreeRelatedMessage;
  message_handlers_["get_data_reply"] =
      &NetworkClient::handleNodeTreeRelatedMessage;
  message_handlers_["get_query_reply"] =
      &NetworkClient::handleNodeTreeRelatedMessage;
  message_handlers_["query_error"] =
      &NetworkClient::handleNodeTreeRelatedMessage;
  message_handlers_["request_ack"] =
      &NetworkClient::handleNodeTreeRelatedMessage;
  message_handlers_["get_bindata_reply"] =
      &NetworkClient::handleNodeTreeRelatedMessage;
  message_handlers_["connection_error"] =
      &NetworkClient::handleConnErrorMessage;
  message_handlers_["plugin_method_run"] =
      &NetworkClient::handlePluginMethodRunMessage;
  message_handlers_["plugin_query_get"] =
      &NetworkClient::handlePluginQueryGetMessage;
  message_handlers_["plugin_broadcast_run"] =
      &NetworkClient::handleBroadcastRunMessage;
  message_handlers_["plugin_handler_unregistered"] =
      &NetworkClient::handlePluginHandlerUnregisteredMessage;
}

void NetworkClient::handleNodeTreeRelatedMessage(const msg_ptr& msg) {
  nodeTree()->addRemoteNodeTreeRelatedMessage(msg);
}

void NetworkClient::handleConnectedMessage(const msg_ptr& /*msg*/) {
  if (connectionStatus() != ConnectionStatus::Connecting) {
    if (output() != nullptr) {
      *output() << "NetworkClient: Very confusing... "
                   "Received \"connected\" message while already connected."
                << endl;
    }
  } else {
    if (output() != nullptr) {
      *output() << "NetworkClient: Received \"connected\" message." << endl;
    }

    setConnectionStatus(ConnectionStatus::Connected);
  }
}

void NetworkClient::handleProtoErrorMessage(const msg_ptr& msg) {
  auto* mpe = dynamic_cast<proto::MsgProtoError*>(msg.get());
  if (mpe != nullptr) {
    if (output() != nullptr) {
      *output() << "Received protocol error message. Aborting connection..."
                << endl
                << "    code: " << mpe->err->code.c_str()
                << "  msg: " << mpe->err->msg.c_str() << endl;
    }

    disconnect();
  }
}

void NetworkClient::handleConnectionsMessage(const msg_ptr& msg) {
  if (output() != nullptr) {
    *output() << "NetworkClient: Received \""
              << QString::fromStdString(msg->object_type) << "\" message."
              << endl;
  }

  // TODO(altran01): Is this something that client should implement in a
  // subclass?
}

void NetworkClient::handleRegistryReplyMessage(const msg_ptr& msg) {
  if (output() != nullptr) {
    *output() << "NetworkClient: Received \""
              << QString::fromStdString(msg->object_type) << "\" message."
              << endl;
  }

  // TODO(altran01): Is this something that client should implement in a
  // subclass?
}

void NetworkClient::handleMthdResMessage(const msg_ptr& msg) {
  if (output() != nullptr) {
    *output() << "NetworkClient: Received \""
              << QString::fromStdString(msg->object_type) << "\" message."
              << endl;
  }
  // TODO(altran01): Is this something that client should implement in a
  // subclass?
}

void NetworkClient::handlePluginTriggerRunMessage(const msg_ptr& msg) {
  if (output() != nullptr) {
    *output() << "NetworkClient: Received \""
              << QString::fromStdString(msg->object_type) << "\" message."
              << endl;
  }

  // TODO(altran01): Is this something that client should implement in a
  // subclass?
}

void NetworkClient::handleConnErrorMessage(const msg_ptr& msg) {
  auto* cem = dynamic_cast<proto::MsgConnectionError*>(msg.get());
  if (cem != nullptr) {
    if (output() != nullptr) {
      *output() << "Received connection error message. Aborting connection..."
                << endl
                << "    code: " << cem->err->code.c_str()
                << "  msg: " << cem->err->msg.c_str() << endl;
    }

    disconnect();
  }
}

void NetworkClient::handlePluginMethodRunMessage(const msg_ptr& msg) {
  if (output() != nullptr) {
    *output() << "NetworkClient: Received \""
              << QString::fromStdString(msg->object_type) << "\" message."
              << endl;
  }

  // TODO(altran01): Is this something that client should implement in a
  // subclass?
}

void NetworkClient::handlePluginQueryGetMessage(const msg_ptr& msg) {
  if (output() != nullptr) {
    *output() << "NetworkClient: Received \""
              << QString::fromStdString(msg->object_type) << "\" message."
              << endl;
  }

  // TODO(altran01): Is this something that client should implement in a
  // subclass?
}

void NetworkClient::handleBroadcastRunMessage(const msg_ptr& msg) {
  if (output() != nullptr) {
    *output() << "NetworkClient: Received \""
              << QString::fromStdString(msg->object_type) << "\" message."
              << endl;
  }

  // TODO(altran01): Is this something that client should implement in a
  // subclass?
}

void NetworkClient::handlePluginHandlerUnregisteredMessage(const msg_ptr& msg) {
  if (output() != nullptr) {
    *output() << "NetworkClient: Received \""
              << QString::fromStdString(msg->object_type) << "\" message."
              << endl;
  }

  // TODO(altran01): Is this something that client should implement in a
  // subclass?
}

void NetworkClient::sendMessage(const msg_ptr& msg) {
  if (client_socket_ != nullptr && client_socket_->isValid()) {
    msgpack::sbuffer buf;
    msgpack::packer<msgpack::sbuffer> packer(buf);
    messages::MsgpackWrapper::dumpObject(packer, msg);
    client_socket_->write(buf.data(), buf.size());
  }
}

void NetworkClient::setConnectionStatus(ConnectionStatus connection_status) {
  if (status_ != connection_status) {
    status_ = connection_status;
    if (output() != nullptr) {
      *output() << "NetworkClient: New connection status: "
                << connStatusStr(connection_status) << "." << endl;
    }
    emit connectionStatusChanged(status_);
  }
}

void NetworkClient::socketConnected() {
  if (output() != nullptr) {
    *output() << "NetworkClient: TCP socket connected - sending an "
                 "authentication key and \"connect\" message."
              << endl;
  }

  node_tree_ = std::make_unique<NodeTree>(this);
  client_socket_->write(authentication_key_);
  sendMsgConnect();
}

void NetworkClient::socketDisconnected() {
  setConnectionStatus(ConnectionStatus::NotConnected);
  if (output() != nullptr) {
    *output() << "NetworkClient: TCP socket disconnected." << endl;
  }

  if (node_tree_) {
    node_tree_.reset();
  }

  if (client_socket_ != nullptr) {
    client_socket_->deleteLater();
    client_socket_ = nullptr;
  }
}

void NetworkClient::newDataAvailable() {
  while (client_socket_ != nullptr) {
    msg_ptr msg = nullptr;
    try {
      msg = msgpack_wrapper_.loadMessage(client_socket_);
    } catch (proto::SchemaError& schema_error) {
      if (output() != nullptr) {
        *output() << "NetworkClient: SchemaError - "
                  << QString::fromStdString(schema_error.msg) << endl;
      }
    }

    if (msg) {
      auto handler_iter = message_handlers_.find(msg->object_type);
      if (handler_iter != message_handlers_.end()) {
        MessageHandler handler = handler_iter->second;
        (this->*handler)(msg);
      } else {
        if (output() != nullptr) {
          *output() << "NetworkClient: Received message of not handled "
                       "type: \""
                    << msg->object_type.c_str() << "\"." << endl;
        }
      }
      emit messageReceived(msg);
    } else {
      break;
    }
  }
}

void NetworkClient::socketError(QAbstractSocket::SocketError socketError) {
  setConnectionStatus(ConnectionStatus::NotConnected);
  if (output() != nullptr && client_socket_ != nullptr) {
    *output() << "NetworkClient: Socket error - "
              << client_socket_->errorString() << endl;
  }
}

void NetworkClient::checkFingerprint(const QList<QSslError>& errors) {
  bool fingerprint_valid = false;
  for (const auto& err : errors) {
    if (err.error() == QSslError::SelfSignedCertificate ||
        err.error() == QSslError::HostNameMismatch) {
      if (!fingerprint_valid) {
        QSslCertificate cert = err.certificate();
        if (cert.isNull()) {
          // This shouldn't happen for those 2 errors, but better safe than
          // sorry
          if (output() != nullptr) {
            *output() << "NetworkClient: received null certificate!" << endl;
          }
          return;
        }
        QByteArray remote_fingerprint =
            cert.digest(QCryptographicHash::Algorithm::Sha256).toHex();
        if (fingerprint_ == remote_fingerprint) {
          fingerprint_valid = true;
        } else {
          if (output() != nullptr) {
            *output()
                << "NetworkClient: Certificate fingerprint mismatch! Expected: "
                << fingerprint_ << ", got: " << remote_fingerprint << endl;
          }
          return;
        }
      }
    } else {
      if (output() != nullptr) {
        *output() << "NetworkClient: unexpected error: " << err.errorString()
                  << endl;
      }
      return;
    }
  }
  client_socket_->ignoreSslErrors(errors);
}

}  // namespace client
}  // namespace veles
