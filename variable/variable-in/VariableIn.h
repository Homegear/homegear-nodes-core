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

namespace VariableIn {

class VariableIn : public Flows::INode {
 public:
  VariableIn(const std::string &path, const std::string &nodeNamespace, const std::string &type, const std::atomic_bool *frontendConnected);
  ~VariableIn() override;

  bool init(const Flows::PNodeInfo &info) override;
  bool start() override;
  void startUpComplete() override;
  void stop() override;

 private:
  enum class VariableType {
    device,
    metadata,
    system,
    flow,
    global
  };

  enum class EventSource {
    all,
    device,
    homegear,
    scriptEngine,
    profileManager,
    nodeBlue,
    rpcClient,
    ipcClient,
    mqtt
  };

  VariableType _variableType = VariableType::device;
  int64_t _lastInput = 0;
  uint32_t _refractionPeriod = 0;
  Flows::PVariable _lastValue;
  bool _outputChangesOnly = false;
  bool _outputOnStartup = false;
  uint64_t _peerId = 0;
  int32_t _channel = -1;
  std::string _variable;
  EventSource _eventSource = EventSource::all;
  Flows::PVariable _metadata;

  Flows::VariableType _type = Flows::VariableType::tVoid;
  std::string _loopPreventionGroup;
  bool _loopPrevention = false;

  void variableEvent(const std::string &source, uint64_t peerId, int32_t channel, const std::string &variable, const Flows::PVariable &value, const Flows::PVariable &metadata) override;
  void flowVariableEvent(const std::string &flowId, const std::string &variable, const Flows::PVariable &value) override;
  void globalVariableEvent(const std::string &variable, const Flows::PVariable &value) override;
  void input(const Flows::PNodeInfo &info, uint32_t index, const Flows::PVariable &message) override;
};

}

#endif
