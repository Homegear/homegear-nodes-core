/* Copyright 2013-2019 Homegear GmbH
 *
 * Homegear is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Homegear is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Homegear.  If not, see <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 */

#include "TcpSocket.h"
#include "../../timers/weekly-program/MyNode.h"

#include <homegear-base/Security/SecureVector.h>

namespace TcpSocket {

TcpSocket::TcpSocket(const std::string &path, const std::string &type, const std::atomic_bool *frontendConnected) : Flows::INode(path, type, frontendConnected) {
  _bl.reset(new BaseLib::SharedObjects(false));

  _localRpcMethods.emplace("send", std::bind(&TcpSocket::send, this, std::placeholders::_1));
  _localRpcMethods.emplace("registerNode", std::bind(&TcpSocket::registerNode, this, std::placeholders::_1));
}

TcpSocket::~TcpSocket() = default;

bool TcpSocket::init(const Flows::PNodeInfo &info) {
  try {
    _nodeInfo = info;
    return true;
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
  return false;
}

bool TcpSocket::start() {
  try {
    auto settingsIterator = _nodeInfo->info->structValue->find("sockettype");
    if (settingsIterator != _nodeInfo->info->structValue->end()) {
      if (settingsIterator->second->stringValue == "server") _type = SocketType::kServer;
      else if (settingsIterator->second->stringValue == "client") _type = SocketType::kClient;
      else _out->printError("Error: Unknown socket type: " + settingsIterator->second->stringValue);
    }

    if (_type == SocketType::kServer) {
      C1Net::TcpServer::TcpServerInfo server_info;
      server_info.log_callback = std::bind(&TcpSocket::log, this, std::placeholders::_1, std::placeholders::_2);
      server_info.packet_received_callback = std::bind(&TcpSocket::packetReceivedServer, this, std::placeholders::_1, std::placeholders::_2);

      settingsIterator = _nodeInfo->info->structValue->find("listenaddress");
      if (settingsIterator != _nodeInfo->info->structValue->end()) server_info.listen_address = settingsIterator->second->stringValue;

      if (!server_info.listen_address.empty() && !BaseLib::Net::isIp(server_info.listen_address)) server_info.listen_address = BaseLib::Net::getMyIpAddress(server_info.listen_address);
      else if (server_info.listen_address.empty()) server_info.listen_address = BaseLib::Net::getMyIpAddress();

      settingsIterator = _nodeInfo->info->structValue->find("listenport");
      if (settingsIterator != _nodeInfo->info->structValue->end()) server_info.port = Flows::Math::getUnsignedNumber(settingsIterator->second->stringValue);

      settingsIterator = _nodeInfo->info->structValue->find("usetlsserver");
      if (settingsIterator != _nodeInfo->info->structValue->end()) server_info.tls = settingsIterator->second->booleanValue;

      if (server_info.tls) {
        std::string tlsNodeId;
        settingsIterator = _nodeInfo->info->structValue->find("tlsserver");
        if (settingsIterator != _nodeInfo->info->structValue->end()) tlsNodeId = settingsIterator->second->stringValue;

        if (!tlsNodeId.empty()) {
          auto certificateInfo = std::make_shared<C1Net::CertificateInfo>();
          certificateInfo->ca_file = getConfigParameter(tlsNodeId, "ca")->stringValue;
          certificateInfo->ca_data = getConfigParameter(tlsNodeId, "cadata.password")->stringValue;
          certificateInfo->cert_file = getConfigParameter(tlsNodeId, "cert")->stringValue;
          certificateInfo->cert_data = getConfigParameter(tlsNodeId, "certdata.password")->stringValue;
          certificateInfo->key_file = getConfigParameter(tlsNodeId, "key")->stringValue;
          certificateInfo->key_data = getConfigParameter(tlsNodeId, "keydata.password")->stringValue;
          server_info.certificates.emplace("*", certificateInfo);
          server_info.require_client_cert = getConfigParameter(tlsNodeId, "clientauth")->booleanValue;
        }
      }

      tcp_server_ = std::make_shared<C1Net::TcpServer>(server_info);

      try {
        tcp_server_->Start();
        _out->printInfo("Info: Server is now listening.");
      }
      catch (BaseLib::Exception &ex) {
        _out->printError("Error starting server: " + std::string(ex.what()));
        return false;
      }
    } else if (_type == SocketType::kClient) {
      C1Net::TcpClient::TcpClientInfo client_info;

      client_info.log_callback = std::bind(&TcpSocket::log, this, std::placeholders::_1, std::placeholders::_2);
      client_info.packet_received_callback = std::bind(&TcpSocket::packetReceivedClient, this, std::placeholders::_1);

      settingsIterator = _nodeInfo->info->structValue->find("address");
      if (settingsIterator != _nodeInfo->info->structValue->end()) client_info.host = settingsIterator->second->stringValue;

      settingsIterator = _nodeInfo->info->structValue->find("port");
      if (settingsIterator != _nodeInfo->info->structValue->end()) client_info.port = Flows::Math::getUnsignedNumber(settingsIterator->second->stringValue);

      settingsIterator = _nodeInfo->info->structValue->find("usetlsclient");
      if (settingsIterator != _nodeInfo->info->structValue->end()) client_info.tls = settingsIterator->second->booleanValue;

      if (client_info.tls) {
        std::string tlsNodeId;
        settingsIterator = _nodeInfo->info->structValue->find("tlsclient");
        if (settingsIterator != _nodeInfo->info->structValue->end()) tlsNodeId = settingsIterator->second->stringValue;

        if (!tlsNodeId.empty()) {
          client_info.ca_file = getConfigParameter(tlsNodeId, "ca")->stringValue;
          client_info.ca_data = getConfigParameter(tlsNodeId, "cadata.password")->stringValue;
          client_info.client_cert_file = getConfigParameter(tlsNodeId, "cert")->stringValue;
          client_info.client_cert_data = getConfigParameter(tlsNodeId, "certdata.password")->stringValue;
          client_info.client_key_file = getConfigParameter(tlsNodeId, "key")->stringValue;
          client_info.client_key_data = getConfigParameter(tlsNodeId, "keydata.password")->stringValue;
        }
      }

      client_info.read_timeout = 10000;
      client_info.write_timeout = 10000;

      tcp_client_ = std::make_shared<C1Net::TcpClient>(client_info);
      try {
        tcp_client_->Start();
        _out->printInfo("Info: Client is now started.");
      }
      catch (BaseLib::Exception &ex) {
        _out->printError("Error starting client: " + std::string(ex.what()));
        return false;
      }
    } else _out->printError("Error: Invalid socket type.");

    return true;
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
  return false;
}

void TcpSocket::stop() {
  try {
    if (_type == SocketType::kServer) {
      if (tcp_server_) tcp_server_->Stop();
    } else if (_type == SocketType::kClient) {
      if (tcp_client_) tcp_client_->Stop();
    }
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void TcpSocket::waitForStop() {
  try {
    if (_type == SocketType::kServer) {
      if (tcp_server_) tcp_server_->WaitForServerStopped();
    } else if (_type == SocketType::kClient) {
      if (tcp_client_) tcp_client_->WaitForClientStopped();
    }
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

Flows::PVariable TcpSocket::getConfigParameterIncoming(const std::string &name) {
  try {
    auto settingsIterator = _nodeInfo->info->structValue->find(name);
    if (settingsIterator != _nodeInfo->info->structValue->end()) return settingsIterator->second;
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
  return std::make_shared<Flows::Variable>();
}

void TcpSocket::log(uint32_t log_level, const std::string &message) {
  _out->printMessage(message, (int32_t)log_level);
}

void TcpSocket::packetReceivedServer(const C1Net::TcpServer::PTcpClientData &client_data, const C1Net::TcpPacket &packet) {
  try {
    auto parameters = std::make_shared<Flows::Array>();
    parameters->reserve(2);
    parameters->push_back(std::make_shared<Flows::Variable>(client_data ? client_data->GetId() : 0));
    parameters->push_back(std::make_shared<Flows::Variable>(packet));
    std::lock_guard nodesGuard(_nodesMutex);
    for (auto &node: _nodes) {
      if (node.second == NodeType::kOutput) continue;
      auto result = invokeNodeMethod(node.first, "packetReceived", parameters, true);
      if (result->errorStruct) _out->printError("Error: Could not call packet received on node " + node.first + ": " + result->structValue->at("faultString")->stringValue);
    }
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void TcpSocket::packetReceivedClient(const C1Net::TcpPacket &packet) {
  packetReceivedServer(nullptr, packet);
}

void TcpSocket::setConnectionState(bool value) {
  try {
    Flows::PArray parameters = std::make_shared<Flows::Array>();
    parameters->push_back(std::make_shared<Flows::Variable>(value));

    std::lock_guard<std::mutex> nodesGuard(_nodesMutex);
    for (auto &node: _nodes) {
      invokeNodeMethod(node.first, "setConnectionState", parameters, false);
    }
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

//{{{ RPC methods
Flows::PVariable TcpSocket::send(const Flows::PArray &parameters) {
  try {
    if (parameters->size() != 2) return Flows::Variable::createError(-1, "Method expects exactly four parameters. " + std::to_string(parameters->size()) + " given.");

    if (parameters->at(0)->type != Flows::VariableType::tInteger && parameters->at(0)->type != Flows::VariableType::tInteger64) return Flows::Variable::createError(-1, "Parameter 1 is not of type Integer.");
    if (parameters->at(1)->type != Flows::VariableType::tBinary) return Flows::Variable::createError(-1, "Parameter 2 is not of type String.");

    if (_type == SocketType::kServer) {
      tcp_server_->Send(parameters->at(0)->integerValue, parameters->at(1)->binaryValue);
    } else {
      tcp_client_->Send(parameters->at(1)->binaryValue);
    }

    return std::make_shared<Flows::Variable>();
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
  return Flows::Variable::createError(-32500, "Unknown application error.");
}

Flows::PVariable TcpSocket::registerNode(const Flows::PArray &parameters) {
  try {
    if (parameters->size() != 2) return Flows::Variable::createError(-1, "Method expects exactly 1 parameter. " + std::to_string(parameters->size()) + " given.");
    if (parameters->at(0)->type != Flows::VariableType::tString) return Flows::Variable::createError(-1, "Parameter 1 is not of type String.");
    if (parameters->at(1)->type != Flows::VariableType::tInteger && parameters->at(1)->type != Flows::VariableType::tInteger64) return Flows::Variable::createError(-1, "Parameter 2 is not of type Integer.");

    auto nodeId = parameters->at(0)->stringValue;

    std::lock_guard<std::mutex> nodesGuard(_nodesMutex);
    _nodes.emplace(nodeId, (NodeType)parameters->at(1)->integerValue);

    if (_type == SocketType::kClient) {
      Flows::PArray parameters = std::make_shared<Flows::Array>();
      parameters->push_back(std::make_shared<Flows::Variable>(tcp_client_ && tcp_client_->Connected()));
      invokeNodeMethod(nodeId, "setConnectionState", parameters, false);
    }

    return std::make_shared<Flows::Variable>();
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
  return Flows::Variable::createError(-32500, "Unknown application error.");
}
//}}}

}
