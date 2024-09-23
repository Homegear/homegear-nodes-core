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
#include <thread>
#include <mutex>

namespace RateLimiter {

class RateLimiter : public Flows::INode {
 public:
  RateLimiter(const std::string &path, const std::string &type, const std::atomic_bool *frontendConnected);
  ~RateLimiter() override;

  bool init(const Flows::PNodeInfo &info) override;
  bool start() override;
  void stop() override;
  void waitForStop() override;
 private:
  enum class RateLimiterState : int32_t {
    kIdle = 0,
    kFirst = 1,
    kFirstOffset = 2,
    kReceiving = 3,
    kWaitingForInput = 4,
  };

  uint32_t _maxInputs = 1;
  uint32_t _interval = 1000;
  bool _outputFirst = true;
  bool _outputChanges = false;

  std::atomic_bool _stopThread{true};
  std::mutex _timerThreadMutex;
  std::thread _timerThread;

  std::mutex _dataMutex;
  RateLimiterState _state = RateLimiterState::kIdle;
  Flows::PVariable _latestInput;
  Flows::PVariable _currentValue = std::make_shared<Flows::Variable>();
  int64_t _firstInputTime = 0;
  size_t _inputCount = 0;
  void timer();
  void input(const Flows::PNodeInfo &info, uint32_t index, const Flows::PVariable &message) override;
};

}

#endif
