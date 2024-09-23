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

MyNode::~MyNode() = default;

bool MyNode::init(const Flows::PNodeInfo &info) {
  try {
    std::string variableType = "device";
    auto settingsIterator = info->info->structValue->find("variabletype");
    if (settingsIterator != info->info->structValue->end()) variableType = settingsIterator->second->stringValue;

    if (variableType == "device") _variableType = VariableType::device;
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

    settingsIterator = info->info->structValue->find("variable");
    if (settingsIterator != info->info->structValue->end()) _variable = settingsIterator->second->stringValue;

    settingsIterator = info->info->structValue->find("wait");
    if (settingsIterator != info->info->structValue->end()) _wait = settingsIterator->second->booleanValue;

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
    if (_variableType == VariableType::flow) {
      setFlowData(_variable, message->structValue->at("payload"));
      message->structValue->emplace("payload", std::make_shared<Flows::Variable>(true));

      output(0, message);
    } else if (_variableType == VariableType::global) {
      setGlobalData(_variable, message->structValue->at("payload"));
      message->structValue->emplace("payload", std::make_shared<Flows::Variable>(true));

      output(0, message);
    } else {
      Flows::PArray parameters = std::make_shared<Flows::Array>();
      parameters->reserve(4);
      parameters->push_back(std::make_shared<Flows::Variable>(_peerId));
      parameters->push_back(std::make_shared<Flows::Variable>(_channel));
      parameters->push_back(std::make_shared<Flows::Variable>(_variable));
      parameters->push_back(message->structValue->at("payload"));
      parameters->push_back(std::make_shared<Flows::Variable>(_wait));

      Flows::PVariable result = invoke("setValue", parameters);

      Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
      if (result->errorStruct) {
        _out->printError("Error setting variable (Peer ID: " + std::to_string(_peerId) + ", channel: " + std::to_string(_channel) + ", name: " + _variable + "): " + result->structValue->at("faultString")->stringValue);
        outputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>(false));
      } else {
        outputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>(true));
      }

      output(0, outputMessage);
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
