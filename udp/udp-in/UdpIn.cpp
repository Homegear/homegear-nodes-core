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

#include "UdpIn.h"

#include <homegear-base/HelperFunctions/Net.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <netdb.h>
#include <homegear-node/JsonDecoder.h>

namespace UdpIn {

UdpIn::UdpIn(const std::string &path, const std::string &type, const std::atomic_bool *frontendConnected) : Flows::INode(path, type, frontendConnected) {
}

UdpIn::~UdpIn() = default;

bool UdpIn::init(const Flows::PNodeInfo &info) {
  try {
    _nodeInfo = info;
    return true;
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return false;
}

bool UdpIn::start() {
  std::string listenAddress;
  uint16_t port = 0;

  auto settingsIterator = _nodeInfo->info->structValue->find("listenaddress");
  if (settingsIterator != _nodeInfo->info->structValue->end()) listenAddress = settingsIterator->second->stringValue;

  if (!listenAddress.empty() && !BaseLib::Net::isIp(listenAddress)) listenAddress = BaseLib::Net::getMyIpAddress(listenAddress);
  else if (listenAddress.empty()) listenAddress = BaseLib::Net::getMyIpAddress();

  settingsIterator = _nodeInfo->info->structValue->find("listenport");
  if (settingsIterator != _nodeInfo->info->structValue->end()) port = (uint16_t)Flows::Math::getUnsignedNumber(settingsIterator->second->stringValue);

  PayloadType payloadType = PayloadType::hex;
  settingsIterator = _nodeInfo->info->structValue->find("output");
  if (settingsIterator != _nodeInfo->info->structValue->end()) {
    if (settingsIterator->second->stringValue == "raw") payloadType = PayloadType::raw;
    else if (settingsIterator->second->stringValue == "json") payloadType = PayloadType::json;
  }

  _stopListenThread = true;
  if (_listenThread.joinable()) _listenThread.join();
  _stopListenThread = false;
  _listenThread = std::thread(&UdpIn::listen, this, listenAddress, port, payloadType);

  return true;
}

void UdpIn::stop() {
  _stopListenThread = true;
}

void UdpIn::waitForStop() {
  if (_listenThread.joinable()) _listenThread.join();
}

int UdpIn::getSocketDescriptor(const std::string &listenAddress, uint16_t port) {
  struct addrinfo *serverInfo = nullptr;
  int socketDescriptor = -1;
  try {
    struct addrinfo hostInfo{};
    hostInfo.ai_family = AF_UNSPEC;
    hostInfo.ai_socktype = SOCK_DGRAM;
    std::string portString = std::to_string(port);
    if (getaddrinfo(listenAddress.c_str(), portString.c_str(), &hostInfo, &serverInfo) != 0) {
      freeaddrinfo(serverInfo);
      serverInfo = nullptr;
      _out->printError("Error: Could not get address information. Is the specified IP address correct?");
      return -1;
    }

    socketDescriptor = socket(serverInfo->ai_family, SOCK_DGRAM | SOCK_CLOEXEC, 0);
    if (socketDescriptor == -1) {
      _out->printError("Error: Could not create socket.");
      freeaddrinfo(serverInfo);
      return -1;
    }

    if (!(fcntl(socketDescriptor, F_GETFL) & O_NONBLOCK)) {
      if (fcntl(socketDescriptor, F_SETFL, fcntl(socketDescriptor, F_GETFL) | O_NONBLOCK) < 0) {
        freeaddrinfo(serverInfo);
        serverInfo = nullptr;
        close(socketDescriptor);
        _out->printError("Error: Could not set non blocking mode.");
        return -1;
      }
    }

    if (serverInfo->ai_family == AF_INET) {
      struct sockaddr_in localSock{};
      localSock.sin_family = serverInfo->ai_family;
      localSock.sin_port = htons(port);
      localSock.sin_addr.s_addr = ((struct sockaddr_in *)serverInfo->ai_addr)->sin_addr.s_addr;

      if (bind(socketDescriptor, (struct sockaddr *)&localSock, sizeof(localSock)) == -1) {
        _out->printError("Error: Binding to address " + listenAddress + " failed: " + std::string(strerror(errno)));
        close(socketDescriptor);
        freeaddrinfo(serverInfo);
        return -1;
      }

      uint32_t localSockSize = sizeof(localSock);
      if (getsockname(socketDescriptor, (struct sockaddr *)&localSock, &localSockSize) == -1) {
        freeaddrinfo(serverInfo);
        serverInfo = nullptr;
        close(socketDescriptor);
        _out->printError("Error: Could not get listen IP and/or port.");
        return -1;
      }

      auto *s = (struct sockaddr_in *)&(localSock.sin_addr);
      char ipStringBuffer[INET6_ADDRSTRLEN + 1];
      inet_ntop(AF_INET, s, ipStringBuffer, sizeof(ipStringBuffer));
      ipStringBuffer[INET6_ADDRSTRLEN] = '\0';
      _out->printInfo("Info: Now listening on IP " + std::string(ipStringBuffer) + " and port " + portString + ".");
    } else //AF_INET6
    {
      struct sockaddr_in6 localSock{};
      localSock.sin6_family = serverInfo->ai_family;
      localSock.sin6_port = htons(port);
      localSock.sin6_addr = ((struct sockaddr_in6 *)serverInfo->ai_addr)->sin6_addr;

      if (bind(socketDescriptor, (struct sockaddr *)&localSock, sizeof(localSock)) == -1) {
        _out->printError("Error: Binding to address " + listenAddress + " failed: " + std::string(strerror(errno)));
        close(socketDescriptor);
        freeaddrinfo(serverInfo);
        return -1;
      }

      uint32_t localSockSize = sizeof(localSock);
      if (getsockname(socketDescriptor, (struct sockaddr *)&localSock, &localSockSize) == -1) {
        freeaddrinfo(serverInfo);
        serverInfo = nullptr;
        close(socketDescriptor);
        _out->printError("Error: Could not get listen IP and/or port.");
        return -1;
      }

      auto *s = (struct sockaddr_in6 *)&(localSock.sin6_addr);
      char ipStringBuffer[INET6_ADDRSTRLEN + 1];
      inet_ntop(AF_INET6, s, ipStringBuffer, sizeof(ipStringBuffer));
      ipStringBuffer[INET6_ADDRSTRLEN] = '\0';
      _out->printInfo("Info: Now listening on IP " + std::string(ipStringBuffer) + " and port " + portString + ".");
    }

    freeaddrinfo(serverInfo);
    return socketDescriptor;
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  close(socketDescriptor);
  freeaddrinfo(serverInfo);
  return -1;
}

void UdpIn::listen(std::string listenAddress, uint16_t port, PayloadType payloadType) {
  try {
    int socketDescriptor = -1;
    std::array<uint8_t, 4096> buffer{};
    while (!_stopListenThread) {
      if (socketDescriptor == -1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        socketDescriptor = getSocketDescriptor(listenAddress, port);
        continue;
      }

      timeval timeout{};
      timeout.tv_sec = 1;
      timeout.tv_usec = 0;

      int32_t nfds = socketDescriptor + 1;
      fd_set readFileDescriptor;
      FD_ZERO(&readFileDescriptor);
      FD_SET(socketDescriptor, &readFileDescriptor);

      auto result = select(nfds, &readFileDescriptor, nullptr, nullptr, &timeout);
      if (result == 0) continue;
      else if (result != 1) {
        close(socketDescriptor);
        socketDescriptor = -1;
        continue;
      }

      struct sockaddr clientInfo{};
      uint32_t addressLength = sizeof(sockaddr);
      ssize_t bytesRead = 0;
      do {
        bytesRead = recvfrom(socketDescriptor, buffer.data(), buffer.size(), 0, &clientInfo, &addressLength);
      } while (bytesRead < 0 && (errno == EAGAIN || errno == EINTR));

      if (bytesRead <= 0) {
        close(socketDescriptor);
        socketDescriptor = -1;
        continue;
      }

      std::array<char, INET6_ADDRSTRLEN + 1> ipStringBuffer{};
      if (clientInfo.sa_family == AF_INET) {
        auto *s = (struct sockaddr_in *)&clientInfo;
        inet_ntop(AF_INET, &s->sin_addr, ipStringBuffer.data(), ipStringBuffer.size());
      } else { // AF_INET6
        auto *s = (struct sockaddr_in6 *)&clientInfo;
        inet_ntop(AF_INET6, &s->sin6_addr, ipStringBuffer.data(), ipStringBuffer.size());
      }
      ipStringBuffer.back() = 0;
      auto senderIp = std::string(ipStringBuffer.data());

      Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
      message->structValue->emplace("senderIp", std::make_shared<Flows::Variable>(senderIp));
      if (payloadType == PayloadType::raw) message->structValue->emplace("payload", std::make_shared<Flows::Variable>(buffer.data(), bytesRead));
      else if (payloadType == PayloadType::hex) message->structValue->emplace("payload", std::make_shared<Flows::Variable>(Flows::HelperFunctions::getHexString(buffer.data(), bytesRead)));
      else if (payloadType == PayloadType::json) message->structValue->emplace("payload", Flows::JsonDecoder::decode(std::string(buffer.begin(), buffer.begin() + bytesRead)));
      output(0, message);
    }

    close(socketDescriptor);
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

}
