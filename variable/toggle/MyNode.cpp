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

MyNode::MyNode(const std::string &path, const std::string &nodeNamespace, const std::string &type, const std::atomic_bool *frontendConnected) : Flows::INode(path, nodeNamespace, type, frontendConnected) {
}

MyNode::~MyNode() = default;

bool MyNode::init(const Flows::PNodeInfo &info) {
  try {
    auto settingsIterator = info->info->structValue->find("true-only");
    if (settingsIterator != info->info->structValue->end()) _trueOnly = settingsIterator->second->booleanValue;

    settingsIterator = info->info->structValue->find("use-feedback");
    if (settingsIterator != info->info->structValue->end()) _useFeedback = settingsIterator->second->booleanValue;

    std::string variableType = "device";
    settingsIterator = info->info->structValue->find("variabletype");
    if (settingsIterator != info->info->structValue->end()) variableType = settingsIterator->second->stringValue;

    if (variableType == "self") _variableType = VariableType::self;
    else if (variableType == "device") _variableType = VariableType::device;
    else if (variableType == "metadata") _variableType = VariableType::metadata;
    else if (variableType == "system") _variableType = VariableType::system;
    else if (variableType == "flow") _variableType = VariableType::flow;
    else if (variableType == "global") _variableType = VariableType::global;

    if (_variableType == VariableType::device || _variableType == VariableType::metadata) {
      settingsIterator = info->info->structValue->find("peerid");
      if (settingsIterator != info->info->structValue->end()) _peerId = Flows::Math::getNumber64(settingsIterator->second->stringValue);
    }

    if (_variableType == VariableType::device) {
      settingsIterator = info->info->structValue->find("channel");
      if (settingsIterator != info->info->structValue->end()) _channel = Flows::Math::getNumber(settingsIterator->second->stringValue);
    }

    if (_variableType != VariableType::self) {
      settingsIterator = info->info->structValue->find("variable");
      if (settingsIterator != info->info->structValue->end()) _variable = settingsIterator->second->stringValue;
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
    _currentValue = getNodeData("value")->booleanValue;

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

void MyNode::input(const Flows::PNodeInfo &info, uint32_t index, const Flows::PVariable &message) {
  try {
    if (index == 0) {
      if (_trueOnly && !(bool)(*message->structValue->at("payload"))) return;

      if (_variableType == VariableType::self) {
        bool newValue = !_currentValue;
        if (!_useFeedback) {
          newValue = !((bool)(*getNodeData("value")));
          setNodeData("value", std::make_shared<Flows::Variable>(newValue));
        } else _currentValue = newValue;

        Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
        outputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>(newValue));

        output(0, outputMessage);
      } else if (_variableType == VariableType::flow) {
        bool newValue = !_currentValue;
        if (!_useFeedback) {
          newValue = !((bool)(*getFlowData(_variable)));
          setFlowData(_variable, std::make_shared<Flows::Variable>(newValue));
        } else _currentValue = newValue;

        Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
        outputMessage->structValue->emplace("variable", std::make_shared<Flows::Variable>(_variable));
        outputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>(newValue));

        output(0, outputMessage);
      } else if (_variableType == VariableType::global) {
        bool newValue = !_currentValue;
        if (!_useFeedback) {
          newValue = !((bool)(*getFlowData(_variable)));
          setGlobalData(_variable, std::make_shared<Flows::Variable>(newValue));
        } else _currentValue = newValue;

        Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
        outputMessage->structValue->emplace("variable", std::make_shared<Flows::Variable>(_variable));
        outputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>(newValue));

        output(0, outputMessage);
      } else {
        Flows::PArray parameters = std::make_shared<Flows::Array>();
        parameters->reserve(4);
        parameters->push_back(std::make_shared<Flows::Variable>(_peerId));
        parameters->push_back(std::make_shared<Flows::Variable>(_channel));
        parameters->push_back(std::make_shared<Flows::Variable>(_variable));

        bool newValue = !_currentValue;
        if (!_useFeedback) {
          Flows::PVariable oldValue = invoke("getValue", parameters);
          if (oldValue->errorStruct) {
            _out->printError("Error getting variable value (Peer ID: " + std::to_string(_peerId) + ", channel: " + std::to_string(_channel) + ", name: " + _variable + "): " + oldValue->structValue->at("faultString")->stringValue);
            return;
          }
          newValue = !oldValue->booleanValue;
        } else _currentValue = newValue;

        parameters->push_back(std::make_shared<Flows::Variable>(newValue));

        Flows::PVariable result = invoke("setValue", parameters);
        if (result->errorStruct) _out->printError("Error setting variable (Peer ID: " + std::to_string(_peerId) + ", channel: " + std::to_string(_channel) + ", name: " + _variable + "): " + result->structValue->at("faultString")->stringValue);

        Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
        outputMessage->structValue->emplace("peerId", std::make_shared<Flows::Variable>(_peerId));
        outputMessage->structValue->emplace("channel", std::make_shared<Flows::Variable>(_channel));
        outputMessage->structValue->emplace("variable", std::make_shared<Flows::Variable>(_variable));
        outputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>(newValue));

        output(0, outputMessage);
      }
    } else if (index == 1 && _useFeedback) {
      _currentValue = (bool)(*message->structValue->at("payload"));
      setNodeData("value", std::make_shared<Flows::Variable>(_currentValue));
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
