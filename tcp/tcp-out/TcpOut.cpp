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
#include "TcpOut.h"

namespace TcpOut {

TcpOut::TcpOut(const std::string &path, const std::string &type, const std::atomic_bool *frontendConnected) : Flows::INode(path, type, frontendConnected) {
  _localRpcMethods.emplace("setConnectionState", std::bind(&TcpOut::setConnectionState, this, std::placeholders::_1));
}

TcpOut::~TcpOut() = default;

bool TcpOut::init(const Flows::PNodeInfo &info) {
  try {
    auto settingsIterator = info->info->structValue->find("socket");
    if (settingsIterator != info->info->structValue->end()) _socket = settingsIterator->second->stringValue;

    settingsIterator = info->info->structValue->find("input");
    if (settingsIterator != info->info->structValue->end()) {
      if (settingsIterator->second->stringValue == "raw") _payloadType = PayloadType::raw;
      else if (settingsIterator->second->stringValue == "json") _payloadType = PayloadType::json;
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

void TcpOut::configNodesStarted() {
  try {
    if (_socket.empty()) {
      _out->printError("Error: This node has no socket assigned.");
      return;
    }
    Flows::PArray parameters = std::make_shared<Flows::Array>();
    parameters->reserve(2);
    parameters->push_back(std::make_shared<Flows::Variable>(_id));
    parameters->push_back(std::make_shared<Flows::Variable>(2));
    Flows::PVariable result = invokeNodeMethod(_socket, "registerNode", parameters, true);
    if (result->errorStruct) _out->printError("Error: Could not register node: " + result->structValue->at("faultString")->stringValue);
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void TcpOut::input(const Flows::PNodeInfo &info, uint32_t index, const Flows::PVariable &message) {
  try {
    if (_socket.empty()) {
      _out->printError("Error: This node has no server assigned.");
      return;
    }

    int32_t clientId = -1;

    auto messageIterator = message->structValue->find("clientId");
    if (messageIterator != message->structValue->end()) {
      clientId = messageIterator->second->integerValue;
    } else {
      messageIterator = message->structValue->find("_internal");
      if (messageIterator != message->structValue->end()) {
        auto internalIterator = messageIterator->second->structValue->find("clientId");
        if (internalIterator != messageIterator->second->structValue->end()) {
          clientId = internalIterator->second->integerValue;
        }
      }
    }

    std::vector<uint8_t> data;
    if (_payloadType == PayloadType::hex) data = BaseLib::HelperFunctions::getUBinary(message->structValue->at("payload")->stringValue);
    else if (_payloadType == PayloadType::json) {
      Flows::JsonEncoder jsonEncoder;
      auto json = jsonEncoder.getString(message->structValue->at("payload"));
      data.insert(data.end(), json.begin(), json.end());
    } else {
      if (message->structValue->at("payload")->type == Flows::VariableType::tBinary) data.insert(data.end(), message->structValue->at("payload")->binaryValue.begin(), message->structValue->at("payload")->binaryValue.end());
      else data.insert(data.end(), message->structValue->at("payload")->stringValue.begin(), message->structValue->at("payload")->stringValue.end());
    }

    Flows::PArray parameters = std::make_shared<Flows::Array>();
    parameters->reserve(2);
    parameters->push_back(std::make_shared<Flows::Variable>(clientId));
    parameters->push_back(std::make_shared<Flows::Variable>(data));

    Flows::PVariable result = invokeNodeMethod(_socket, "send", parameters, true);
    if (result->errorStruct) _out->printError("Error sending data: " + result->structValue->at("faultString")->stringValue);
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

//{{{ RPC methods
Flows::PVariable TcpOut::setConnectionState(const Flows::PArray& parameters)
{
  try
  {
    _out->printWarning("Moin0");

    if(parameters->size() != 1) return Flows::Variable::createError(-1, "Method expects exactly one parameter. " + std::to_string(parameters->size()) + " given.");
    if(parameters->at(0)->type != Flows::VariableType::tBoolean) return Flows::Variable::createError(-1, "Parameter is not of type boolean.");

    _out->printWarning("Moin1");

    Flows::PVariable status = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
    if(parameters->at(0)->booleanValue)
    {
      status->structValue->emplace("text", std::make_shared<Flows::Variable>("connected"));
      status->structValue->emplace("fill", std::make_shared<Flows::Variable>("green"));
      status->structValue->emplace("shape", std::make_shared<Flows::Variable>("dot"));
    }
    else
    {
      status->structValue->emplace("text", std::make_shared<Flows::Variable>("disconnected"));
      status->structValue->emplace("fill", std::make_shared<Flows::Variable>("red"));
      status->structValue->emplace("shape", std::make_shared<Flows::Variable>("dot"));
    }
    nodeEvent("status/" + _id, status, true);

    return std::make_shared<Flows::Variable>();
  }
  catch(const std::exception& ex)
  {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch(...)
  {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
  return Flows::Variable::createError(-32500, "Unknown application error.");
}
//}}}

}
