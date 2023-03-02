/* Copyright 2013-2019 Homegear GmbH
 *
 * Homegear is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Homegear is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Homegear.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU Lesser General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
*/

#ifndef MQTT_H_
#define MQTT_H_

#include <homegear-node/Variable.h>
#include <homegear-node/Output.h>
#include <homegear-node/HelperFunctions.h>
#include <homegear-base/Security/SecureVector.h>
#include <homegear-base/BaseLib.h>
#include <regex>

namespace MyNode {

class Mqtt : public BaseLib::IQueue {
 public:
  typedef std::string NodeId;
  typedef std::string Topic;
  typedef std::regex TopicRegex;

  struct MqttSettings {
    std::string brokerHostname;
    std::string brokerPort;
    std::string clientId;
    std::string username;
    std::string password;
    bool enableSSL = false;
    std::string caPath;
    std::string caData;
    std::string certPath;
    std::string certData;
    std::string keyPath;
    std::shared_ptr<BaseLib::Security::SecureVector<uint8_t>> keyData;
    bool verifyCertificate = true;
  };

  struct TopicEntry {

  };

  struct MqttMessage {
    std::string topic;
    std::vector<char> message;
    bool retain = true;
  };

  Mqtt(std::shared_ptr<BaseLib::SharedObjects> bl, std::shared_ptr<Flows::Output> output);

  ~Mqtt() override;

  void start();

  void stop();

  void waitForStop();

  void setSettings(const std::shared_ptr<MqttSettings> &settings);

  void setInvoke(std::function<Flows::PVariable(std::string, std::string, Flows::PArray &, bool)> value) { _invoke.swap(value); }

  void registerNode(std::string &node);

  void registerTopic(std::string &node, std::string &topic);

  void unregisterTopic(std::string &node, std::string &topic);

  /**
   * Queues a generic message for publishing to the MQTT broker.
   *
   * @param topic The MQTT topic.
   * @param payload The MQTT payload.
   */
  void queueMessage(std::string &topic, std::string &payload, bool retain);

 private:
  class QueueEntrySend : public BaseLib::IQueueEntry {
   public:
    QueueEntrySend() = default;

    explicit QueueEntrySend(std::shared_ptr<MqttMessage> &message) { this->message = message; }

    ~QueueEntrySend() override = default;

    std::shared_ptr<MqttMessage> message;
  };

  class QueueEntryReceived : public BaseLib::IQueueEntry {
   public:
    QueueEntryReceived() = default;

    explicit QueueEntryReceived(std::vector<char> &data) { this->data = data; }

    ~QueueEntryReceived() override = default;

    std::vector<char> data;
  };

  class Request {
   public:
    std::mutex mutex;
    std::condition_variable conditionVariable;
    bool mutexReady = false;
    std::vector<char> response;

    uint8_t getResponseControlByte() const { return _responseControlByte; }

    explicit Request(uint8_t responseControlByte) { _responseControlByte = responseControlByte; };

    ~Request() = default;
   private:
    uint8_t _responseControlByte;
  };

  class RequestByType {
   public:
    std::mutex mutex;
    std::condition_variable conditionVariable;
    bool mutexReady = false;
    std::vector<char> response;

    RequestByType() = default;

    ~RequestByType() = default;
  };

  std::shared_ptr<BaseLib::SharedObjects> _bl;
  std::shared_ptr<Flows::Output> _out;
  std::shared_ptr<MqttSettings> _settings;
  std::function<Flows::PVariable(std::string, std::string, Flows::PArray &, bool)> _invoke;
  std::mutex _topicsMutex;
  std::unordered_map<Topic, std::pair<TopicRegex, std::set<NodeId>>> _topics;
  std::mutex _nodesMutex;
  std::set<std::string> _nodes;
  std::unique_ptr<C1Net::TcpSocket> _socket;
  std::thread _pingThread;
  std::thread _listenThread;
  std::atomic_bool _reconnecting;
  std::mutex _reconnectThreadMutex;
  std::thread _reconnectThread;
  std::mutex _connectMutex;
  std::atomic_bool _started;
  std::atomic_bool _connected;
  std::atomic<int16_t> _packetId;
  std::mutex _requestsMutex;
  std::map<int16_t, std::shared_ptr<Request>> _requests;
  std::mutex _requestsByTypeMutex;
  std::map<uint8_t, std::shared_ptr<RequestByType>> _requestsByType;

  Mqtt(const Mqtt &);

  Mqtt &operator=(const Mqtt &);

  void connect();

  void reconnect();

  void reconnectThread();

  void disconnect();

  void processQueueEntry(int32_t index, std::shared_ptr<BaseLib::IQueueEntry> &entry) override;

  std::vector<char> getLengthBytes(uint32_t length);

  uint32_t getLength(std::vector<char> packet, uint32_t &lengthBytes);

  void printConnectionError(char resultCode);

  /**
   * Publishes data to the MQTT broker.
   *
   * @param topic The topic without Homegear prefix ("/homegear/UNIQUEID/") and without starting "/" (e.g. c/d).
   * @param data The data to publish.
   */
  void publish(const std::string &topic, const std::vector<char> &data, bool retain);

  void ping();

  void getResponseByType(const std::vector<char> &packet, std::vector<char> &responseBuffer, uint8_t responseType, bool errors = true);

  void getResponse(const std::vector<char> &packet, std::vector<char> &responseBuffer, uint8_t responseType, int16_t packetId, bool errors = true);

  void listen();

  void processData(std::vector<char> &data);

  void processPublish(std::vector<char> &data);

  void subscribe(std::string &topic);

  void unsubscribe(std::string &topic);

  void send(const std::vector<char> &data);

  std::string &escapeTopic(std::string &topic);
};

}

#endif
