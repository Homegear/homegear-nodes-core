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

#include "TcpIn.h"

namespace TcpIn {

TcpIn::TcpIn(const std::string &path, const std::string &type, const std::atomic_bool *frontendConnected) : Flows::INode(path, type, frontendConnected) {
  _localRpcMethods.emplace("packetReceived", std::bind(&TcpIn::packetReceived, this, std::placeholders::_1));
  _localRpcMethods.emplace("setConnectionState", std::bind(&TcpIn::setConnectionState, this, std::placeholders::_1));
}

TcpIn::~TcpIn() = default;

bool TcpIn::init(const Flows::PNodeInfo &info) {
  try {
    auto settingsIterator = info->info->structValue->find("socket");
    if (settingsIterator != info->info->structValue->end()) _socket = settingsIterator->second->stringValue;

    settingsIterator = info->info->structValue->find("output");
    if (settingsIterator != info->info->structValue->end()) {
      if (settingsIterator->second->stringValue == "rawBinary") _payloadType = PayloadType::rawBinary;
      else if (settingsIterator->second->stringValue == "string") _payloadType = PayloadType::string;
      else if (settingsIterator->second->stringValue == "json") _payloadType = PayloadType::json;
    }

    std::string splitAfterType = "string";
    settingsIterator = info->info->structValue->find("splitAfterType");
    if (settingsIterator != info->info->structValue->end()) {
      splitAfterType = settingsIterator->second->stringValue;
    }

    settingsIterator = info->info->structValue->find("splitAfter");
    if (settingsIterator != info->info->structValue->end()) {
      if (splitAfterType == "string") {
        auto splitAfter = BaseLib::Rpc::JsonDecoder::decodeString(settingsIterator->second->stringValue);
        _splitAfter = std::vector<uint8_t>(splitAfter.begin(), splitAfter.end());
      } else {
        auto array = Flows::JsonDecoder::decode(settingsIterator->second->stringValue);
        _splitAfter.reserve(array->arrayValue->size());
        for (auto &byte: *array->arrayValue) {
          _splitAfter.push_back((uint8_t)byte->integerValue);
        }
      }
    }

    settingsIterator = info->info->structValue->find("removeSplitBytes");
    if (settingsIterator != info->info->structValue->end()) {
      _removeSplitBytes = settingsIterator->second->booleanValue;
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

void TcpIn::configNodesStarted() {
  try {
    if (_socket.empty()) {
      _out->printError("Error: This node has no server assigned.");
      return;
    }
    Flows::PArray parameters = std::make_shared<Flows::Array>();
    parameters->reserve(2);
    parameters->push_back(std::make_shared<Flows::Variable>(_id));
    parameters->push_back(std::make_shared<Flows::Variable>(1));
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

//{{{ RPC methods
Flows::PVariable TcpIn::packetReceived(const Flows::PArray &parameters) {
  try {
    if (parameters->size() != 2) return Flows::Variable::createError(-1, "Method expects exactly 2 parameters. " + std::to_string(parameters->size()) + " given.");
    if (parameters->at(0)->type != Flows::VariableType::tInteger && parameters->at(0)->type != Flows::VariableType::tInteger64) return Flows::Variable::createError(-1, "Parameter 1 is not of type integer.");
    if (parameters->at(1)->type != Flows::VariableType::tBinary) return Flows::Variable::createError(-1, "Parameter 2 is not of type binary.");

    bool finished = false;

    auto payload = parameters->at(1)->binaryValue;

    while (!finished) {
      std::vector<uint8_t> data;
      if (!_splitAfter.empty()) {
        auto iterator = std::search(payload.begin(), payload.end(), _splitAfter.begin(), _splitAfter.end());
        if (iterator == payload.end()) {
          finished = true;
          if (_buffer.size() + payload.size() > 102400) {
            _out->printError("Error: Buffer is full.");
            _buffer.clear();
            return std::make_shared<Flows::Variable>();
          }
          _buffer.insert(_buffer.end(), payload.begin(), payload.end());
          return std::make_shared<Flows::Variable>();
        }
        data = _buffer;
        _buffer.clear();
        if (_removeSplitBytes) {
          data.insert(data.end(), payload.begin(), iterator);
        } else {
          data.insert(data.end(), payload.begin(), iterator + (signed)_splitAfter.size());
        }
        payload = std::vector<uint8_t>(iterator + (signed)_splitAfter.size(), payload.end());
      } else {
        finished = true;
        data.swap(payload);
      }

      Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
      message->structValue->emplace("clientId", std::make_shared<Flows::Variable>(parameters->at(0)->integerValue));
      if (_payloadType == PayloadType::rawBinary) message->structValue->emplace("payload", std::make_shared<Flows::Variable>(data));
      else if (_payloadType == PayloadType::string) message->structValue->emplace("payload", std::make_shared<Flows::Variable>(std::string(data.begin(), data.end())));
      else if (_payloadType == PayloadType::hex) message->structValue->emplace("payload", std::make_shared<Flows::Variable>(BaseLib::HelperFunctions::getHexString(data)));
      else if (_payloadType == PayloadType::json) message->structValue->emplace("payload", Flows::JsonDecoder::decode(std::string(data.begin(), data.end())));

      Flows::PVariable internal = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
      internal->structValue->emplace("clientId", std::make_shared<Flows::Variable>(parameters->at(0)->integerValue));

      setInternalMessage(internal);

      output(0, message);
    }

    return std::make_shared<Flows::Variable>();
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
  return Flows::Variable::createError(-32500, "Unknown application error.");
}

Flows::PVariable TcpIn::setConnectionState(const Flows::PArray& parameters)
{
  try
  {
    if(parameters->size() != 1) return Flows::Variable::createError(-1, "Method expects exactly one parameter. " + std::to_string(parameters->size()) + " given.");
    if(parameters->at(0)->type != Flows::VariableType::tBoolean) return Flows::Variable::createError(-1, "Parameter is not of type boolean.");

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
