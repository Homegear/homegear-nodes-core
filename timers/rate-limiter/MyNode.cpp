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

#include <homegear-base/HelperFunctions/HelperFunctions.h>
#include "MyNode.h"

namespace MyNode {

MyNode::MyNode(const std::string &path, const std::string &nodeNamespace, const std::string &type, const std::atomic_bool *frontendConnected) : Flows::INode(path, nodeNamespace, type, frontendConnected) {
}

MyNode::~MyNode() {
  _stopThread = true;
}

bool MyNode::init(const Flows::PNodeInfo &info) {
  try {
    auto settingsIterator = info->info->structValue->find("max-inputs");
    if (settingsIterator != info->info->structValue->end()) _maxInputs = Flows::Math::getUnsignedNumber(settingsIterator->second->stringValue);
    if (_maxInputs < 1) _maxInputs = 1;

    settingsIterator = info->info->structValue->find("interval");
    if (settingsIterator != info->info->structValue->end()) _interval = Flows::Math::getUnsignedNumber(settingsIterator->second->stringValue);

    settingsIterator = info->info->structValue->find("output-first");
    if (settingsIterator != info->info->structValue->end()) _outputFirst = settingsIterator->second->booleanValue;

    return true;
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return false;
}

bool MyNode::start() {
  try {
    std::lock_guard<std::mutex> timerGuard(_timerThreadMutex);
    _stopThread = true;
    if (_timerThread.joinable()) _timerThread.join();
    _stopThread = false;
    _timerThread = std::thread(&MyNode::timer, this);

    return true;
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return false;
}

void MyNode::stop() {
  std::lock_guard<std::mutex> timerGuard(_timerThreadMutex);
  _stopThread = true;
}

void MyNode::waitForStop() {
  try {
    std::lock_guard<std::mutex> timerGuard(_timerThreadMutex);
    _stopThread = true;
    if (_timerThread.joinable()) _timerThread.join();
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void MyNode::timer() {
  int32_t sleepingTime = _interval;
  if (sleepingTime < 1) sleepingTime = 1;

  int64_t startTime = Flows::HelperFunctions::getTime();

  while (!_stopThread) {
    try {
      if (sleepingTime > 1000 && sleepingTime < 30000) {
        int32_t iterations = sleepingTime / 100;
        for (int32_t j = 0; j < iterations; j++) {
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
          if (_stopThread) break;
        }
        if (sleepingTime % 100) std::this_thread::sleep_for(std::chrono::milliseconds(sleepingTime % 100));
      } else if (sleepingTime >= 30000) {
        int32_t iterations = sleepingTime / 1000;
        for (int32_t j = 0; j < iterations; j++) {
          std::this_thread::sleep_for(std::chrono::milliseconds(1000));
          if (_stopThread) break;
        }
        if (sleepingTime % 1000) std::this_thread::sleep_for(std::chrono::milliseconds(sleepingTime % 1000));
      } else {
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepingTime));
        if (_stopThread) break;
      }

      std::lock_guard<std::mutex> dataGuard(_dataMutex);
      if (_state == RateLimiterState::kFirst) {
        //The timer runs continuously, so the interval between input and this code position is less than interval at most times. Add the offset and sleep again.
        startTime = _firstInputTime;
        sleepingTime = _interval - (Flows::HelperFunctions::getTime() - _firstInputTime);
        if (sleepingTime < 1) sleepingTime = 1;
        else if ((unsigned)sleepingTime > _interval) sleepingTime = _interval;
        _state = RateLimiterState::kFirstOffset;
        continue;
      } else if(_state == RateLimiterState::kFirstOffset) {
        _state = RateLimiterState::kReceiving;
      } else if (_state == RateLimiterState::kReceiving) {
        _state = RateLimiterState::kWaitingForInput;
      } else if (_state == RateLimiterState::kWaitingForInput) {
        if (_lastInput) {
          output(0, _lastInput);
          _lastInput.reset();
        }
        _state = RateLimiterState::kIdle;
      }
      _inputCount = 0;

      int64_t diff = Flows::HelperFunctions::getTime() - startTime;
      if (diff <= (int64_t)_interval) sleepingTime = _interval;
      else sleepingTime = _interval - (diff - _interval);
      if (sleepingTime < 1) sleepingTime = 1;
      startTime = Flows::HelperFunctions::getTime();
    }
    catch (const std::exception &ex) {
      _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
  }
}

void MyNode::input(const Flows::PNodeInfo &info, uint32_t index, const Flows::PVariable &message) {
  try {
    std::lock_guard<std::mutex> dataGuard(_dataMutex);
    if (_state == RateLimiterState::kIdle) {
      _firstInputTime = Flows::HelperFunctions::getTime();
      _state = RateLimiterState::kFirst;
    }
    if (_inputCount < _maxInputs) {
      _lastInput.reset();

      if ((!_outputFirst && _state != RateLimiterState::kFirst && _state != RateLimiterState::kFirstOffset) || _outputFirst) {
        _inputCount++;
        output(0, message);
      } else {
        _inputCount++;
        _lastInput = message;
      }

      if (_state == RateLimiterState::kWaitingForInput) _state = RateLimiterState::kReceiving;
    } else {
      _lastInput = message;
    }
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

}

