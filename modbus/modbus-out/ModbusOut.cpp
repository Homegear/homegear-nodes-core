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

#include "ModbusOut.h"

namespace ModbusOut {

ModbusOut::ModbusOut(const std::string &path, const std::string &type, const std::atomic_bool *frontendConnected) : Flows::INode(path, type, frontendConnected) {
  _localRpcMethods.emplace("setConnectionState", std::bind(&ModbusOut::setConnectionState, this, std::placeholders::_1));
}

ModbusOut::~ModbusOut() = default;

bool ModbusOut::init(const Flows::PNodeInfo &info) {
  try {
    int32_t inputIndex = -1;

    auto settingsIterator = info->info->structValue->find("server");
    if (settingsIterator != info->info->structValue->end()) _socket = settingsIterator->second->stringValue;

    settingsIterator = info->info->structValue->find("registers");
    if (settingsIterator != info->info->structValue->end()) {
      for (auto &element : *settingsIterator->second->arrayValue) {
        inputIndex++;

        auto modbustypeIterator = element->structValue->find("mt");
        if (modbustypeIterator == element->structValue->end()) continue;

        auto indexIterator = element->structValue->find("r");
        if (indexIterator == element->structValue->end()) continue;

        auto countIterator = element->structValue->find("c");
        if (countIterator == element->structValue->end()) continue;

        auto ibIterator = element->structValue->find("ib");
        if (ibIterator == element->structValue->end()) continue;

        auto irIterator = element->structValue->find("ir");
        if (irIterator == element->structValue->end()) continue;

        int32_t index = Flows::Math::getNumber(indexIterator->second->stringValue);
        int32_t count = Flows::Math::getNumber(countIterator->second->stringValue);

        if (index < 0) continue;
        if (count < 1) count = 1;

        auto registerInfo = std::make_shared<RegisterInfo>();
        if (modbustypeIterator->second->type == Flows::VariableType::tInteger || modbustypeIterator->second->type == Flows::VariableType::tInteger64) registerInfo->modbusType = (ModbusType)modbustypeIterator->second->integerValue;
        else registerInfo->modbusType = (ModbusType)Flows::Math::getNumber(modbustypeIterator->second->stringValue);
        registerInfo->inputIndex = (uint32_t)inputIndex;
        registerInfo->index = (uint32_t)index;
        registerInfo->count = registerInfo->modbusType == ModbusType::tHoldingRegister ? (uint32_t)count : 1;
        registerInfo->invertBytes = ibIterator->second->booleanValue;
        registerInfo->invertRegisters = irIterator->second->booleanValue;
        _registers.emplace(inputIndex, registerInfo);
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

void ModbusOut::configNodesStarted() {
  try {
    if (_socket.empty()) {
      _out->printError("Error: This node has no Modbus server assigned.");
      return;
    }

    Flows::PArray parameters = std::make_shared<Flows::Array>();
    parameters->reserve(2);
    parameters->push_back(std::make_shared<Flows::Variable>(_id));

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

void ModbusOut::input(const Flows::PNodeInfo &info, uint32_t index, const Flows::PVariable &message) {
  try {
    auto registersIterator = _registers.find(index);
    if (registersIterator == _registers.end()) return;

    Flows::PVariable payload = std::make_shared<Flows::Variable>();
    *payload = *(message->structValue->at("payload"));
    if (registersIterator->second->modbusType == ModbusType::tHoldingRegister) {
      if (payload->type == Flows::VariableType::tString) {
        payload->binaryValue.reserve(registersIterator->second->count * 2);
        payload->binaryValue.insert(payload->binaryValue.end(), payload->stringValue.begin(), payload->stringValue.end());
        payload->binaryValue.resize(registersIterator->second->count * 2, 0);
      } else if (payload->type == Flows::VariableType::tBoolean) payload->binaryValue.push_back((uint8_t)payload->booleanValue);
      else if (payload->type == Flows::VariableType::tInteger) {
        if (registersIterator->second->count == 1) {
          payload->binaryValue.reserve(2);
          payload->binaryValue.push_back((payload->integerValue >> 8) & 0xFF);
          payload->binaryValue.push_back(payload->integerValue & 0xFF);
        } else if (registersIterator->second->count == 2) {
          payload->binaryValue.reserve(4);
          payload->binaryValue.push_back(payload->integerValue >> 24);
          payload->binaryValue.push_back((payload->integerValue >> 16) & 0xFF);
          payload->binaryValue.push_back((payload->integerValue >> 8) & 0xFF);
          payload->binaryValue.push_back(payload->integerValue & 0xFF);
        } else if (registersIterator->second->count == 3) {
          payload->binaryValue.reserve(6);
          payload->binaryValue.push_back(0);
          payload->binaryValue.push_back(0);
          payload->binaryValue.push_back(payload->integerValue >> 24);
          payload->binaryValue.push_back((payload->integerValue >> 16) & 0xFF);
          payload->binaryValue.push_back((payload->integerValue >> 8) & 0xFF);
          payload->binaryValue.push_back(payload->integerValue & 0xFF);
        } else if (registersIterator->second->count == 4) {
          payload->binaryValue.reserve(8);
          payload->binaryValue.push_back(0);
          payload->binaryValue.push_back(0);
          payload->binaryValue.push_back(0);
          payload->binaryValue.push_back(0);
          payload->binaryValue.push_back(payload->integerValue >> 24);
          payload->binaryValue.push_back((payload->integerValue >> 16) & 0xFF);
          payload->binaryValue.push_back((payload->integerValue >> 8) & 0xFF);
          payload->binaryValue.push_back(payload->integerValue & 0xFF);
        }
      } else if (payload->type == Flows::VariableType::tInteger64) {
        if (registersIterator->second->count == 1) {
          payload->binaryValue.reserve(2);
          payload->binaryValue.push_back((payload->integerValue64 >> 8) & 0xFF);
          payload->binaryValue.push_back(payload->integerValue64 & 0xFF);
        } else if (registersIterator->second->count == 2) {
          payload->binaryValue.reserve(4);
          payload->binaryValue.push_back(payload->integerValue64 >> 24);
          payload->binaryValue.push_back((payload->integerValue64 >> 16) & 0xFF);
          payload->binaryValue.push_back((payload->integerValue64 >> 8) & 0xFF);
          payload->binaryValue.push_back(payload->integerValue64 & 0xFF);
        } else if (registersIterator->second->count == 3) {
          payload->binaryValue.reserve(6);
          payload->binaryValue.push_back((payload->integerValue64 >> 40) & 0xFF);
          payload->binaryValue.push_back((payload->integerValue64 >> 32) & 0xFF);
          payload->binaryValue.push_back((payload->integerValue64 >> 24) & 0xFF);
          payload->binaryValue.push_back((payload->integerValue64 >> 16) & 0xFF);
          payload->binaryValue.push_back((payload->integerValue64 >> 8) & 0xFF);
          payload->binaryValue.push_back(payload->integerValue64 & 0xFF);
        } else if (registersIterator->second->count == 4) {
          payload->binaryValue.reserve(8);
          payload->binaryValue.push_back(payload->integerValue64 >> 56);
          payload->binaryValue.push_back((payload->integerValue64 >> 48) & 0xFF);
          payload->binaryValue.push_back((payload->integerValue64 >> 40) & 0xFF);
          payload->binaryValue.push_back((payload->integerValue64 >> 32) & 0xFF);
          payload->binaryValue.push_back((payload->integerValue64 >> 24) & 0xFF);
          payload->binaryValue.push_back((payload->integerValue64 >> 16) & 0xFF);
          payload->binaryValue.push_back((payload->integerValue64 >> 8) & 0xFF);
          payload->binaryValue.push_back(payload->integerValue64 & 0xFF);
        }
      } else if (payload->type == Flows::VariableType::tFloat) {
        if (registersIterator->second->count == 2) {
          uint32_t floatValue = Flows::Math::getIeee754Binary32((float)payload->floatValue);
          payload->binaryValue.reserve(4);
          payload->binaryValue.push_back(floatValue >> 24);
          payload->binaryValue.push_back((floatValue >> 16) & 0xFF);
          payload->binaryValue.push_back((floatValue >> 8) & 0xFF);
          payload->binaryValue.push_back(floatValue & 0xFF);
        } else if (registersIterator->second->count == 4) {
          uint64_t doubleValue = Flows::Math::getIeee754Binary64(payload->floatValue);
          payload->binaryValue.push_back(doubleValue >> 56);
          payload->binaryValue.push_back((doubleValue >> 48) & 0xFF);
          payload->binaryValue.push_back((doubleValue >> 40) & 0xFF);
          payload->binaryValue.push_back((doubleValue >> 32) & 0xFF);
          payload->binaryValue.push_back((doubleValue >> 24) & 0xFF);
          payload->binaryValue.push_back((doubleValue >> 16) & 0xFF);
          payload->binaryValue.push_back((doubleValue >> 8) & 0xFF);
          payload->binaryValue.push_back(doubleValue & 0xFF);
        }
      }
      payload->setType(Flows::VariableType::tBinary);
      if (payload->binaryValue.empty()) return;

      Flows::PArray parameters = std::make_shared<Flows::Array>();
      parameters->reserve(6);
      parameters->push_back(std::make_shared<Flows::Variable>((int32_t)ModbusType::tHoldingRegister));
      parameters->push_back(std::make_shared<Flows::Variable>(registersIterator->second->index));
      parameters->push_back(std::make_shared<Flows::Variable>(registersIterator->second->count));
      parameters->push_back(std::make_shared<Flows::Variable>(registersIterator->second->invertBytes));
      parameters->push_back(std::make_shared<Flows::Variable>(registersIterator->second->invertRegisters));
      parameters->push_back(payload);

      invokeNodeMethod(_socket, "writeRegisters", parameters, false);
    } else {
      if (payload->type != Flows::VariableType::tBinary) {
        payload->binaryValue.push_back((bool)(*payload));
        payload->setType(Flows::VariableType::tBinary);
      } else if (payload->binaryValue.size() != 1) payload->binaryValue.resize(1);

      Flows::PArray parameters = std::make_shared<Flows::Array>();
      parameters->reserve(4);
      parameters->push_back(std::make_shared<Flows::Variable>((int32_t)ModbusType::tCoil));
      parameters->push_back(std::make_shared<Flows::Variable>(registersIterator->second->index));
      parameters->push_back(std::make_shared<Flows::Variable>(registersIterator->second->count));
      parameters->push_back(payload);

      invokeNodeMethod(_socket, "writeRegisters", parameters, false);
    }
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

//{{{ RPC methods
Flows::PVariable ModbusOut::setConnectionState(const Flows::PArray& parameters) {
  try {
    if (parameters->size() != 1) return Flows::Variable::createError(-1, "Method expects exactly one parameter. " + std::to_string(parameters->size()) + " given.");
    if (parameters->at(0)->type != Flows::VariableType::tBoolean) return Flows::Variable::createError(-1, "Parameter is not of type boolean.");

    Flows::PVariable status = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
    if (parameters->at(0)->booleanValue) {
      status->structValue->emplace("text", std::make_shared<Flows::Variable>("connected"));
      status->structValue->emplace("fill", std::make_shared<Flows::Variable>("green"));
      status->structValue->emplace("shape", std::make_shared<Flows::Variable>("dot"));
    } else {
      status->structValue->emplace("text", std::make_shared<Flows::Variable>("disconnected"));
      status->structValue->emplace("fill", std::make_shared<Flows::Variable>("red"));
      status->structValue->emplace("shape", std::make_shared<Flows::Variable>("dot"));
    }
    nodeEvent("statusBottom/" + _id, status, true);

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
//}}}

}
