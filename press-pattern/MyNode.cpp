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

#include "MyNode.h"

namespace MyNode {

MyNode::MyNode(const std::string &path, const std::string &type, const std::atomic_bool *frontendConnected) : Flows::INode(path, type, frontendConnected) {
}

MyNode::~MyNode() {
  _stopThread = true;
}

bool MyNode::init(const Flows::PNodeInfo &info) {
  try {
    auto settingsIterator = info->info->structValue->find("timeout");
    if (settingsIterator != info->info->structValue->end()) _timeout = Flows::Math::getUnsignedNumber(settingsIterator->second->stringValue);

    settingsIterator = info->info->structValue->find("outputs");
    if (settingsIterator != info->info->structValue->end()) _outputs = settingsIterator->second->integerValue == 0 ? Flows::Math::getUnsignedNumber(settingsIterator->second->stringValue) : settingsIterator->second->integerValue;

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
  return true;
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
  try {
    int64_t restTime = (_inputTime + _timeout) - Flows::HelperFunctions::getTime();
    if (restTime < 1) restTime = 1;
    int64_t sleepingTime = 10;
    if (_timeout >= 1000) sleepingTime = 100;
    else if (_timeout >= 30000) sleepingTime = 1000;

    while (restTime > 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(sleepingTime));
      if (_stopThread) {
        _firstPress = true;
        return;
      }

      restTime = (_inputTime + _timeout) - Flows::HelperFunctions::getTime();
    }

    int32_t outputIndex = _counter;
    if (outputIndex >= (signed)_outputs) outputIndex = _outputs - 1;
    if (_state) {
      _longPress = true;
      outputIndex = 0;
    }

    if (_longPress || _outputs > 2) {
      Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
      Flows::PVariable value = std::make_shared<Flows::Variable>(true);
      outputMessage->structValue->emplace("payload", value);
      output(outputIndex, outputMessage);
    }
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
  _firstPress = true;
}

void MyNode::input(const Flows::PNodeInfo &info, uint32_t index, const Flows::PVariable &message) {
  try {
    Flows::PVariable &input = message->structValue->at("payload");
    if (*input) {
      _state = true;
      _inputTime = Flows::HelperFunctions::getTime();

      std::lock_guard<std::mutex> timerGuard(_timerThreadMutex);
      if (_firstPress) {
        _firstPress = false;
        _stopThread = true;
        if (_timerThread.joinable()) _timerThread.join();
        _stopThread = false;
        _counter = 1;
        _timerThread = std::thread(&MyNode::timer, this);
      } else _counter++;
    } else if (_state) {
      _state = false;
      if (_longPress) {
        _longPress = false;
        Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
        outputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>(false));
        output(0, outputMessage);
      } else if (_outputs == 2) {
        Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
        Flows::PVariable value = std::make_shared<Flows::Variable>(true);
        outputMessage->structValue->emplace("payload", value);
        output(1, outputMessage);
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

