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

namespace MyNode {

MyNode::MyNode(const std::string &path, const std::string &type, const std::atomic_bool *frontendConnected) : Flows::INode(path, type, frontendConnected) {
  _bl = std::unique_ptr<BaseLib::SharedObjects>(new BaseLib::SharedObjects());
  _jsonDecoder = std::unique_ptr<Flows::JsonDecoder>(new Flows::JsonDecoder());
}

MyNode::~MyNode() = default;

bool MyNode::init(const Flows::PNodeInfo &info) {
  try {
    auto settingsIterator = info->info->structValue->find("url");
    if (settingsIterator != info->info->structValue->end()) _url = settingsIterator->second->stringValue;

    settingsIterator = info->info->structValue->find("method");
    if (settingsIterator != info->info->structValue->end() && settingsIterator->second->stringValue != "use") {
      _method = settingsIterator->second->stringValue;
    }

    settingsIterator = info->info->structValue->find("ret");
    if (settingsIterator != info->info->structValue->end()) {
      if (settingsIterator->second->stringValue == "bin") _returnType = ReturnType::bin;
      else if (settingsIterator->second->stringValue == "obj") _returnType = ReturnType::obj;
      else _returnType = ReturnType::txt;
    }

    settingsIterator = info->info->structValue->find("tls");
    if (settingsIterator != info->info->structValue->end()) _tlsNode = settingsIterator->second->stringValue;

    settingsIterator = info->info->structValue->find("usetls");
    if (settingsIterator != info->info->structValue->end()) _useTls = settingsIterator->second->booleanValue;

    settingsIterator = info->info->structValue->find("basicauth");
    if (settingsIterator != info->info->structValue->end()) _useBasicAuth = settingsIterator->second->booleanValue;

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

void MyNode::configNodesStarted() {
  try {
    std::string username = getNodeData("username")->stringValue;
    std::string password = getNodeData("password")->stringValue;

    if (_useBasicAuth && !username.empty()) {
      std::string raw = username + ":" + password;
      BaseLib::Base64::encode(raw, _basicAuth);
      _basicAuth = "Authorization: Basic " + _basicAuth + "\r\n";
    }

    if (!_url.empty()) setUrl(_url);
  }
  catch (BaseLib::Exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void MyNode::setUrl(std::string url) {
  try {
    if (url.compare(0, 7, "http://") == 0) {
      url = url.substr(7);
    } else if (url.compare(0, 8, "https://") == 0) {
      _useTls = true;
      _verifyCertificate = true;
      url = url.substr(8);
    } else {
      _out->printError("Error: URL does not start with http:// or https://.");
      return;
    }

    if (_useTls && !_tlsNode.empty()) {
      _caData = getConfigParameter(_tlsNode, "cadata.password")->stringValue;
      _certData = getConfigParameter(_tlsNode, "certdata.password")->stringValue;
      auto keyData = getConfigParameter(_tlsNode, "keydata.password")->stringValue;
      _keyData = std::make_shared<BaseLib::Security::SecureVector<uint8_t>>();
      _keyData->insert(_keyData->end(), keyData.begin(), keyData.end());
      _caPath = getConfigParameter(_tlsNode, "ca")->stringValue;
      _certPath = getConfigParameter(_tlsNode, "cert")->stringValue;
      _keyPath = getConfigParameter(_tlsNode, "key")->stringValue;
      _verifyCertificate = getConfigParameter(_tlsNode, "verifyservercert")->booleanValue;
    }

    auto pathPair = BaseLib::HelperFunctions::splitFirst(url, '/');
    _path = '/' + pathPair.second;

    auto hostPair = BaseLib::HelperFunctions::splitLast(pathPair.first, ':');
    _hostname = hostPair.first;
    if (_hostname.size() >= 2 && _hostname.front() == '[' && _hostname.back() == ']') {
      _hostname = _hostname.substr(1, _hostname.size() - 2);
    }
    if (_hostname.empty()) {
      _out->printError("Error: URL does not contain a valid hostname or IP address.");
      return;
    }

    _port = Flows::Math::getNumber(hostPair.second);
    if (_port <= 0 || _port > 65535) _port = _useTls ? 443 : 80;

    _httpClient = std::make_unique<BaseLib::HttpClient>(_bl.get(), _hostname, _port, false, _useTls, _verifyCertificate, _caPath, _caData, _certPath, _certData, _keyPath, _keyData);
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void MyNode::input(const Flows::PNodeInfo &info, uint32_t index, const Flows::PVariable &message) {
  try {
    auto message_iterator = message->structValue->find("url");
    if (message_iterator != message->structValue->end()) {
      if (message_iterator->second->stringValue.empty()) return;
      setUrl(message_iterator->second->stringValue);
    }

    auto method = _method;
    if (method.empty()) {
      message_iterator = message->structValue->find("method");
      if (message_iterator != message->structValue->end()) {
        method = message_iterator->second->stringValue;
      }

      if (method.empty()) {
        method = "GET";
      }
    }

    if (!_httpClient) return;

    std::string request_headers;
    message_iterator = message->structValue->find("headers");
    if (message_iterator != message->structValue->end()) {
      std::ostringstream headers_stream;
      for (auto &header: *message_iterator->second->structValue) {
        headers_stream << header.first << ": " << header.second->stringValue << "\r\n";
      }
      request_headers = headers_stream.str();
    }

    std::string content;
    if (message->structValue->at("payload")->type == Flows::VariableType::tBinary) {
      content.insert(content.end(), message->structValue->at("payload")->binaryValue.begin(), message->structValue->at("payload")->binaryValue.end());
    } else {
      content = message->structValue->at("payload")->stringValue;
    }

    BaseLib::Http result;
    if (method == "GET") {
      std::string getRequest = "GET " + _path + " HTTP/1.1\r\nUser-Agent: Homegear\r\nHost: " + _hostname + ":" + std::to_string(_port) + "\r\n" + _basicAuth + request_headers + "Connection: Close" + "\r\n\r\n";
      _httpClient->sendRequest(getRequest, result);
    } else if (method == "DELETE") {
      std::string deleteRequest = "DELETE " + _path + " HTTP/1.1\r\nUser-Agent: Homegear\r\nHost: " + _hostname + ":" + std::to_string(_port) + "\r\n" + _basicAuth + request_headers + "Connection: Close" + "\r\n\r\n";
      _httpClient->sendRequest(deleteRequest, result);
    } else if (method == "PUT") {
      std::string putRequest = "PUT " + _path + " HTTP/1.1\r\nUser-Agent: Homegear\r\nHost: " + _hostname + ":" + std::to_string(_port) + "\r\n" + _basicAuth + request_headers + "Connection: Close\r\nContent-Length: " + std::to_string(content.size()) + "\r\n\r\n";
      putRequest.insert(putRequest.end(), content.begin(), content.end());
      _httpClient->sendRequest(putRequest, result);
    } else if (method == "POST") {
      std::string postRequest = "POST " + _path + " HTTP/1.1\r\nUser-Agent: Homegear\r\nHost: " + _hostname + ":" + std::to_string(_port) + "\r\n" + _basicAuth + request_headers + "Connection: Close\r\nContent-Length: " + std::to_string(content.size()) + "\r\n\r\n";
      postRequest.insert(postRequest.end(), content.begin(), content.end());
      _httpClient->sendRequest(postRequest, result);
    } else if (method == "PATCH") {
      std::string patchRequest = "PATCH " + _path + " HTTP/1.1\r\nUser-Agent: Homegear\r\nHost: " + _hostname + ":" + std::to_string(_port) + "\r\n" + _basicAuth + request_headers + "Connection: Close\r\nContent-Length: " + std::to_string(content.size()) + "\r\n\r\n";
      patchRequest.insert(patchRequest.end(), content.begin(), content.end());
      _httpClient->sendRequest(patchRequest, result);
    } else {
      return;
    }

    Flows::PVariable outputMessage = std::make_shared<Flows::Variable>();
    *outputMessage = *message;
    (*outputMessage->structValue)["statusCode"] = std::make_shared<Flows::Variable>(result.getHeader().responseCode);

    Flows::PVariable headers = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
    (*outputMessage->structValue)["headers"] = headers;
    auto &header = result.getHeader().fields;
    for (auto &field: header) {
      headers->structValue->emplace(field.first, std::make_shared<Flows::Variable>(field.second));
    }

    if (_returnType == ReturnType::bin) {
      (*outputMessage->structValue)["payload"] = std::make_shared<Flows::Variable>(result.getContent());
    } else if (_returnType == ReturnType::obj) {
      try {
        (*outputMessage->structValue)["payload"] = _jsonDecoder->decode(result.getContent());
      }
      catch (Flows::JsonDecoderException &ex) {
        (*outputMessage->structValue)["payload"] = std::make_shared<Flows::Variable>(std::string(result.getContent().data(), result.getContent().size()));
      }
    } else (*outputMessage->structValue)["payload"] = std::make_shared<Flows::Variable>(std::string(result.getContent().data(), result.getContent().size()));

    output(0, outputMessage);
  }
  catch (BaseLib::Exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

}
