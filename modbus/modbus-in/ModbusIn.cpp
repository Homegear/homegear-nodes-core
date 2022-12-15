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
#include "ModbusIn.h"

namespace ModbusIn {

ModbusIn::ModbusIn(const std::string &path, const std::string &type, const std::atomic_bool *frontendConnected) : Flows::INode(path, type, frontendConnected) {
  _localRpcMethods.emplace("packetReceived", std::bind(&ModbusIn::packetReceived, this, std::placeholders::_1));
  _localRpcMethods.emplace("setConnectionState", std::bind(&ModbusIn::setConnectionState, this, std::placeholders::_1));
}

ModbusIn::~ModbusIn() = default;

bool ModbusIn::init(const Flows::PNodeInfo &info) {
  try {
    _outputs = 0;

    int32_t outputIndex = -1;

    auto settingsIterator = info->info->structValue->find("server");
    if (settingsIterator != info->info->structValue->end()) _socket = settingsIterator->second->stringValue;

    settingsIterator = info->info->structValue->find("changes-only");
    if (settingsIterator != info->info->structValue->end()) _outputChangesOnly = settingsIterator->second->booleanValue;

    settingsIterator = info->info->structValue->find("registers");
    if (settingsIterator != info->info->structValue->end()) {
      for (auto &element : *settingsIterator->second->arrayValue) {
        outputIndex++;

        auto modbustypeIterator = element->structValue->find("mt");
        if (modbustypeIterator == element->structValue->end()) continue;

        auto indexIterator = element->structValue->find("r");
        if (indexIterator == element->structValue->end()) continue;

        auto countIterator = element->structValue->find("c");
        if (countIterator == element->structValue->end()) continue;

        auto typeIterator = element->structValue->find("t");
        if (typeIterator == element->structValue->end()) continue;

        auto ibIterator = element->structValue->find("ib");
        if (ibIterator == element->structValue->end()) continue;

        auto irIterator = element->structValue->find("ir");
        if (irIterator == element->structValue->end()) continue;

        auto nameIterator = element->structValue->find("n");
        if (nameIterator == element->structValue->end()) continue;

        int32_t index = Flows::Math::getNumber(indexIterator->second->stringValue);
        int32_t count = Flows::Math::getNumber(countIterator->second->stringValue);

        if (index < 0) continue;
        if (count < 1) count = 1;

        auto registerInfo = std::make_shared<RegisterInfo>();
        if (modbustypeIterator->second->type == Flows::VariableType::tInteger || modbustypeIterator->second->type == Flows::VariableType::tInteger64) registerInfo->modbusType = (ModbusType)modbustypeIterator->second->integerValue;
        else registerInfo->modbusType = (ModbusType)Flows::Math::getNumber(modbustypeIterator->second->stringValue);
        registerInfo->outputIndex = (uint32_t)outputIndex;
        registerInfo->index = (uint32_t)index;
        registerInfo->count = registerInfo->modbusType == ModbusType::tHoldingRegister || registerInfo->modbusType == ModbusType::tInputRegister ? (uint32_t)count : 1;
        registerInfo->name = nameIterator->second->stringValue;

        auto &type = typeIterator->second->stringValue;
        if (type == "bool") registerInfo->type = RegisterType::tBool;
        else if (type == "int") registerInfo->type = RegisterType::tInt;
        else if (type == "uint") registerInfo->type = RegisterType::tUInt;
        else if (type == "float" || type == "num") registerInfo->type = RegisterType::tFloat;
        else if (type == "string" || type == "str") registerInfo->type = RegisterType::tString;
        else registerInfo->type = RegisterType::tBin;

        registerInfo->invertBytes = ibIterator->second->booleanValue;
        registerInfo->invertRegisters = irIterator->second->booleanValue;
        if (registerInfo->modbusType == ModbusType::tHoldingRegister) _registers[registerInfo->index].emplace(registerInfo->count, registerInfo);
        else if (registerInfo->modbusType == ModbusType::tInputRegister) _inputRegisters[registerInfo->index].emplace(registerInfo->count, registerInfo);
        else if (registerInfo->modbusType == ModbusType::tCoil) _coils[registerInfo->index].emplace(registerInfo->count, registerInfo);
        else if (registerInfo->modbusType == ModbusType::tDiscreteInput) _discreteInputs[registerInfo->index].emplace(registerInfo->count, registerInfo);
        _outputs++;
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

void ModbusIn::configNodesStarted() {
  try {
    if (_socket.empty()) {
      _out->printError("Error: This node has no Modbus server assigned.");
      return;
    }

    Flows::PArray parameters = std::make_shared<Flows::Array>();
    parameters->reserve(2);
    parameters->push_back(std::make_shared<Flows::Variable>(_id));
    Flows::PVariable registers = std::make_shared<Flows::Variable>(Flows::VariableType::tArray);
    registers->arrayValue->reserve(_outputs);
    parameters->push_back(registers);

    for (auto &index : _registers) {
      for (auto &count : index.second) {
        Flows::PVariable element = std::make_shared<Flows::Variable>(Flows::VariableType::tArray);
        element->arrayValue->reserve(6);
        element->arrayValue->push_back(std::make_shared<Flows::Variable>((int32_t)ModbusType::tHoldingRegister));
        element->arrayValue->push_back(std::make_shared<Flows::Variable>(index.first));
        element->arrayValue->push_back(std::make_shared<Flows::Variable>(count.first));
        element->arrayValue->push_back(std::make_shared<Flows::Variable>(count.second->invertBytes));
        element->arrayValue->push_back(std::make_shared<Flows::Variable>(count.second->invertRegisters));
        element->arrayValue->push_back(std::make_shared<Flows::Variable>(_outputChangesOnly));
        registers->arrayValue->push_back(element);
      }
    }

    for (auto &index : _inputRegisters) {
      for (auto &count : index.second) {
        Flows::PVariable element = std::make_shared<Flows::Variable>(Flows::VariableType::tArray);
        element->arrayValue->reserve(6);
        element->arrayValue->push_back(std::make_shared<Flows::Variable>((int32_t)ModbusType::tInputRegister));
        element->arrayValue->push_back(std::make_shared<Flows::Variable>(index.first));
        element->arrayValue->push_back(std::make_shared<Flows::Variable>(count.first));
        element->arrayValue->push_back(std::make_shared<Flows::Variable>(count.second->invertBytes));
        element->arrayValue->push_back(std::make_shared<Flows::Variable>(count.second->invertRegisters));
        element->arrayValue->push_back(std::make_shared<Flows::Variable>(_outputChangesOnly));
        registers->arrayValue->push_back(element);
      }
    }

    for (auto &index : _coils) {
      for (auto &count : index.second) {
        Flows::PVariable element = std::make_shared<Flows::Variable>(Flows::VariableType::tArray);
        element->arrayValue->reserve(4);
        element->arrayValue->push_back(std::make_shared<Flows::Variable>((int32_t)ModbusType::tCoil));
        element->arrayValue->push_back(std::make_shared<Flows::Variable>(index.first));
        element->arrayValue->push_back(std::make_shared<Flows::Variable>(count.first));
        element->arrayValue->push_back(std::make_shared<Flows::Variable>(_outputChangesOnly));
        registers->arrayValue->push_back(element);
      }
    }

    for (auto &index : _discreteInputs) {
      for (auto &count : index.second) {
        Flows::PVariable element = std::make_shared<Flows::Variable>(Flows::VariableType::tArray);
        element->arrayValue->reserve(4);
        element->arrayValue->push_back(std::make_shared<Flows::Variable>((int32_t)ModbusType::tDiscreteInput));
        element->arrayValue->push_back(std::make_shared<Flows::Variable>(index.first));
        element->arrayValue->push_back(std::make_shared<Flows::Variable>(count.first));
        element->arrayValue->push_back(std::make_shared<Flows::Variable>(_outputChangesOnly));
        registers->arrayValue->push_back(element);
      }
    }

    if (!registers->arrayValue->empty()) {
      Flows::PVariable result = invokeNodeMethod(_socket, "registerNode", parameters, true);
      if (result->errorStruct) _out->printError("Error: Could not register node: " + result->structValue->at("faultString")->stringValue);
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
Flows::PVariable ModbusIn::packetReceived(const Flows::PArray& parameters) {
  try {
    if (parameters->size() != 1) return Flows::Variable::createError(-1, "Method expects exactly one parameter. " + std::to_string(parameters->size()) + " given.");
    if (parameters->at(0)->type != Flows::VariableType::tArray) return Flows::Variable::createError(-1, "Parameter 1 is not of type array.");

    for (auto &packet : *parameters->at(0)->arrayValue) {
      if (packet->arrayValue->size() != 4 || packet->arrayValue->at(3)->binaryValue.empty()) continue;

      auto modbusType = (ModbusType)packet->arrayValue->at(0)->integerValue;
      if (modbusType == ModbusType::tHoldingRegister || modbusType == ModbusType::tInputRegister) {
        auto indexIterator = modbusType == ModbusType::tHoldingRegister ? _registers.find(packet->arrayValue->at(1)->integerValue) : _inputRegisters.find(packet->arrayValue->at(1)->integerValue);
        if (indexIterator == _registers.end()) continue;

        auto countIterator = indexIterator->second.find(packet->arrayValue->at(2)->integerValue);
        if (countIterator == indexIterator->second.end()) continue;

        if (_outputChangesOnly && packet->arrayValue->at(3)->binaryValue == countIterator->second->lastValue) {
          continue;
        }
        countIterator->second->lastValue = packet->arrayValue->at(3)->binaryValue;

        Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
        message->structValue->emplace((ModbusType)packet->arrayValue->at(0)->integerValue == ModbusType::tHoldingRegister ? "holdingRegister" : "inputRegister", std::make_shared<Flows::Variable>(indexIterator->first));
        message->structValue->emplace("count", std::make_shared<Flows::Variable>(countIterator->first));
        message->structValue->emplace("name", std::make_shared<Flows::Variable>(countIterator->second->name));

        switch (countIterator->second->type) {
          case RegisterType::tBin:message->structValue->emplace("payload", packet->arrayValue->at(3));
            break;
          case RegisterType::tBool: {
            bool payload = false;
            for (auto &byte : packet->arrayValue->at(3)->binaryValue) {
              if ((bool)byte) {
                payload = true;
                break;
              }
            }
            message->structValue->emplace("payload", std::make_shared<Flows::Variable>(payload));
          }
            break;
          case RegisterType::tInt: {
            if (packet->arrayValue->at(3)->binaryValue.size() == 2) {
              int16_t number = (((int16_t)packet->arrayValue->at(3)->binaryValue.at(0)) << 8) | packet->arrayValue->at(3)->binaryValue.at(1);
              message->structValue->emplace("payload", std::make_shared<Flows::Variable>(number));
            } else if (packet->arrayValue->at(3)->binaryValue.size() == 4) {
              int32_t number = (((int32_t)packet->arrayValue->at(3)->binaryValue.at(0)) << 24) | (((int32_t)packet->arrayValue->at(3)->binaryValue.at(1)) << 16) | (((int32_t)packet->arrayValue->at(3)->binaryValue.at(2)) << 8)
                  | packet->arrayValue->at(3)->binaryValue.at(3);
              message->structValue->emplace("payload", std::make_shared<Flows::Variable>(number));
            } else if (packet->arrayValue->at(3)->binaryValue.size() == 8) {
              int64_t number = (((int64_t)packet->arrayValue->at(3)->binaryValue.at(0)) << 56) | (((int64_t)packet->arrayValue->at(3)->binaryValue.at(1)) << 48) | (((int64_t)packet->arrayValue->at(3)->binaryValue.at(2)) << 40)
                  | (((int64_t)packet->arrayValue->at(3)->binaryValue.at(3)) << 32) | (((int64_t)packet->arrayValue->at(3)->binaryValue.at(4)) << 24) | (((int64_t)packet->arrayValue->at(3)->binaryValue.at(5)) << 16)
                  | (((int64_t)packet->arrayValue->at(3)->binaryValue.at(6)) << 8) | packet->arrayValue->at(3)->binaryValue.at(7);
              message->structValue->emplace("payload", std::make_shared<Flows::Variable>(number));
            } else {
              message->structValue->emplace("invalid", std::make_shared<Flows::Variable>(true));
              message->structValue->emplace("hex", std::make_shared<Flows::Variable>(BaseLib::HelperFunctions::getHexString(packet->arrayValue->at(3)->binaryValue)));
            }
          }
            break;
          case RegisterType::tUInt: {
            if (packet->arrayValue->at(3)->binaryValue.size() == 2) {
              uint16_t number = (((uint16_t)packet->arrayValue->at(3)->binaryValue.at(0)) << 8) | packet->arrayValue->at(3)->binaryValue.at(1);
              message->structValue->emplace("payload", std::make_shared<Flows::Variable>(number));
            } else if (packet->arrayValue->at(3)->binaryValue.size() == 4) {
              uint32_t number = (((uint32_t)packet->arrayValue->at(3)->binaryValue.at(0)) << 24) | (((uint32_t)packet->arrayValue->at(3)->binaryValue.at(1)) << 16) | (((uint32_t)packet->arrayValue->at(3)->binaryValue.at(2)) << 8)
                  | packet->arrayValue->at(3)->binaryValue.at(3);
              message->structValue->emplace("payload", std::make_shared<Flows::Variable>(number));
            } else if (packet->arrayValue->at(3)->binaryValue.size() == 8) {
              uint64_t number = (((uint64_t)packet->arrayValue->at(3)->binaryValue.at(0)) << 56) | (((uint64_t)packet->arrayValue->at(3)->binaryValue.at(1)) << 48) | (((uint64_t)packet->arrayValue->at(3)->binaryValue.at(2)) << 40)
                  | (((uint64_t)packet->arrayValue->at(3)->binaryValue.at(3)) << 32) | (((uint64_t)packet->arrayValue->at(3)->binaryValue.at(4)) << 24) | (((uint64_t)packet->arrayValue->at(3)->binaryValue.at(5)) << 16)
                  | (((uint64_t)packet->arrayValue->at(3)->binaryValue.at(6)) << 8) | packet->arrayValue->at(3)->binaryValue.at(7);
              message->structValue->emplace("payload", std::make_shared<Flows::Variable>(number));
            } else {
              message->structValue->emplace("invalid", std::make_shared<Flows::Variable>(true));
              message->structValue->emplace("hex", std::make_shared<Flows::Variable>(BaseLib::HelperFunctions::getHexString(packet->arrayValue->at(3)->binaryValue)));
            }
          }
            break;
          case RegisterType::tFloat: {
            if (packet->arrayValue->at(3)->binaryValue.size() == 4) {
              uint32_t intNumber = (((uint32_t)packet->arrayValue->at(3)->binaryValue.at(0)) << 24) | (((uint32_t)packet->arrayValue->at(3)->binaryValue.at(1)) << 16) | (((uint32_t)packet->arrayValue->at(3)->binaryValue.at(2)) << 8)
                  | packet->arrayValue->at(3)->binaryValue.at(3);
              float number = Flows::Math::getFloatFromIeee754Binary32(intNumber);
              message->structValue->emplace("payload", std::make_shared<Flows::Variable>(number));
            } else if (packet->arrayValue->at(3)->binaryValue.size() == 8) {
              uint64_t intNumber = (((uint64_t)packet->arrayValue->at(3)->binaryValue.at(0)) << 56) | (((uint64_t)packet->arrayValue->at(3)->binaryValue.at(1)) << 48) | (((uint64_t)packet->arrayValue->at(3)->binaryValue.at(2)) << 40)
                  | (((uint64_t)packet->arrayValue->at(3)->binaryValue.at(3)) << 32) | (((uint64_t)packet->arrayValue->at(3)->binaryValue.at(4)) << 24) | (((uint64_t)packet->arrayValue->at(3)->binaryValue.at(5)) << 16)
                  | (((uint64_t)packet->arrayValue->at(3)->binaryValue.at(6)) << 8) | packet->arrayValue->at(3)->binaryValue.at(7);
              double number = Flows::Math::getDoubleFromIeee754Binary64(intNumber);
              message->structValue->emplace("payload", std::make_shared<Flows::Variable>(number));
            } else {
              message->structValue->emplace("invalid", std::make_shared<Flows::Variable>(true));
              message->structValue->emplace("hex", std::make_shared<Flows::Variable>(BaseLib::HelperFunctions::getHexString(packet->arrayValue->at(3)->binaryValue)));
            }
          }
            break;
          case RegisterType::tString:message->structValue->emplace("payload", std::make_shared<Flows::Variable>(std::string((char *)packet->arrayValue->at(3)->binaryValue.data(), packet->arrayValue->at(3)->binaryValue.size())));
            break;
        }

        output(countIterator->second->outputIndex, message);
      } else if (modbusType == ModbusType::tCoil) {
        auto indexIterator = _coils.find(packet->arrayValue->at(1)->integerValue);
        if (indexIterator == _coils.end()) continue;

        auto countIterator = indexIterator->second.find(packet->arrayValue->at(2)->integerValue);
        if (countIterator == indexIterator->second.end()) continue;

        if (_outputChangesOnly && packet->arrayValue->at(3)->binaryValue == countIterator->second->lastValue) continue;
        countIterator->second->lastValue = packet->arrayValue->at(3)->binaryValue;

        Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
        message->structValue->emplace("coil", std::make_shared<Flows::Variable>(indexIterator->first));
        message->structValue->emplace("payload", std::make_shared<Flows::Variable>((bool)packet->arrayValue->at(3)->binaryValue.back()));

        output(countIterator->second->outputIndex, message);
      } else {
        auto indexIterator = _discreteInputs.find(packet->arrayValue->at(1)->integerValue);
        if (indexIterator == _discreteInputs.end()) continue;

        auto countIterator = indexIterator->second.find(packet->arrayValue->at(2)->integerValue);
        if (countIterator == indexIterator->second.end()) continue;

        if (_outputChangesOnly && packet->arrayValue->at(3)->binaryValue == countIterator->second->lastValue) continue;
        countIterator->second->lastValue = packet->arrayValue->at(3)->binaryValue;

        Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
        message->structValue->emplace("discreteInput", std::make_shared<Flows::Variable>(indexIterator->first));
        message->structValue->emplace("payload", std::make_shared<Flows::Variable>((bool)packet->arrayValue->at(3)->binaryValue.back()));

        output(countIterator->second->outputIndex, message);
      }
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

Flows::PVariable ModbusIn::setConnectionState(const Flows::PArray& parameters) {
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
