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

namespace MyNode {

class MyNode : public Flows::INode {
 public:
  MyNode(const std::string &path, const std::string &nodeNamespace, const std::string &type, const std::atomic_bool *frontendConnected);
  ~MyNode() override;

  bool init(const Flows::PNodeInfo &info) override;
  bool start() override;
  void stop() override;
  void waitForStop() override;

  Flows::PVariable getConfigParameterIncoming(const std::string &name) override;
 private:
  enum class SplitType {
    no,
    character,
    timeout,
    fixedLength
  };

  Flows::PNodeInfo _nodeInfo;

  std::mutex _nodesMutex;
  std::set<std::string> _nodes;

  std::shared_ptr<BaseLib::SharedObjects> _bl;
  std::shared_ptr<BaseLib::SerialReaderWriter> _serial;
  std::atomic_bool _stopThread{true};
  std::thread _readThread;

  //{{{ Settings
  std::string _serialPort;
  int32_t _baudRate = 57600;
  BaseLib::SerialReaderWriter::CharacterSize _dataBits = BaseLib::SerialReaderWriter::CharacterSize::Eight;
  bool _evenParity = false;
  bool _oddParity = false;
  int32_t _stopBits = 1;
  char _newLine = '\n';
  int32_t _timeout = 0;
  int32_t _fixedCount = 1;
  bool _binaryOutput = true;
  SplitType _splitType = SplitType::no;
  bool _addCharacter = false;
  //}}}

  void listenThread();
  void reopen();
  void packetReceived(const Flows::PVariable& data);

  //{{{ RPC methods
  Flows::PVariable registerNode(const Flows::PArray& parameters);
  Flows::PVariable write(const Flows::PArray& parameters);
  //}}}
};

}

#endif
