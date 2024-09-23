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

#include "MyNode.h"
#include <homegear-base/Security/SecureVector.h>

#include <memory>

namespace MyNode {

MyNode::MyNode(const std::string &path, const std::string &type, const std::atomic_bool *frontendConnected) : Flows::INode(path, type, frontendConnected) {
  _bl.reset(new BaseLib::SharedObjects(false));

  _localRpcMethods.emplace("send", std::bind(&MyNode::send, this, std::placeholders::_1));
  _localRpcMethods.emplace("registerNode", std::bind(&MyNode::registerNode, this, std::placeholders::_1));

  std::string authRequiredHeader =
      "HTTP/1.1 401 Authorization Required\r\nWWW-Authenticate: Basic realm=\"Authentication Required\"\r\nConnection: Keep-Alive\r\nContent-Length: 255\r\n\r\n<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\"><html><head><meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"><title>Authorization Required</title></head><body>Authorization Required</body></html>";
  _authRequiredHeader.insert(_authRequiredHeader.end(), authRequiredHeader.begin(), authRequiredHeader.end());
}

MyNode::~MyNode() = default;

bool MyNode::init(const Flows::PNodeInfo &info) {
  _nodeInfo = info;
  return true;
}

bool MyNode::start() {
  try {
    BaseLib::HttpServer::HttpServerInfo serverInfo;
    serverInfo.packetReceivedCallback = std::bind(&MyNode::packetReceived, this, std::placeholders::_1, std::placeholders::_2);

    std::string listenAddress;
    std::string port;

    auto settingsIterator = _nodeInfo->info->structValue->find("listenaddress");
    if (settingsIterator != _nodeInfo->info->structValue->end()) listenAddress = settingsIterator->second->stringValue;

    if (!listenAddress.empty() && !BaseLib::Net::isIp(listenAddress)) listenAddress = BaseLib::Net::getMyIpAddress(listenAddress);
    else if (listenAddress.empty()) listenAddress = BaseLib::Net::getMyIpAddress();

    settingsIterator = _nodeInfo->info->structValue->find("port");
    if (settingsIterator != _nodeInfo->info->structValue->end()) port = settingsIterator->second->stringValue;

    settingsIterator = _nodeInfo->info->structValue->find("usetls");
    if (settingsIterator != _nodeInfo->info->structValue->end()) serverInfo.useSsl = settingsIterator->second->booleanValue;

    if (serverInfo.useSsl) {
      std::string tlsNodeId;
      settingsIterator = _nodeInfo->info->structValue->find("tls");
      if (settingsIterator != _nodeInfo->info->structValue->end()) tlsNodeId = settingsIterator->second->stringValue;

      if (!tlsNodeId.empty()) {
        auto certificateInfo = std::make_shared<C1Net::CertificateInfo>();
        certificateInfo->ca_file = getConfigParameter(tlsNodeId, "ca")->stringValue;
        certificateInfo->ca_data = getConfigParameter(tlsNodeId, "cadata.password")->stringValue;
        certificateInfo->cert_file = getConfigParameter(tlsNodeId, "cert")->stringValue;
        certificateInfo->cert_data = getConfigParameter(tlsNodeId, "certdata.password")->stringValue;
        certificateInfo->key_file = getConfigParameter(tlsNodeId, "key")->stringValue;
        certificateInfo->key_data = getConfigParameter(tlsNodeId, "keydata.password")->stringValue;
        serverInfo.listenAddress = listenAddress;
        serverInfo.port = Flows::Math::getUnsignedNumber(port);
        serverInfo.certificates.emplace("*", certificateInfo);
        serverInfo.dhParamData = getConfigParameter(tlsNodeId, "dhdata.password")->stringValue;
        serverInfo.dhParamFile = getConfigParameter(tlsNodeId, "dh")->stringValue;
        serverInfo.requireClientCert = getConfigParameter(tlsNodeId, "clientauth")->booleanValue;
      }
    }

    _username = getNodeData("username")->stringValue;
    _password = getNodeData("password")->stringValue;

    try {
      _socket = std::make_unique<BaseLib::HttpServer>(_bl.get(), serverInfo);
      _socket->start();
    }
    catch (BaseLib::Exception &ex) {
      _out->printError("Error starting server: " + std::string(ex.what()));
      return false;
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

void MyNode::stop() {
  try {
    _socket->stop();
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void MyNode::waitForStop() {
  try {
    _socket->waitForStop();
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

Flows::PVariable MyNode::getConfigParameterIncoming(const std::string &name) {
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

std::string &MyNode::createPathRegex(std::string &path, std::unordered_map<int32_t, std::string> &paramsMap) {
  if (path.empty()) return path;

  BaseLib::HelperFunctions::stringReplace(path, "[", "\\[");
  BaseLib::HelperFunctions::stringReplace(path, "]", "\\]");
  BaseLib::HelperFunctions::stringReplace(path, "?", "\\?");
  BaseLib::HelperFunctions::stringReplace(path, "(", "\\(");
  BaseLib::HelperFunctions::stringReplace(path, ")", "\\)");
  BaseLib::HelperFunctions::stringReplace(path, "\\", "\\\\");
  BaseLib::HelperFunctions::stringReplace(path, "$", "\\$");
  BaseLib::HelperFunctions::stringReplace(path, "^", "\\^");
  BaseLib::HelperFunctions::stringReplace(path, "*", "\\*");
  BaseLib::HelperFunctions::stringReplace(path, ".", "\\.");
  BaseLib::HelperFunctions::stringReplace(path, "|", "\\|");

  std::vector<std::string> parts = BaseLib::HelperFunctions::splitAll(path, '/');
  path.clear();
  for (uint32_t i = 0; i < parts.size(); i++) {
    if (parts[i].empty()) {
      if (path.empty() || path.back() != '/') path.append("\\/");
    } else if (parts[i].front() == ':') {
      if (parts[i].size() == 1) paramsMap.emplace(i, std::to_string(i - 1));
      else paramsMap.emplace(i, parts[i].substr(1));
      path.append("[^\\/]+");
      if (i != parts.size() - 1) path.append("\\/");
    } else if (parts[i].front() == '#') {
      if (parts[i].size() == 1) paramsMap.emplace(i, std::to_string(i - 1));
      else paramsMap.emplace(i, parts[i].substr(1));
      for (uint32_t j = i + 1; j < parts.size(); j++) {
        paramsMap.emplace(j, parts[j]);
      }
      path.append(".*");
      break;
    } else {
      path.append(parts[i]);
      if (i != parts.size() - 1) path.append("\\/");
    }
  }

  path = "^" + path + "$";
  return path;
}

C1Net::TcpPacket MyNode::getError(int32_t code, const std::string& longDescription) {
  std::string codeDescription = _http.getStatusText(code);
  std::string
      contentString = "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\"><html><head><title>" + std::to_string(code) + " " + codeDescription + "</title></head><body><h1>" + codeDescription + "</h1><p>" + longDescription + "<br/></p></body></html>";
  std::string header;
  std::vector<std::string> additionalHeaders;
  BaseLib::Http::constructHeader(contentString.size(), "text/html", code, codeDescription, additionalHeaders, header);
  C1Net::TcpPacket content;
  content.insert(content.end(), header.begin(), header.end());
  content.insert(content.end(), contentString.begin(), contentString.end());
  return content;
}

void MyNode::packetReceived(int32_t clientId, BaseLib::Http http) {
  try {
    if (!_username.empty()) //Basic Auth
    {
      if (http.getHeader().authorization.empty()) {
        _socket->send(clientId, _authRequiredHeader);
        return;
      }

      std::pair<std::string, std::string> authData = BaseLib::HelperFunctions::splitLast(http.getHeader().authorization, ' ');
      BaseLib::HelperFunctions::toLower(authData.first);
      if (authData.first != "basic") {
        _socket->send(clientId, _authRequiredHeader);
        return;
      }
      std::string decodedData;
      BaseLib::Base64::decode(authData.second, decodedData);
      std::pair<std::string, std::string> credentials = BaseLib::HelperFunctions::splitLast(decodedData, ':');
      BaseLib::HelperFunctions::toLower(credentials.first);
      if (credentials.first != _username || credentials.second != _password) {
        _socket->send(clientId, _authRequiredHeader);
        return;
      }
    }

    std::set<std::pair<std::string, Flows::PVariable>> nodes;

    {
      std::lock_guard<std::mutex> nodesGuard(_nodesMutex);
      for (auto &node : _nodes) {

        auto methodIterator = node.second.find(http.getHeader().method);
        if (methodIterator == node.second.end()) continue;
        if (std::regex_match(http.getHeader().path, methodIterator->second.pathRegex)) {
          Flows::PVariable params = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
          std::vector<std::string> parts = BaseLib::HelperFunctions::splitAll(http.getHeader().path, '/');
          for (uint32_t i = 0; i < parts.size(); i++) {
            if (parts[i].empty()) continue;
            auto paramIterator = methodIterator->second.paramsMap.find(i);
            if (paramIterator != methodIterator->second.paramsMap.end()) {
              params->structValue->emplace(paramIterator->second, std::make_shared<Flows::Variable>(parts[i]));
            } else {
              params->structValue->emplace(std::to_string(i - 1), std::make_shared<Flows::Variable>(parts[i]));
            }
          }

          std::pair<std::string, Flows::PVariable> nodeInfo = std::make_pair(methodIterator->second.id, params);
          nodes.emplace(nodeInfo);
        }
      }
    }
    if (nodes.empty()) {
      C1Net::TcpPacket response = getError(404, "The requested URL " + http.getHeader().path + " was not found on this server.");
      _socket->send(clientId, response);
      return;
    }

    Flows::PVariable headers = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
    for (auto &header : http.getHeader().fields) {
      if (header.first == "authorization") continue;
      headers->structValue->emplace(header.first, std::make_shared<Flows::Variable>(header.second));
    }

    Flows::PArray parameters = std::make_shared<Flows::Array>();
    parameters->reserve(8);
    parameters->push_back(std::make_shared<Flows::Variable>(clientId));
    parameters->push_back(std::make_shared<Flows::Variable>(http.getHeader().path));
    parameters->push_back(std::make_shared<Flows::Variable>(http.getHeader().args));
    parameters->push_back(std::make_shared<Flows::Variable>());
    parameters->push_back(std::make_shared<Flows::Variable>(http.getHeader().method));
    parameters->push_back(std::make_shared<Flows::Variable>(http.getHeader().contentType));
    parameters->push_back(headers);
    parameters->push_back(std::make_shared<Flows::Variable>());

    if (http.getHeader().contentType == "multipart/form-data") {
      Flows::PVariable content = std::make_shared<Flows::Variable>(Flows::VariableType::tArray);

      auto formData = http.decodeMultipartFormdata();
      for (const auto &element : formData) {
        if (content->arrayValue->size() + 1 > content->arrayValue->capacity()) content->arrayValue->reserve(content->arrayValue->size() + 11);
        if (element->contentType == "multipart/mixed") {
          for (const auto &innerElement : element->multipartMixed) {
            if (content->arrayValue->size() + 1 > content->arrayValue->capacity()) content->arrayValue->reserve(content->arrayValue->size() + 11);
            if (innerElement->contentType == "multipart/mixed") continue; //Not allowed nested

            Flows::PVariable formElement = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
            for (auto &header : innerElement->header) {
              formElement->structValue->emplace(header.first, std::make_shared<Flows::Variable>(header.second));
            }

            formElement->structValue->emplace("name", std::make_shared<Flows::Variable>(innerElement->name));
            formElement->structValue->emplace("filename", std::make_shared<Flows::Variable>(innerElement->filename));
            formElement->structValue->emplace("file", std::make_shared<Flows::Variable>(*(innerElement->data)));

            content->arrayValue->push_back(formElement);
          }
        } else {
          Flows::PVariable formElement = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
          for (auto &header : element->header) {
            formElement->structValue->emplace(header.first, std::make_shared<Flows::Variable>(header.second));
          }

          formElement->structValue->emplace("name", std::make_shared<Flows::Variable>(element->name));
          formElement->structValue->emplace("filename", std::make_shared<Flows::Variable>(element->filename));
          formElement->structValue->emplace("file", std::make_shared<Flows::Variable>(*(element->data)));

          content->arrayValue->push_back(formElement);
        }
      }

      parameters->at(7) = content;
    } else {
      std::string content(http.getContent().data(), http.getContentSize());
      parameters->at(7) = std::make_shared<Flows::Variable>(content);
    }

    for (const auto &node : nodes) {
      parameters->at(3) = node.second;
      invokeNodeMethod(node.first, "packetReceived", parameters, false);
    }
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

std::string MyNode::constructHeader(uint32_t contentLength, int32_t code, const Flows::PVariable& headers) {
  std::string header;

  std::string additionalHeaders;
  additionalHeaders.reserve(1024);
  for (auto &headerLine : *headers->arrayValue) {
    if (headerLine->stringValue.empty()) continue;
    if (headerLine->stringValue.compare(0, 8, "location") == 0) code = 301;
    additionalHeaders.append(headerLine->stringValue + "\r\n");
  }

  header.reserve(1024);
  header.append("HTTP/1.1 " + std::to_string(code) + " " + _http.getStatusText(code) + "\r\n");
  header.append("Connection: close\r\n");
  header.append(additionalHeaders);
  header.append("Content-Length: ").append(std::to_string(contentLength)).append("\r\n\r\n");

  return header;
}

//{{{ RPC methods
Flows::PVariable MyNode::send(const Flows::PArray& parameters) {
  try {
    if (parameters->size() != 4) return Flows::Variable::createError(-1, "Method expects exactly four parameters. " + std::to_string(parameters->size()) + " given.");
    if (parameters->at(0)->type != Flows::VariableType::tInteger && parameters->at(0)->type != Flows::VariableType::tInteger64) return Flows::Variable::createError(-1, "Parameter 1 is not of type integer.");
    if (parameters->at(1)->type != Flows::VariableType::tInteger && parameters->at(1)->type != Flows::VariableType::tInteger64) return Flows::Variable::createError(-1, "Parameter 2 is not of type integer.");
    if (parameters->at(2)->type != Flows::VariableType::tArray) return Flows::Variable::createError(-1, "Parameter 2 is not of type array.");
    if (parameters->at(3)->type != Flows::VariableType::tString) return Flows::Variable::createError(-1, "Parameter 4 is not of type string.");

    std::string header = constructHeader(parameters->at(3)->stringValue.size(), parameters->at(1)->integerValue, parameters->at(2));

    C1Net::TcpPacket response;
    response.insert(response.end(), header.begin(), header.end());
    response.insert(response.end(), parameters->at(3)->stringValue.begin(), parameters->at(3)->stringValue.end());

    _socket->send(parameters->at(0)->integerValue, response, true);

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

Flows::PVariable MyNode::registerNode(const Flows::PArray& parameters) {
  try {
    if (parameters->size() != 3) return Flows::Variable::createError(-1, "Method expects exactly 3 parameters. " + std::to_string(parameters->size()) + " given.");
    if (parameters->at(0)->type != Flows::VariableType::tString) return Flows::Variable::createError(-1, "Parameter 1 is not of type string.");
    if (parameters->at(1)->type != Flows::VariableType::tString) return Flows::Variable::createError(-1, "Parameter 2 is not of type string.");
    if (parameters->at(2)->type != Flows::VariableType::tString) return Flows::Variable::createError(-1, "Parameter 3 is not of type string.");

    NodeInfo info;
    info.id = parameters->at(0)->stringValue;

    std::unordered_map<int32_t, std::string> paramsMap;
    createPathRegex(parameters->at(2)->stringValue, info.paramsMap);

    info.pathRegex = std::regex(parameters->at(2)->stringValue);

    std::lock_guard<std::mutex> nodesGuard(_nodesMutex);
    _nodes[parameters->at(2)->stringValue].emplace(BaseLib::HelperFunctions::toUpper(parameters->at(1)->stringValue), std::move(info));

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
