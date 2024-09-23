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

#include "Mqtt.h"

namespace MyNode {

Mqtt::Mqtt(std::shared_ptr<BaseLib::SharedObjects> bl, std::shared_ptr<Flows::Output> output) : BaseLib::IQueue(bl.get(), 2, 1000) {
  try {
    _packetId = 1;

    _bl = bl;
    _out = output;
    _started = false;
    _reconnecting = false;
    _connected = false;
    C1Net::TcpSocketInfo tcp_socket_info;
    auto dummy_socket = std::make_shared<C1Net::Socket>(-1);
    _socket = std::make_unique<C1Net::TcpSocket>(tcp_socket_info, dummy_socket);
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

Mqtt::~Mqtt() {
  try {
    waitForStop();
    _bl.reset();
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void Mqtt::start() {
  try {
    if (_started) return;
    _started = true;

    startQueue(0, false, 1, 0, SCHED_OTHER);
    startQueue(1, false, 5, 0, SCHED_OTHER);

    C1Net::TcpSocketInfo tcp_socket_info;
    C1Net::TcpSocketHostInfo tcp_socket_host_info;
    tcp_socket_host_info.host = _settings->brokerHostname;
    tcp_socket_host_info.port = BaseLib::Math::getUnsignedNumber(_settings->brokerPort);
    tcp_socket_host_info.tls = _settings->enableSSL;
    tcp_socket_host_info.verify_certificate = _settings->verifyCertificate;
    tcp_socket_host_info.ca_file = _settings->caPath;
    tcp_socket_host_info.ca_data = _settings->caData;
    tcp_socket_host_info.client_cert_file = _settings->certPath;
    tcp_socket_host_info.client_cert_data = _settings->certData;
    tcp_socket_host_info.client_key_file = _settings->keyPath;
    if (_settings->keyData) tcp_socket_host_info.client_key_data = std::string(_settings->keyData->begin(), _settings->keyData->end());

    _socket = std::make_unique<C1Net::TcpSocket>(tcp_socket_info, tcp_socket_host_info);

    _bl->threadManager.start(_listenThread, true, &Mqtt::listen, this);
    _bl->threadManager.start(_pingThread, true, &Mqtt::ping, this);
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void Mqtt::stop() {
  try {
    _started = false;
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void Mqtt::waitForStop() {
  try {
    _started = false;
    stopQueue(1);
    stopQueue(0);
    disconnect();
    _bl->threadManager.join(_pingThread);
    _bl->threadManager.join(_listenThread);
    {
      std::lock_guard<std::mutex> reconnectGuard(_reconnectThreadMutex);
      _bl->threadManager.join(_reconnectThread);
    }
    _socket->Shutdown();
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void Mqtt::setSettings(const std::shared_ptr<MqttSettings> &settings) {
  if (!_settings) _settings = settings;
  else _out->printWarning("Warning: Tried to set MQTT settings even though there were already set.");
}

uint32_t Mqtt::getLength(std::vector<char> packet, uint32_t &lengthBytes) {
  // From section 2.2.3 of the MQTT specification version 3.1.1
  uint32_t multiplier = 1;
  uint32_t value = 0;
  uint32_t pos = 1;
  char encodedByte = 0;
  lengthBytes = 0;
  do {
    if (pos >= packet.size()) return 0;
    encodedByte = packet[pos];
    lengthBytes++;
    value += ((uint32_t)(encodedByte & 127)) * multiplier;
    multiplier *= 128;
    pos++;
    if (multiplier > 128 * 128 * 128) return 0;
  } while ((encodedByte & 128) != 0);
  return value;
}

std::vector<char> Mqtt::getLengthBytes(uint32_t length) {
  // From section 2.2.3 of the MQTT specification version 3.1.1
  std::vector<char> result;
  do {
    char byte = length % 128;
    length = length / 128;
    if (length > 0) byte = byte | 128;
    result.push_back(byte);
  } while (length > 0);
  return result;
}

void Mqtt::printConnectionError(char resultCode) {
  switch (resultCode) {
    case 0: //No error
      break;
    case 1: _out->printError("Error: Connection refused. Unacceptable protocol version.");
      break;
    case 2: _out->printError("Error: Connection refused. Client identifier rejected. Please change the client identifier in mqtt.conf.");
      break;
    case 3: _out->printError("Error: Connection refused. Server unavailable.");
      break;
    case 4: _out->printError("Error: Connection refused. Bad username or password.");
      break;
    case 5: _out->printError("Error: Connection refused. Unauthorized.");
      break;
    default: _out->printError("Error: Connection refused. Unknown error: " + std::to_string(resultCode));
      break;
  }
}

void Mqtt::getResponseByType(const std::vector<char> &packet, std::vector<char> &responseBuffer, uint8_t responseType, bool errors) {
  try {
    if (!_socket->Connected()) {
      if (errors) _out->printError("Error: Could not send packet to MQTT server, because we are not connected.");
      return;
    }
    std::shared_ptr<RequestByType> request(new RequestByType());
    _requestsByTypeMutex.lock();
    _requestsByType[responseType] = request;
    _requestsByTypeMutex.unlock();
    std::unique_lock<std::mutex> lock(request->mutex);
    try {
      _socket->Send((uint8_t *)packet.data(), packet.size());

      if (!request->conditionVariable.wait_for(lock, std::chrono::milliseconds(5000), [&] { return request->mutexReady; })) {
        if (errors) _out->printError("Error: No response received to packet: " + Flows::HelperFunctions::getHexString(packet));
      }
      responseBuffer = request->response;

      _requestsByTypeMutex.lock();
      _requestsByType.erase(responseType);
      _requestsByTypeMutex.unlock();
      return;
    }
    catch (const C1Net::ClosedException &) {
      if (errors) _out->printError("Error: Socket closed while sending packet.");
    }
    catch (const C1Net::TimeoutException &ex) { _socket->Shutdown(); }
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    _requestsByTypeMutex.unlock();
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    _requestsByTypeMutex.unlock();
  }
}

void Mqtt::getResponse(const std::vector<char> &packet, std::vector<char> &responseBuffer, uint8_t responseType, int16_t packetId, bool errors) {
  try {
    if (!_socket->Connected()) {
      if (errors) _out->printError("Error: Could not send packet to MQTT server, because we are not connected.");
      return;
    }
    std::shared_ptr<Request> request(new Request(responseType));
    _requestsMutex.lock();
    _requests[packetId] = request;
    _requestsMutex.unlock();
    std::unique_lock<std::mutex> lock(request->mutex);
    try {
      send(packet);

      if (!request->conditionVariable.wait_for(lock, std::chrono::milliseconds(5000), [&] { return request->mutexReady; })) {
        if (errors) _out->printError("Error: No response received to packet: " + Flows::HelperFunctions::getHexString(packet));
      }
      responseBuffer = request->response;

      _requestsMutex.lock();
      _requests.erase(packetId);
      _requestsMutex.unlock();
      return;
    }
    catch (const C1Net::ClosedException &) {
      _out->printError("Error: Socket closed while sending packet.");
    }
    catch (const C1Net::TimeoutException &ex) { _socket->Shutdown(); }
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    _requestsMutex.unlock();
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    _requestsMutex.unlock();
  }
}

void Mqtt::ping() {
  try {
    std::vector<char> ping{(char)0xC0, 0};
    std::vector<char> pong(5);
    int32_t i = 0;
    while (_started) {
      if (_connected) {
        getResponseByType(ping, pong, 0xD0, false);
        if (pong.empty()) {
          _out->printError("Error: No PINGRESP received.");
          _socket->Shutdown();
        }
      }

      i = 0;
      while (_started && i < 20) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        i++;
      }
    }
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void Mqtt::listen() {
  std::vector<char> data;
  std::vector<char> buffer(4096);
  uint32_t bytesReceived = 0;
  uint32_t length = 0;
  uint32_t dataLength = 0;
  uint32_t lengthBytes = 0;
  bool moreData = false;
  while (_started) {
    try {
      if (!_socket->Connected()) {
        if (!_started) return;
        reconnect();
        for (int32_t i = 0; i < 300; i++) {
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
          if (_socket->Connected() || !_started) break;
        }
        continue;
      }
      try {
        do {
          bytesReceived = _socket->Read((uint8_t *)buffer.data(), buffer.size(), moreData);
          if (bytesReceived > 0) {
            data.insert(data.end(), buffer.begin(), buffer.begin() + bytesReceived);
            if (data.size() > 1000000) {
              _out->printError("Could not read packet: Too much data.");
              break;
            }
          }
          if (length == 0) {
            length = getLength(data, lengthBytes);
            dataLength = length + lengthBytes + 1;
          }

          while (length > 0 && data.size() > dataLength) {
            //Multiple MQTT packets in one TCP packet
            std::vector<char> data2(data.begin(), data.begin() + dataLength);
            processData(data2);
            data2 = std::vector<char>(data.begin() + dataLength, data.begin() + dataLength + (data.size() - dataLength));
            data = std::move(data2);
            length = getLength(data, lengthBytes);
            dataLength = length + lengthBytes + 1;
          }
          if (bytesReceived == buffer.size()) {
            //Check if packet size is exactly a multiple of bufferMax
            if (data.size() == dataLength) break;
          }
        } while (bytesReceived == buffer.size() || dataLength > data.size());
      }
      catch (const C1Net::ClosedException &ex) {
        _socket->Shutdown();
        if (_started) _out->printWarning("Warning: Connection to MQTT server closed.");
        continue;
      }
      catch (const C1Net::TimeoutException &ex) {
        continue;
      }
      catch (const C1Net::Exception &ex) {
        _socket->Shutdown();
        _out->printError("Error: " + std::string(ex.what()));
        continue;
      }

      if (data.empty()) continue;
      if (data.size() > 1000000) {
        data.clear();
        length = 0;
        continue;
      }

      processData(data);
      data.clear();
      length = 0;
    }
    catch (const std::exception &ex) {
      _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch (...) {
      _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
    data.clear();
    length = 0;
  }
}

void Mqtt::processData(std::vector<char> &data) {
  try {
    int16_t id = 0;
    uint8_t type = 0;
    if (data.size() == 2 && data.at(0) == (char)0xD0 && data.at(1) == 0) type = 0xD0;
    else if (data.size() == 4 && data[0] == 0x20 && data[1] == 2 && data[2] == 0 && data[3] == 0) type = 0x20; //CONNACK
    else if (data.size() == 4 && data[0] == 0x40 && data[1] == 2)
      id = (((uint16_t)data[2]) << 8) + (uint8_t)data[3]; //PUBACK
    else if (data.size() == 5 && data[0] == (char)0x90 && data[1] == 3)
      id = (((uint16_t)data[2]) << 8) + (uint8_t)data[3]; //SUBACK
    if (type != 0) {
      _requestsByTypeMutex.lock();
      std::map<uint8_t, std::shared_ptr<RequestByType>>::iterator requestIterator = _requestsByType.find(type);
      if (requestIterator != _requestsByType.end()) {
        std::shared_ptr<RequestByType> request = requestIterator->second;
        _requestsByTypeMutex.unlock();
        request->response = data;
        {
          std::lock_guard<std::mutex> lock(request->mutex);
          request->mutexReady = true;
        }
        request->conditionVariable.notify_one();
        return;
      } else _requestsByTypeMutex.unlock();
    }
    if (id != 0) {
      _requestsMutex.lock();
      std::map<int16_t, std::shared_ptr<Request>>::iterator requestIterator = _requests.find(id);
      if (requestIterator != _requests.end()) {
        std::shared_ptr<Request> request = requestIterator->second;
        _requestsMutex.unlock();
        if (data[0] == (char)request->getResponseControlByte()) {
          request->response = data;
          {
            std::lock_guard<std::mutex> lock(request->mutex);
            request->mutexReady = true;
          }
          request->conditionVariable.notify_one();
          return;
        }
      } else _requestsMutex.unlock();
    }
    if (data.size() > 4 && (data[0] & 0xF0) == 0x30) //PUBLISH
    {
      std::shared_ptr<BaseLib::IQueueEntry> entry(new QueueEntryReceived(data));
      if (!enqueue(1, entry)) _out->printError("Error: Too many received packets are queued to be processed. Your packet processing is too slow. Dropping packet.");
    }
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void Mqtt::processPublish(std::vector<char> &data) {
  try {
    uint32_t lengthBytes = 0;
    uint32_t length = getLength(data, lengthBytes);
    if (1 + lengthBytes >= data.size() - 1 || length == 0) {
      _out->printError("Error: Invalid packet format: " + BaseLib::HelperFunctions::getHexString(data));
      return;
    }
    uint8_t qos = data[0] & 6;
    bool retain = data[0] & 1;
    uint32_t topicLength = 1 + lengthBytes + 2 + (((uint16_t)data[1 + lengthBytes])
        << 8) + (uint8_t)data[1 + lengthBytes + 1];
    uint32_t payloadPos = (qos > 0) ? topicLength + 2 : topicLength;
    if (qos == 4) {
      _out->printError("Error: Received publish packet with QoS 2. That was not requested.");
    } else if (qos == 2) {
      std::vector<char> puback{0x40, 2, data[topicLength], data[topicLength + 1]};
      send(puback);
    }
    std::string topic(data.data() + (1 + lengthBytes + 2), topicLength - (1 + lengthBytes + 2));
    std::string payload;
    if (payloadPos < data.size()) {
      payload = std::string(data.data() + payloadPos, data.size() - payloadPos);
    }

    if (!_invoke) return;

    std::lock_guard<std::mutex> topicsGuard(_topicsMutex);
    for (auto &topicIterator: _topics) {
      if (topicIterator.first == "#" || std::regex_match(topic, topicIterator.second.first)) {
        for (auto &node: topicIterator.second.second) {
          Flows::PArray parameters = std::make_shared<Flows::Array>();
          parameters->reserve(3);
          parameters->push_back(std::make_shared<Flows::Variable>(topic));
          parameters->push_back(std::make_shared<Flows::Variable>(payload));
          parameters->push_back(std::make_shared<Flows::Variable>(retain));
          _invoke(node, "publish", parameters, false);
        }
      }
    }
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void Mqtt::send(const std::vector<char> &data) {
  try {
    _socket->Send((uint8_t *)data.data(), data.size());
  }
  catch (const C1Net::ClosedException &) {
    _out->printError("Error: Socket closed while sending packet.");
  }
  catch (const C1Net::TimeoutException &ex) { _socket->Shutdown(); }
  catch (const C1Net::Exception &ex) { _socket->Shutdown(); }
}

void Mqtt::subscribe(std::string &topic) {
  try {
    if (topic.empty()) return;
    std::vector<char> payload;
    payload.reserve(200);
    int16_t id = 0;
    while (id == 0) id = _packetId++;
    payload.push_back(id >> 8);
    payload.push_back(id & 0xFF);
    payload.push_back(topic.size() >> 8);
    payload.push_back(topic.size() & 0xFF);
    payload.insert(payload.end(), topic.begin(), topic.end());
    payload.push_back(1); //QoS
    std::vector<char> lengthBytes = getLengthBytes(payload.size());
    std::vector<char> subscribePacket;
    subscribePacket.reserve(1 + lengthBytes.size() + payload.size());
    subscribePacket.push_back(0x82); //Control packet type
    subscribePacket.insert(subscribePacket.end(), lengthBytes.begin(), lengthBytes.end());
    subscribePacket.insert(subscribePacket.end(), payload.begin(), payload.end());
    for (int32_t i = 0; i < 3; i++) {
      try {
        std::vector<char> response;
        getResponse(subscribePacket, response, 0x90, id, false);
        if (response.size() == 0 || (response.at(4) != 0 && response.at(4) != 1)) {
          //Ignore => mosquitto does not send SUBACK
        } else break;
      }
      catch (const C1Net::ClosedException &) {
        _out->printError("Error: Socket closed while sending packet.");
        break;
      }
      catch (const C1Net::TimeoutException &ex) {
        _socket->Shutdown();
        break;
      }
    }
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void Mqtt::unsubscribe(std::string &topic) {
  try {
    std::vector<char> payload;
    payload.reserve(200);
    int16_t id = 0;
    while (id == 0) id = _packetId++;
    payload.push_back(id >> 8);
    payload.push_back(id & 0xFF);
    payload.push_back(topic.size() >> 8);
    payload.push_back(topic.size() & 0xFF);
    payload.insert(payload.end(), topic.begin(), topic.end());
    payload.push_back(1); //QoS
    std::vector<char> lengthBytes = getLengthBytes(payload.size());
    std::vector<char> unsubscribePacket;
    unsubscribePacket.reserve(1 + lengthBytes.size() + payload.size());
    unsubscribePacket.push_back(0xA2); //Control packet type
    unsubscribePacket.insert(unsubscribePacket.end(), lengthBytes.begin(), lengthBytes.end());
    unsubscribePacket.insert(unsubscribePacket.end(), payload.begin(), payload.end());
    for (int32_t i = 0; i < 3; i++) {
      try {
        std::vector<char> response;
        getResponse(unsubscribePacket, response, 0xB0, id, false);
        break;
      }
      catch (const C1Net::ClosedException &) {
        _out->printError("Error: Socket closed while sending packet.");
        break;
      }
      catch (const C1Net::TimeoutException &ex) {
        _socket->Shutdown();
        break;
      }
    }
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void Mqtt::reconnectThread() {
  try {
    connect();
    if (!_invoke) return;
    Flows::PArray parameters = std::make_shared<Flows::Array>();
    if (_socket->Connected()) {
      parameters->push_back(std::make_shared<Flows::Variable>(true));
      std::lock_guard<std::mutex> topicsGuard(_topicsMutex);
      for (auto &topicIterator: _topics) {
        std::string topic = topicIterator.first;
        subscribe(topic);
      }
    } else parameters->push_back(std::make_shared<Flows::Variable>(false));

    std::lock_guard<std::mutex> nodesGuard(_nodesMutex);
    for (auto &node: _nodes) {
      _invoke(node, "setConnectionState", parameters, false);
    }
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void Mqtt::reconnect() {
  if (!_started) return;
  try {
    std::lock_guard<std::mutex> reconnectThreadGuard(_reconnectThreadMutex);
    if (_reconnecting || _socket->Connected()) return;
    _reconnecting = true;
    _bl->threadManager.join(_reconnectThread);
    _bl->threadManager.start(_reconnectThread, true, &Mqtt::reconnectThread, this);
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void Mqtt::connect() {
  _reconnecting = true;
  _connectMutex.lock();
  for (int32_t i = 0; i < 5; i++) {
    try {
      if (_socket->Connected() || !_started) {
        _connectMutex.unlock();
        _reconnecting = false;
        return;
      }
      _connected = false;
      _socket->Open();
      std::vector<char> payload;
      payload.reserve(200);
      payload.push_back(0); //String size MSB
      payload.push_back(4); //String size LSB
      payload.push_back('M');
      payload.push_back('Q');
      payload.push_back('T');
      payload.push_back('T');
      payload.push_back(4); //Protocol level
      payload.push_back(2); //Connect flags (Clean session)
      if (!_settings->username.empty()) payload.at(7) |= 0x80;
      if (!_settings->password.empty()) payload.at(7) |= 0x40;
      payload.push_back(0); //Keep alive MSB (in seconds)
      payload.push_back(0x3C); //Keep alive LSB
      std::string temp = _settings->clientId;
      if (temp.empty()) temp = "HomegearNode";
      payload.push_back(temp.size() >> 8);
      payload.push_back(temp.size() & 0xFF);
      payload.insert(payload.end(), temp.begin(), temp.end());
      if (!_settings->username.empty()) {
        temp = _settings->username;
        payload.push_back(temp.size() >> 8);
        payload.push_back(temp.size() & 0xFF);
        payload.insert(payload.end(), temp.begin(), temp.end());
      }
      if (!_settings->password.empty()) {
        temp = _settings->password;
        payload.push_back(temp.size() >> 8);
        payload.push_back(temp.size() & 0xFF);
        payload.insert(payload.end(), temp.begin(), temp.end());
      }
      std::vector<char> lengthBytes = getLengthBytes(payload.size());
      std::vector<char> connectPacket;
      connectPacket.reserve(1 + lengthBytes.size() + payload.size());
      connectPacket.push_back(0x10); //Control packet type
      connectPacket.insert(connectPacket.end(), lengthBytes.begin(), lengthBytes.end());
      connectPacket.insert(connectPacket.end(), payload.begin(), payload.end());
      std::vector<char> response(10);
      getResponseByType(connectPacket, response, 0x20, false);
      bool retry = false;
      if (response.size() != 4) {
        if (response.size() == 0) {}
        else if (response.size() != 4) _out->printError("Error: CONNACK packet has wrong size.");
        else if (response[0] != 0x20 || response[1] != 0x02 || response[2] != 0) _out->printError("Error: CONNACK has wrong content.");
        else if (response[3] != 1) printConnectionError(response[3]);
        retry = true;
      } else {
        _out->printInfo("Info: Successfully connected to MQTT server using protocol version 4.");
        _connected = true;
        _connectMutex.unlock();
        _reconnecting = false;
        return;
      }
      if (retry && _started) {
        _socket->Open();
        payload.clear();
        payload.reserve(200);
        payload.push_back(0); //String size MSB
        payload.push_back(6); //String size LSB
        payload.push_back('M');
        payload.push_back('Q');
        payload.push_back('I');
        payload.push_back('s');
        payload.push_back('d');
        payload.push_back('p');
        payload.push_back(3); //Protocol level
        payload.push_back(2); //Connect flags (Clean session)
        if (!_settings->username.empty()) payload.at(7) |= 0x80;
        if (!_settings->password.empty()) payload.at(7) |= 0x40;
        payload.push_back(0); //Keep alive MSB (in seconds)
        payload.push_back(0x3C); //Keep alive LSB
        temp = _settings->clientId;
        if (temp.empty()) temp = "HomegearNode";
        payload.push_back(temp.size() >> 8);
        payload.push_back(temp.size() & 0xFF);
        payload.insert(payload.end(), temp.begin(), temp.end());
        if (!_settings->username.empty()) {
          temp = _settings->username;
          payload.push_back(temp.size() >> 8);
          payload.push_back(temp.size() & 0xFF);
          payload.insert(payload.end(), temp.begin(), temp.end());
        }
        if (!_settings->password.empty()) {
          temp = _settings->password;
          payload.push_back(temp.size() >> 8);
          payload.push_back(temp.size() & 0xFF);
          payload.insert(payload.end(), temp.begin(), temp.end());
        }
        std::vector<char> lengthBytes = getLengthBytes(payload.size());
        std::vector<char> connectPacket;
        connectPacket.reserve(1 + lengthBytes.size() + payload.size());
        connectPacket.push_back(0x10); //Control packet type
        connectPacket.insert(connectPacket.end(), lengthBytes.begin(), lengthBytes.end());
        connectPacket.insert(connectPacket.end(), payload.begin(), payload.end());
        getResponseByType(connectPacket, response, 0x20, false);
        if (response.size() != 4) {
          if (response.size() == 0) _out->printError("Error: Connection to MQTT server with protocol version 3 failed.");
          else if (response.size() != 4) _out->printError("Error: CONNACK packet has wrong size.");
          else if (response[0] != 0x20 || response[1] != 0x02 || response[2] != 0) _out->printError("Error: CONNACK has wrong content.");
          else printConnectionError(response[3]);
        } else {
          _out->printInfo("Info: Successfully connected to MQTT server using protocol version 3.");
          _connected = true;
          _connectMutex.unlock();
          _reconnecting = false;
          return;
        }
      }

    }
    catch (const std::exception &ex) {
      _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch (...) {
      _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
  }
  _connectMutex.unlock();
  _reconnecting = false;
}

void Mqtt::disconnect() {
  try {
    _connected = false;
    std::vector<char> disconnect = {(char)0xE0, 0};
    if (_socket->Connected()) _socket->Send((uint8_t *)disconnect.data(), disconnect.size());
    _socket->Shutdown();
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

std::string &Mqtt::escapeTopic(std::string &topic) {
  if (topic.empty() || topic == "#") return topic;
  BaseLib::HelperFunctions::stringReplace(topic, "[", "\\[");
  BaseLib::HelperFunctions::stringReplace(topic, "]", "\\]");
  BaseLib::HelperFunctions::stringReplace(topic, "?", "\\?");
  BaseLib::HelperFunctions::stringReplace(topic, "(", "\\(");
  BaseLib::HelperFunctions::stringReplace(topic, ")", "\\)");
  BaseLib::HelperFunctions::stringReplace(topic, "\\", "\\\\");
  BaseLib::HelperFunctions::stringReplace(topic, "/", "\\/");
  BaseLib::HelperFunctions::stringReplace(topic, "$", "\\$");
  BaseLib::HelperFunctions::stringReplace(topic, "^", "\\^");
  BaseLib::HelperFunctions::stringReplace(topic, "*", "\\*");
  BaseLib::HelperFunctions::stringReplace(topic, ".", "\\.");
  BaseLib::HelperFunctions::stringReplace(topic, "|", "\\|");
  BaseLib::HelperFunctions::stringReplace(topic, "+", "[^\\/]+");
  if (topic.back() == '#') topic = topic.substr(0, topic.length() - 1) + ".*";
  topic = "^" + topic + "$";
  return topic;
}

void Mqtt::registerNode(std::string &node) {
  try {
    std::lock_guard<std::mutex> nodesGuard(_nodesMutex);
    _nodes.emplace(node);

    Flows::PArray parameters = std::make_shared<Flows::Array>();
    parameters->push_back(std::make_shared<Flows::Variable>(_socket && _socket->Connected()));
    _invoke(node, "setConnectionState", parameters, false);
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void Mqtt::registerTopic(std::string &node, std::string &topic) {
  try {
    BaseLib::HelperFunctions::trim(topic);
    std::lock_guard<std::mutex> topicsGuard(_topicsMutex);
    std::string escapedTopic = topic;
    escapeTopic(escapedTopic);
    if (_topics.find(topic) == _topics.end()) {
      subscribe(topic);
      _topics[topic].first = std::regex(escapedTopic);
    }
    _topics[topic].second.emplace(node);
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void Mqtt::unregisterTopic(std::string &node, std::string &topic) {
  try {
    BaseLib::HelperFunctions::trim(topic);
    std::lock_guard<std::mutex> topicsGuard(_topicsMutex);
    auto topicIterator = _topics.find(topic);
    if (topicIterator == _topics.end()) return;
    topicIterator->second.second.erase(node);
    if (topicIterator->second.second.empty()) {
      _topics.erase(topicIterator);
      unsubscribe(topic);
    }
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void Mqtt::queueMessage(std::string &topic, std::string &payload, bool retain) {
  try {
    if (!_started) return;
    std::shared_ptr<MqttMessage> message = std::make_shared<MqttMessage>();
    message->topic = std::move(topic);
    message->message.insert(message->message.end(), payload.begin(), payload.end());
    message->retain = retain;
    std::shared_ptr<BaseLib::IQueueEntry> entry = std::make_shared<QueueEntrySend>(message);
    if (!enqueue(0, entry)) _out->printError("Error: Too many packets are queued to be processed. Your packet processing is too slow. Dropping packet.");
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void Mqtt::publish(const std::string &topic, const std::vector<char> &data, bool retain) {
  try {
    if (topic.empty() || data.empty() || !_started) return;
    std::vector<char> packet;
    std::vector<char> payload;
    payload.reserve(topic.size() + 2 + 2 + data.size());
    payload.push_back(topic.size() >> 8);
    payload.push_back(topic.size() & 0xFF);
    payload.insert(payload.end(), topic.begin(), topic.end());
    int16_t id = 0;
    while (id == 0) id = _packetId++;
    payload.push_back(id >> 8);
    payload.push_back(id & 0xFF);
    payload.insert(payload.end(), data.begin(), data.end());
    std::vector<char> lengthBytes = getLengthBytes(payload.size());
    packet.reserve(1 + lengthBytes.size() + payload.size());
    retain ? packet.push_back(0x33) : packet.push_back(0x32);
    packet.insert(packet.end(), lengthBytes.begin(), lengthBytes.end());
    packet.insert(packet.end(), payload.begin(), payload.end());
    int32_t j = 0;
    std::vector<char> response(7);
    _out->printInfo("Info: Publishing topic " + topic);
    for (int32_t i = 0; i < 25; i++) {
      if (_reconnecting) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        if (!_started) return;
        continue;
      }
      if (!_socket->Connected()) reconnect();
      if (!_started) break;
      if (i == 1) packet[0] |= 8;
      getResponse(packet, response, 0x40, id, true);
      if (response.empty()) {
        //_socket->close();
        //reconnect();
        if (i >= 5) _out->printWarning("Warning: No PUBACK received.");
      } else return;

      j = 0;
      while (_started && j < 5) {

        if (i < 5) {
          j += 5;
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
        } else {
          j++;
          std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
      }
    }
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void Mqtt::processQueueEntry(int32_t index, std::shared_ptr<BaseLib::IQueueEntry> &entry) {
  try {
    if (index == 0) //Send
    {
      std::shared_ptr<QueueEntrySend> queueEntry;
      queueEntry = std::dynamic_pointer_cast<QueueEntrySend>(entry);
      if (!queueEntry || !queueEntry->message) return;
      publish(queueEntry->message->topic, queueEntry->message->message, queueEntry->message->retain);
    } else {
      std::shared_ptr<QueueEntryReceived> queueEntry;
      queueEntry = std::dynamic_pointer_cast<QueueEntryReceived>(entry);
      if (!queueEntry) return;
      processPublish(queueEntry->data);
    }
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

}