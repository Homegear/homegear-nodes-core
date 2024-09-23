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

#include <regex>

namespace MyNode {

class MyNode : public Flows::INode {
 public:
  MyNode(const std::string &path, const std::string &type, const std::atomic_bool *frontendConnected);
  ~MyNode() override;

  bool init(const Flows::PNodeInfo &info) override;
  bool start() override;
  void stop() override;
  void waitForStop() override;

  Flows::PVariable getConfigParameterIncoming(const std::string &name) override;
 private:
  struct NodeInfo {
    std::string id;
    std::regex pathRegex;
    std::unordered_map<int32_t, std::string> paramsMap;
  };

  std::shared_ptr<BaseLib::SharedObjects> _bl;
  Flows::PNodeInfo _nodeInfo;
  std::unique_ptr<BaseLib::HttpServer> _socket;
  std::string _username;
  std::string _password;
  BaseLib::Http _http;

  std::mutex _nodesMutex;
  std::unordered_map<std::string, std::unordered_map<std::string, NodeInfo>> _nodes;

  C1Net::TcpPacket _authRequiredHeader;

  std::string &createPathRegex(std::string &path, std::unordered_map<int32_t, std::string> &paramsMap);

  C1Net::TcpPacket getError(int32_t code, const std::string& longDescription);
  std::string constructHeader(uint32_t contentLength, int32_t code, const Flows::PVariable& headers);
  void packetReceived(int32_t clientId, BaseLib::Http http);

  //{{{ RPC methods
  Flows::PVariable send(const Flows::PArray& parameters);
  Flows::PVariable registerNode(const Flows::PArray& parameters);
  //}}}
};

}

#endif
