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

#ifndef MYNODE_H_
#define MYNODE_H_

#include <homegear-node/INode.h>
#include <homegear-base/BaseLib.h>
#include <c1-net/TcpServer.h>
#include <c1-net/TcpClient.h>

#include <regex>

namespace TcpSocket {

class TcpSocket : public Flows::INode {
 public:
  TcpSocket(const std::string &path, const std::string &type, const std::atomic_bool *frontendConnected);
  ~TcpSocket() override;

  bool init(const Flows::PNodeInfo &info) override;
  bool start() override;
  void stop() override;
  void waitForStop() override;

  Flows::PVariable getConfigParameterIncoming(const std::string &name) override;
 private:
  enum class SocketType {
    kUndefined,
    kClient,
    kServer
  };

  enum class NodeType {
    kUndefined = 0,
    kInput = 1,
    kOutput = 2
  };

  std::shared_ptr<BaseLib::SharedObjects> _bl;
  Flows::PNodeInfo _nodeInfo;
  SocketType _type = SocketType::kUndefined;

  std::mutex _nodesMutex;
  std::unordered_map<std::string, NodeType> _nodes;

  C1Net::PTcpServer tcp_server_;
  C1Net::PTcpClient tcp_client_;

  void setConnectionState(bool value);
  void log(uint32_t log_level, const std::string &message);
  void packetReceivedServer(const C1Net::TcpServer::PTcpClientData &client_data, const C1Net::TcpPacket &packet);
  void packetReceivedClient(const C1Net::TcpPacket &packet);

  //{{{ RPC methods
  Flows::PVariable send(const Flows::PArray &parameters);
  Flows::PVariable registerNode(const Flows::PArray &parameters);
  //}}}
};

}

#endif
