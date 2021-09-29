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
    std::string address;
    std::string port;

    auto settingsIterator = _nodeInfo->info->structValue->find("sockettype");
    if (settingsIterator != _nodeInfo->info->structValue->end()) {
      if (settingsIterator->second->stringValue == "server") _type = SocketType::kServer;
      else if (settingsIterator->second->stringValue == "client") _type = SocketType::kClient;
      else _out->printError("Error: Unknown socket type: " + settingsIterator->second->stringValue);
    }

    if (_type == SocketType::kServer) {
      BaseLib::TcpSocket::TcpServerInfo serverInfo;
      serverInfo.packetReceivedCallback = std::bind(&TcpSocket::packetReceived, this, std::placeholders::_1, std::placeholders::_2);

      settingsIterator = _nodeInfo->info->structValue->find("listenaddress");
      if (settingsIterator != _nodeInfo->info->structValue->end()) address = settingsIterator->second->stringValue;

      if (!address.empty() && !BaseLib::Net::isIp(address)) address = BaseLib::Net::getMyIpAddress(address);
      else if (address.empty()) address = BaseLib::Net::getMyIpAddress();

      settingsIterator = _nodeInfo->info->structValue->find("listenport");
      if (settingsIterator != _nodeInfo->info->structValue->end()) port = settingsIterator->second->stringValue;

      settingsIterator = _nodeInfo->info->structValue->find("usetlsserver");
      if (settingsIterator != _nodeInfo->info->structValue->end()) serverInfo.useSsl = settingsIterator->second->booleanValue;

      if (serverInfo.useSsl) {
        std::string tlsNodeId;
        settingsIterator = _nodeInfo->info->structValue->find("tlsserver");
        if (settingsIterator != _nodeInfo->info->structValue->end()) tlsNodeId = settingsIterator->second->stringValue;

        if (!tlsNodeId.empty()) {
          BaseLib::TcpSocket::PCertificateInfo certificateInfo = std::make_shared<BaseLib::TcpSocket::CertificateInfo>();
          certificateInfo->caFile = getConfigParameter(tlsNodeId, "ca")->stringValue;
          certificateInfo->caData = getConfigParameter(tlsNodeId, "cadata.password")->stringValue;
          certificateInfo->certFile = getConfigParameter(tlsNodeId, "cert")->stringValue;
          certificateInfo->certData = getConfigParameter(tlsNodeId, "certdata.password")->stringValue;
          certificateInfo->keyFile = getConfigParameter(tlsNodeId, "key")->stringValue;
          auto keyData = getConfigParameter(tlsNodeId, "keydata.password")->stringValue;
          auto secureKeyData = std::make_shared<BaseLib::Security::SecureVector<uint8_t>>();
          secureKeyData->insert(secureKeyData->end(), keyData.begin(), keyData.end());
          certificateInfo->keyData = secureKeyData;
          serverInfo.certificates.emplace("*", certificateInfo);
          serverInfo.dhParamData = getConfigParameter(tlsNodeId, "dhdata.password")->stringValue;
          serverInfo.dhParamFile = getConfigParameter(tlsNodeId, "dh")->stringValue;
          serverInfo.requireClientCert = getConfigParameter(tlsNodeId, "clientauth")->booleanValue;
        }
      }

      _socket = std::make_shared<BaseLib::TcpSocket>(_bl.get(), serverInfo);

      try {
        std::string listenAddress;
        _socket->startServer(address, port, listenAddress);
        _out->printInfo("Info: Server is now listening on address " + listenAddress + ".");
      }
      catch (BaseLib::Exception &ex) {
        _out->printError("Error starting server: " + std::string(ex.what()));
        return false;
      }
    } else if (_type == SocketType::kClient) {
      bool useTls = false;

      settingsIterator = _nodeInfo->info->structValue->find("address");
      if (settingsIterator != _nodeInfo->info->structValue->end()) address = settingsIterator->second->stringValue;

      settingsIterator = _nodeInfo->info->structValue->find("port");
      if (settingsIterator != _nodeInfo->info->structValue->end()) port = settingsIterator->second->stringValue;

      settingsIterator = _nodeInfo->info->structValue->find("usetlsclient");
      if (settingsIterator != _nodeInfo->info->structValue->end()) useTls = settingsIterator->second->booleanValue;

      if (useTls) {
        std::string tlsNodeId;
        settingsIterator = _nodeInfo->info->structValue->find("tlsclient");
        if (settingsIterator != _nodeInfo->info->structValue->end()) tlsNodeId = settingsIterator->second->stringValue;

        if (!tlsNodeId.empty()) {
          BaseLib::TcpSocket::PCertificateInfo certificateInfo = std::make_shared<BaseLib::TcpSocket::CertificateInfo>();
          certificateInfo->caFile = getConfigParameter(tlsNodeId, "ca")->stringValue;
          certificateInfo->caData = getConfigParameter(tlsNodeId, "cadata.password")->stringValue;
          certificateInfo->certFile = getConfigParameter(tlsNodeId, "cert")->stringValue;
          certificateInfo->certData = getConfigParameter(tlsNodeId, "certdata.password")->stringValue;
          certificateInfo->keyFile = getConfigParameter(tlsNodeId, "key")->stringValue;
          auto keyData = getConfigParameter(tlsNodeId, "keydata.password")->stringValue;
          auto secureKeyData = std::make_shared<BaseLib::Security::SecureVector<uint8_t>>();
          secureKeyData->insert(secureKeyData->end(), keyData.begin(), keyData.end());
          certificateInfo->keyData = secureKeyData;

          _socket =
              std::make_shared<BaseLib::TcpSocket>(_bl.get(), address, port, useTls, true, certificateInfo->caFile, certificateInfo->caData, certificateInfo->certFile, certificateInfo->certData, certificateInfo->keyFile, certificateInfo->keyData);
        }
      } else {
        _socket = std::make_shared<BaseLib::TcpSocket>(_bl.get(), address, port);
      }

      if (_socket) {
        _socket->setConnectionRetries(1);
        _socket->setReadTimeout(100000);
      }
    } else _out->printError("Error: Invalid socket type.");

    _stopListenThread = true;
    if (_listenThread.joinable()) _listenThread.join();
    if (_type == SocketType::kClient) {
      _stopListenThread = false;
      _listenThread = std::thread(&TcpSocket::listen, this);
    }

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
    _stopListenThread = true;
    if (_type == SocketType::kServer) {
      if (_socket) _socket->stopServer();
    } else if (_type == SocketType::kClient) {
      if (_socket) _socket->close();
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
      if (_socket) _socket->waitForServerStopped();
    } else if (_type == SocketType::kClient) {
      if (_socket) _socket->close();
    }
    if (_listenThread.joinable()) _listenThread.join();
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

void TcpSocket::packetReceived(int32_t clientId, BaseLib::TcpSocket::TcpPacket &packet) {
  try {
    auto parameters = std::make_shared<Flows::Array>();
    parameters->reserve(2);
    parameters->push_back(std::make_shared<Flows::Variable>(clientId));
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

void TcpSocket::listen() {
  try {
    BaseLib::TcpSocket::TcpPacket data;
    std::vector<char> buffer(4096);
    uint32_t bytesReceived = 0;

    while (!_stopListenThread) {
      if (!_socket->connected()) {
        _socket->open();
        if (!_socket->connected()) {
          setConnectionState(false);
          for (int32_t i = 0; i < 10; i++) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            if (_stopListenThread) return;
          }
          continue;
        } else {
          setConnectionState(true);
        }
      }

      try {
        bytesReceived = _socket->proofread(buffer.data(), buffer.size());
        if (bytesReceived > 0) {
          data.clear();
          data.insert(data.end(), buffer.begin(), buffer.begin() + bytesReceived);
          packetReceived(0, data);
        }
      }
      catch (BaseLib::SocketClosedException &ex) {
        _socket->close();
        _out->printWarning("Warning: Connection to server closed.");
        continue;
      }
      catch (BaseLib::SocketTimeOutException &ex) {
        continue;
      }
      catch (BaseLib::SocketOperationException &ex) {
        _socket->close();
        _out->printError("Error: " + std::string(ex.what()));
        continue;
      }
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
      _socket->sendToClient(parameters->at(0)->integerValue, parameters->at(1)->binaryValue);
    } else {
      _socket->proofwrite((const char *)parameters->at(1)->binaryValue.data(), parameters->at(1)->binaryValue.size());
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
      parameters->push_back(std::make_shared<Flows::Variable>(_socket && _socket->connected()));
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
