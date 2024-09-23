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

#include "UdpOut.h"

#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <netdb.h>
#include <homegear-base/HelperFunctions/HelperFunctions.h>
#include <homegear-node/JsonEncoder.h>

namespace UdpOut {

UdpOut::UdpOut(const std::string &path, const std::string &type, const std::atomic_bool *frontendConnected) : Flows::INode(path, type, frontendConnected) {
}

bool UdpOut::init(const Flows::PNodeInfo &info) {
  try {
    auto settingsIterator = info->info->structValue->find("hostname");
    if (settingsIterator != info->info->structValue->end()) _hostname = settingsIterator->second->stringValue;

    settingsIterator = info->info->structValue->find("port");
    if (settingsIterator != info->info->structValue->end()) _port = (uint16_t)Flows::Math::getUnsignedNumber(settingsIterator->second->stringValue);

    _payloadType = PayloadType::hex;
    settingsIterator = info->info->structValue->find("input");
    if (settingsIterator != info->info->structValue->end()) {
      if (settingsIterator->second->stringValue == "raw") _payloadType = PayloadType::raw;
      else if (settingsIterator->second->stringValue == "json") _payloadType = PayloadType::json;
    }

    if (_hostname.empty() || _port == 0) {
      _out->printError("Error: Hostname or port are invalid.");
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

void UdpOut::input(const Flows::PNodeInfo &info, uint32_t index, const Flows::PVariable &message) {
  struct addrinfo *serverInfo = nullptr;
  int socketDescriptor = -1;
  try {
    if (_hostname.empty() || _port == 0) {
      _out->printError("Error: Hostname or port are invalid.");
      return;
    }

    std::vector<uint8_t> data;
    if (_payloadType == PayloadType::hex) data = BaseLib::HelperFunctions::getUBinary(message->structValue->at("payload")->stringValue);
    else if (_payloadType == PayloadType::json) {
      Flows::JsonEncoder jsonEncoder;
      auto json = jsonEncoder.getString(message->structValue->at("payload"));
      data.insert(data.end(), json.begin(), json.end());
    } else {
      if (message->structValue->at("payload")->type == Flows::VariableType::tBinary) data = message->structValue->at("payload")->binaryValue;
      else data.insert(data.end(), message->structValue->at("payload")->stringValue.begin(), message->structValue->at("payload")->stringValue.end());
    }

    struct addrinfo hostInfo{};
    hostInfo.ai_family = AF_UNSPEC;
    hostInfo.ai_socktype = SOCK_DGRAM;
    std::string portString = std::to_string(_port);
    if (getaddrinfo(_hostname.c_str(), portString.c_str(), &hostInfo, &serverInfo) != 0) {
      freeaddrinfo(serverInfo);
      serverInfo = nullptr;
      _out->printError("Error: Could not get address information. Is the specified IP address correct?");
      return;
    }

    char ipStringBuffer[INET6_ADDRSTRLEN + 1];
    if (serverInfo->ai_family == AF_INET) {
      auto *s = (struct sockaddr_in *)serverInfo->ai_addr;
      inet_ntop(AF_INET, &s->sin_addr, ipStringBuffer, sizeof(ipStringBuffer));
    } else { // AF_INET6
      auto *s = (struct sockaddr_in6 *)serverInfo->ai_addr;
      inet_ntop(AF_INET6, &s->sin6_addr, ipStringBuffer, sizeof(ipStringBuffer));
    }
    ipStringBuffer[INET6_ADDRSTRLEN] = '\0';
    auto clientIpAddress = std::string(ipStringBuffer);

    socketDescriptor = socket(serverInfo->ai_family, SOCK_DGRAM | SOCK_CLOEXEC, 0);
    if (socketDescriptor == -1) {
      freeaddrinfo(serverInfo);
      _out->printError("Error: Could not create socket.");
      return;
    }

    int32_t totalBytesWritten = 0;
    while (totalBytesWritten < (signed)data.size()) {
      ssize_t bytesWritten = 0;
      if (serverInfo->ai_family == AF_INET) {
        struct sockaddr_in serverAddress{};
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(_port);
        serverAddress.sin_addr.s_addr = ((struct sockaddr_in *)serverInfo->ai_addr)->sin_addr.s_addr;

        bytesWritten = sendto(socketDescriptor, data.data() + totalBytesWritten, data.size() - totalBytesWritten, 0, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
      } else {
        struct sockaddr_in6 serverAddress{};
        serverAddress.sin6_family = AF_INET6;
        serverAddress.sin6_port = htons(_port);
        serverAddress.sin6_addr = ((struct sockaddr_in6 *)serverInfo->ai_addr)->sin6_addr;

        bytesWritten = sendto(socketDescriptor, data.data() + totalBytesWritten, data.size() - totalBytesWritten, 0, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
      }
      if (bytesWritten <= 0) {
        if (bytesWritten == -1 && (errno == EINTR || errno == EAGAIN)) continue;
        close(socketDescriptor);
        freeaddrinfo(serverInfo);
        _out->printError(std::string("Error sending packet to client: ") + strerror(errno));
        return;
      }
      totalBytesWritten += bytesWritten;
    }
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  close(socketDescriptor);
  freeaddrinfo(serverInfo);
}

}
