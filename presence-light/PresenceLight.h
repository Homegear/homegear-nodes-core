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

#ifndef PRESENCELIGHT_H_
#define PRESENCELIGHT_H_

#include <homegear-node/INode.h>
#include <mutex>
#include <thread>

namespace PresenceLight {

class PresenceLight : public Flows::INode {
 public:
  PresenceLight(const std::string &path, const std::string &type, const std::atomic_bool *frontendConnected);
  ~PresenceLight() override;

  bool init(const Flows::PNodeInfo &info) override;
  bool start() override;
  void startUpComplete() override;
  void stop() override;
  void waitForStop() override;
 private:
  //{{{ Only used by one thread / protected by input mutex of Homegear
  uint32_t _onTime = 0;
  uint32_t _alwaysOnTime = 21600000;
  uint32_t _alwaysOffTime = 21600000;
  int64_t _lastInput = -1;
  bool _switchOffOnInFalse = false;
  bool _keepOn = false;
  uint32_t _refractionTime = 1000;
  bool _outputChangesOnly = false;
  //}}}

  std::atomic_bool _stopThread{true};
  std::atomic_bool _stopped{true};
  std::mutex _timerThreadMutex;
  std::thread _timerThread;
  std::atomic_bool _toggleProfile0Only{false};
  std::atomic_bool _restoreProfile{false};
  std::atomic_bool _booleanStateValue{true};
  std::atomic<int64_t> _stateValue{1};
  std::atomic<int64_t> _lastNonNullStateValue{1};

  std::atomic_bool _enabled{true};
  std::atomic_bool _manuallyEnabled{false};
  std::atomic_bool _manuallyDisabled{false};
  std::atomic<int64_t> _onTo{-1};
  int64_t _inBlockedUntil = 0;
  std::atomic<int64_t> _alwaysOnTo{-1};
  std::atomic<int64_t> _alwaysOffTo{-1};
  std::mutex _stateOutputMutex; //Used in method stateOutput
  Flows::PVariable _lastOutput = std::make_shared<Flows::Variable>();

  bool getLightState();
  Flows::PVariable getLightStateVariable();
  void timer();
  void input(const Flows::PNodeInfo &info, uint32_t index, const Flows::PVariable &message) override;
  void stateOutput(const Flows::PVariable &value);
};

}

#endif
