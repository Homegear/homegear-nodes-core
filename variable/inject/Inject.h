/* Copyright 2013-2021 Homegear GmbH
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
#include <ctime>

namespace Inject {

class Inject : public Flows::INode {
 public:
  Inject(const std::string &path, const std::string &type, const std::atomic_bool *frontendConnected);
  ~Inject() override;

  bool init(const Flows::PNodeInfo &info) override;
  bool start() override;
  void stop() override;
  void waitForStop() override;
  void setNodeVariable(const std::string &variable, const Flows::PVariable &value) override;
 private:
  std::atomic_bool _stopThread{true};
  std::mutex _workerThreadMutex;
  std::thread _workerThread;

  std::vector<Flows::PVariable> _properties;
  enum Mode {
    None,
    Interval,
    Interval_Time,
    Time
  };

  struct Time {
    uint8_t hour = 0;
    uint8_t minute = 0;
  };

  struct Interval {
    uint8_t start = 0;
    uint8_t stop = 1;
  };


  Mode _mode = None;
  bool _once = false;
  int32_t _onceDelay = 1;
  int32_t _sleepingTime = 1;
  struct Interval _interval{0, 1};
  struct Time _startingTime {12, 0};

  //sunday = 0
  std::map<int, bool> _days {{0, false}, {1, false}, {2, false}, {3, false}, {4, false}, {5, false}, {6, false}};

  void evalMode();
  void intervalMode();
  void intervalTimeMode();
  void timeMode();
  void sendMessage();
  static std::tm* getTime();
};

}

#endif
