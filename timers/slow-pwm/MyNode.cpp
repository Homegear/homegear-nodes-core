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

MyNode::MyNode(const std::string &path, const std::string &type, const std::atomic_bool *frontendConnected) : Flows::INode(path, type, frontendConnected) {
}

MyNode::~MyNode() {
  _stopThread = true;
}

bool MyNode::init(const Flows::PNodeInfo &info) {
  try {
    auto settingsIterator = info->info->structValue->find("period");
    if (settingsIterator != info->info->structValue->end()) _period = Flows::Math::getNumber(settingsIterator->second->stringValue);

    settingsIterator = info->info->structValue->find("dutycyclemin");
    if (settingsIterator != info->info->structValue->end()) _dutyCycleMin = Flows::Math::getNumber(settingsIterator->second->stringValue);

    settingsIterator = info->info->structValue->find("dutycyclemax");
    if (settingsIterator != info->info->structValue->end()) _dutyCycleMax = Flows::Math::getNumber(settingsIterator->second->stringValue);

    if (_dutyCycleMax <= _dutyCycleMin) {
      _out->printError("Error: Duty cycle maximum is smaller than or equal to duty cycle minimum. Setting both to defaults.");
      _dutyCycleMin = 0;
      _dutyCycleMax = 100;
    }

    if (_period < 10) _period = 10;
    _period *= 1000;

    auto enabled = getNodeData("enabled");
    if (enabled->type == Flows::VariableType::tBoolean) _enabled = enabled->booleanValue;

    _startTimeAll = getNodeData("startTimeAll")->integerValue;
    if (_startTimeAll == 0) _startTimeAll = Flows::HelperFunctions::getTime();

    _currentDutyCycle = getNodeData("dutycycle")->integerValue;

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

bool MyNode::start() {
  try {
    _stopped = false;
    if (!_enabled) return true;
    std::lock_guard<std::mutex> timerGuard(_timerMutex);
    _stopThread = false;
    if (_timerThread.joinable()) _timerThread.join();
    _timerThread = std::thread(&MyNode::timer, this);

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

void MyNode::stop() {
  try {
    _stopped = true;
    _stopThread = true;
    setNodeData("startTimeAll", std::make_shared<Flows::Variable>(_startTimeAll));
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void MyNode::waitForStop() {
  try {
    std::lock_guard<std::mutex> timerGuard(_timerMutex);
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
  uint32_t pwmPosition = (Flows::HelperFunctions::getTime() - _startTimeAll) % _period;
  bool pwmState = pwmPosition <= _currentDutyCycle && _currentDutyCycle > _dutyCycleMin;
  bool lastPwmState = pwmState;

  Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
  message->structValue->emplace("payload", std::make_shared<Flows::Variable>(pwmState));
  output(0, message);

  while (!_stopThread) {
    try {
      if (_period < 120000) std::this_thread::sleep_for(std::chrono::milliseconds(100));
      else std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      if (_stopThread) return;

      pwmPosition = (Flows::HelperFunctions::getTime() - _startTimeAll) % _period;
      pwmState = pwmPosition <= _currentDutyCycle && _currentDutyCycle > _dutyCycleMin;
      if (pwmState != lastPwmState || BaseLib::HelperFunctions::getTime() - _lastOutput >= _period) {
        _lastOutput = BaseLib::HelperFunctions::getTime();
        lastPwmState = pwmState;
        message->structValue->at("payload")->booleanValue = pwmState;
        output(0, message);
      }
    }
    catch (const std::exception &ex) {
      _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch (...) {
      _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
  }
}

void MyNode::input(const Flows::PNodeInfo &info, uint32_t index, const Flows::PVariable &message) {
  try {
    if (index == 0) {
      auto &payload = message->structValue->at("payload");
      double dutyCycle = (payload->type == Flows::VariableType::tFloat) ? payload->floatValue : payload->integerValue64;
      if (dutyCycle > _dutyCycleMax) dutyCycle = _dutyCycleMax;
      else if (dutyCycle < _dutyCycleMin) dutyCycle = _dutyCycleMin;
      uint32_t currentDutyCycle = std::lround((double)_period * ((dutyCycle - (double)_dutyCycleMin) / (double)(_dutyCycleMax - _dutyCycleMin)));
      _currentDutyCycle = currentDutyCycle;
      setNodeData("dutycycle", std::make_shared<Flows::Variable>(currentDutyCycle));
    } else if (index == 1) {
      bool newState = (bool)(*message->structValue->at("payload"));
      if (newState != _enabled) {
        _enabled = newState;
        setNodeData("enabled", std::make_shared<Flows::Variable>(_enabled));
        Flows::PVariable status = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
        status->structValue->emplace("text", std::make_shared<Flows::Variable>(_enabled ? "enabled" : "disabled"));
        nodeEvent("statusTop/" + _id, status);
        std::lock_guard<std::mutex> timerGuard(_timerMutex);
        _stopThread = true;
        if (_timerThread.joinable()) _timerThread.join();
        if (_stopped) return;
        if (_enabled) {
          _startTimeAll = Flows::HelperFunctions::getTime();
          _stopThread = false;
          _timerThread = std::thread(&MyNode::timer, this);
        } else {
          Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
          outputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>(false));
          output(0, outputMessage);
        }
      }
    }
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

}
