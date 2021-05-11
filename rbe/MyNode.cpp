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

MyNode::MyNode(const std::string &path, const std::string &type, const std::atomic_bool *frontendConnected)
    : Flows::INode(path, type, frontendConnected) {
}

MyNode::~MyNode() = default;

bool MyNode::init(const Flows::PNodeInfo &info) {
  try {
    auto settingsIterator = info->info->structValue->find("mode");
    if (settingsIterator != info->info->structValue->end()) {
      if (settingsIterator->second->stringValue.compare("blockValueChange") == 0) {
        _mode = blockValueChange;
      } else if (settingsIterator->second->stringValue.compare("blockValueChangeIgnore") == 0) {
        _mode = blockValueChangeIgnore;
      } else if (settingsIterator->second->stringValue.compare("blockValueChangeGreaterEqual") == 0) {
        _mode = blockValueChangeGreaterEqual;
      } else if (settingsIterator->second->stringValue.compare("blockValueChangeGreater") == 0) {
        _mode = blockValueChangeGreater;
      } else if (settingsIterator->second->stringValue.compare("blockIfValueChangeGreaterEqual") == 0) {
        _mode = blockIfValueChangeGreaterEqual;
      } else if (settingsIterator->second->stringValue.compare("blockIfValueChangeGreater") == 0) {
        _mode = blockIfValueChangeGreater;
      }
    }

    settingsIterator = info->info->structValue->find("compareTo");
    if (settingsIterator != info->info->structValue->end()) {
      if (settingsIterator->second->stringValue.compare("lastOutput") == 0) {
        _compareTo = lastOutput;
      } else if (settingsIterator->second->stringValue.compare("lastInput") == 0) {
        _compareTo = lastInput;
      }
    }

    settingsIterator = info->info->structValue->find("inputs");
    if (settingsIterator != info->info->structValue->end()) {
      _inputs = settingsIterator->second->integerValue;
    }

    if (_mode == blockIfValueChangeGreaterEqual || _mode == blockIfValueChangeGreater) {
      settingsIterator = info->info->structValue->find("startValue");
      if (settingsIterator != info->info->structValue->end()) {
        if (!settingsIterator->second->stringValue.empty()) {
          _startValue = Flows::Math::getDouble(settingsIterator->second->stringValue);
          for (int i = 0; i < _inputs; ++i) {
            _lastInput.insert_or_assign(i, _startValue);
          }
        }
      }
    }

    settingsIterator = info->info->structValue->find("range");
    if (settingsIterator != info->info->structValue->end()) {
      _range = Flows::Math::getDouble(settingsIterator->second->stringValue);
    }

    settingsIterator = info->info->structValue->find("rangeType");
    if (settingsIterator != info->info->structValue->end()) {
      if (settingsIterator->second->stringValue.compare("flatValue") == 0) {
        _rangeType = flatValue;
      } else if (settingsIterator->second->stringValue.compare("percent") == 0) {
        _rangeType = percent;
      }
    }

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

}

void MyNode::waitForStop() {
  try {

  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void MyNode::input(const Flows::PNodeInfo &info,
                   uint32_t index,
                   const Flows::PVariable &message) { //is executed, when a new message arrives
  try {
    Flows::PVariable &input = message->structValue->at("payload");
    input->floatValue = std::llround(input->floatValue * std::pow(10, 7)) / std::pow(10, 7);
    Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);

    switch (_mode) {
      case blockValueChange: {
        auto it = _lastInput.find(index);
        if (it != _lastInput.end()) {
          if (it->second != input->floatValue) {
            it->second = input->floatValue;
            message->structValue->emplace("payload", std::make_shared<Flows::Variable>(input->floatValue));
            output(index, message);
          }
        } else {
          _lastInput.insert_or_assign(index, input->floatValue);
          message->structValue->emplace("payload", std::make_shared<Flows::Variable>(input->floatValue));
          output(index, message);
        }
        break;
      }
      case blockValueChangeIgnore: {
        auto it = _lastInput.find(index);
        if (it != _lastInput.end()) {
          if (it->second != input->floatValue) {
            it->second = input->floatValue;
            message->structValue->emplace("payload", std::make_shared<Flows::Variable>(input->floatValue));
            output(index, message);
          }
        } else {
          _lastInput.insert_or_assign(index, input->floatValue);
        }
        break;
      }
      case blockValueChangeGreaterEqual: {
        auto it = _lastInput.find(index);
        if (it != _lastInput.end()) {
          double difference = _range;
          if (_rangeType == percent) {
            difference = it->second * (_range / 100);
          }
          difference = std::abs(difference);
          switch (_compareTo) {
            case lastOutput:
              if ((it->second - difference) >= input->floatValue || (it->second + difference) <= input->floatValue) {
                it->second = input->floatValue;
                message->structValue->emplace("payload", std::make_shared<Flows::Variable>(input->floatValue));
                output(index, message);
              }
              break;
            case lastInput:
              if ((it->second - difference) >= input->floatValue || (it->second + difference) <= input->floatValue) {
                message->structValue->emplace("payload", std::make_shared<Flows::Variable>(input->floatValue));
                output(index, message);
              }
              it->second = input->floatValue;
              break;
          }
        } else {
          _lastInput.insert_or_assign(index, input->floatValue);
          message->structValue->emplace("payload", std::make_shared<Flows::Variable>(input->floatValue));
          output(index, message);
        }
        break;
      }
      case blockValueChangeGreater: {
        auto it = _lastInput.find(index);
        if (it != _lastInput.end()) {
          double difference = _range;
          if (_rangeType == percent) {
            difference = it->second * (_range / 100);
          }
          difference = std::abs(difference);
          switch (_compareTo) {
            case lastOutput:
              if ((it->second - difference) > input->floatValue || (it->second + difference) < input->floatValue) {
                it->second = input->floatValue;
                message->structValue->emplace("payload", std::make_shared<Flows::Variable>(input->floatValue));
                output(index, message);
              }
              break;
            case lastInput:
              if ((it->second - difference) > input->floatValue || (it->second + difference) < input->floatValue) {
                message->structValue->emplace("payload", std::make_shared<Flows::Variable>(input->floatValue));
                output(index, message);
              }
              it->second = input->floatValue;
              break;
          }
        } else {
          _lastInput.insert_or_assign(index, input->floatValue);
          message->structValue->emplace("payload", std::make_shared<Flows::Variable>(input->floatValue));
          output(index, message);
        }
        break;
      }
      case blockIfValueChangeGreaterEqual: {
        auto it = _lastInput.find(index);
        if (it != _lastInput.end()) {
          double difference = _range;
          if (_rangeType == percent) {
            difference = it->second * (_range / 100);
          }
          difference = std::abs(difference);
          switch (_compareTo) {
            case lastOutput:
              if ((it->second - difference) < input->floatValue && (it->second + difference) > input->floatValue) {
                it->second = input->floatValue;
                message->structValue->emplace("payload", std::make_shared<Flows::Variable>(input->floatValue));
                output(index, message);
              }
              break;
            case lastInput:
              if ((it->second - difference) < input->floatValue && (it->second + difference) > input->floatValue) {
                message->structValue->emplace("payload", std::make_shared<Flows::Variable>(input->floatValue));
                output(index, message);
              }
              it->second = input->floatValue;
              break;
          }
        } else {
          _lastInput.insert_or_assign(index, input->floatValue);
          message->structValue->emplace("payload", std::make_shared<Flows::Variable>(input->floatValue));
          output(index, message);
        }
        break;
      }
      case blockIfValueChangeGreater: {
        auto it = _lastInput.find(index);
        if (it != _lastInput.end()) {
          double difference = _range;
          if (_rangeType == percent) {
            difference = it->second * (_range / 100);
          }
          difference = std::abs(difference);
          switch (_compareTo) {
            case lastOutput:
              if ((it->second - difference) <= input->floatValue && (it->second + difference) >= input->floatValue) {
                it->second = input->floatValue;
                message->structValue->emplace("payload", std::make_shared<Flows::Variable>(input->floatValue));
                output(index, message);
              }
              break;
            case lastInput:
              if ((it->second - difference) <= input->floatValue && (it->second + difference) >= input->floatValue) {
                message->structValue->emplace("payload", std::make_shared<Flows::Variable>(input->floatValue));
                output(index, message);
              }
              it->second = input->floatValue;
              break;
          }
        } else {
          _lastInput.insert_or_assign(index, input->floatValue);
          message->structValue->emplace("payload", std::make_shared<Flows::Variable>(input->floatValue));
          output(index, message);
        }
        break;
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

