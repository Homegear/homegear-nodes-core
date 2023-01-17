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

#include "ModbusHost.h"

#include <memory>

namespace ModbusHost {

ModbusHost::ModbusHost(const std::string &path, const std::string &type, const std::atomic_bool *frontendConnected) : Flows::INode(path, type, frontendConnected) {
  _localRpcMethods.emplace("registerNode", std::bind(&ModbusHost::registerNode, this, std::placeholders::_1));
  _localRpcMethods.emplace("triggerPoll", std::bind(&ModbusHost::triggerPoll, this, std::placeholders::_1));
  _localRpcMethods.emplace("writeRegisters", std::bind(&ModbusHost::writeRegisters, this, std::placeholders::_1));
}

ModbusHost::~ModbusHost() = default;

bool ModbusHost::init(const Flows::PNodeInfo &info) {
  _nodeInfo = info;
  return true;
}

bool ModbusHost::start() {
  try {
    std::shared_ptr<Modbus::ModbusSettings> modbusSettings = std::make_shared<Modbus::ModbusSettings>();

    auto settingsIterator = _nodeInfo->info->structValue->find("server");
    if (settingsIterator != _nodeInfo->info->structValue->end()) modbusSettings->server = settingsIterator->second->stringValue;

    settingsIterator = _nodeInfo->info->structValue->find("port");
    if (settingsIterator != _nodeInfo->info->structValue->end()) modbusSettings->port = Flows::Math::getNumber(settingsIterator->second->stringValue);

    settingsIterator = _nodeInfo->info->structValue->find("interval");
    if (settingsIterator != _nodeInfo->info->structValue->end()) {
      int64_t interval = Flows::Math::getNumber64(settingsIterator->second->stringValue);
      modbusSettings->interval = interval;
    }
    if (modbusSettings->interval < 0) modbusSettings->interval = 100;

    settingsIterator = _nodeInfo->info->structValue->find("delay");
    if (settingsIterator != _nodeInfo->info->structValue->end()) modbusSettings->delay = Flows::Math::getNumber(settingsIterator->second->stringValue);
    if (modbusSettings->delay > modbusSettings->interval) modbusSettings->delay = modbusSettings->interval;

    settingsIterator = _nodeInfo->info->structValue->find("slaveid");
    if (settingsIterator != _nodeInfo->info->structValue->end()) modbusSettings->slaveId = Flows::Math::getNumber(settingsIterator->second->stringValue);

    settingsIterator = _nodeInfo->info->structValue->find("keepalive");
    if (settingsIterator != _nodeInfo->info->structValue->end()) modbusSettings->keepAlive = settingsIterator->second->booleanValue;

    settingsIterator = _nodeInfo->info->structValue->find("debug");
    if (settingsIterator != _nodeInfo->info->structValue->end()) modbusSettings->debug = settingsIterator->second->booleanValue;

    settingsIterator = _nodeInfo->info->structValue->find("readregisters");
    if (settingsIterator != _nodeInfo->info->structValue->end()) {
      for (auto &element: *settingsIterator->second->arrayValue) {
        auto startIterator = element->structValue->find("s");
        if (startIterator == element->structValue->end()) continue;

        auto endIterator = element->structValue->find("e");
        if (endIterator == element->structValue->end()) continue;

        auto invIterator = element->structValue->find("inv");
        if (invIterator == element->structValue->end()) continue;

        int32_t start = Flows::Math::getNumber(startIterator->second->stringValue);
        int32_t end = Flows::Math::getNumber(endIterator->second->stringValue);

        if (start < 0 || end < 0 || end < start) continue;

        modbusSettings->readRegisters.emplace_back(std::make_tuple(start, end, invIterator->second->booleanValue));
      }
    }

    settingsIterator = _nodeInfo->info->structValue->find("writeregisters");
    if (settingsIterator != _nodeInfo->info->structValue->end()) {
      for (auto &element: *settingsIterator->second->arrayValue) {
        auto startIterator = element->structValue->find("s");
        if (startIterator == element->structValue->end()) continue;

        auto endIterator = element->structValue->find("e");
        if (endIterator == element->structValue->end()) continue;

        auto invIterator = element->structValue->find("inv");
        if (invIterator == element->structValue->end()) continue;

        auto rocIterator = element->structValue->find("roc");
        if (rocIterator == element->structValue->end()) continue;

        int32_t start = Flows::Math::getNumber(startIterator->second->stringValue);
        int32_t end = Flows::Math::getNumber(endIterator->second->stringValue);

        if (start < 0 || end < 0 || end < start) continue;

        modbusSettings->writeRegisters.emplace_back(std::make_tuple(start, end, invIterator->second->booleanValue, rocIterator->second->booleanValue));
      }
    }

    settingsIterator = _nodeInfo->info->structValue->find("readinputregisters");
    if (settingsIterator != _nodeInfo->info->structValue->end()) {
      for (auto &element: *settingsIterator->second->arrayValue) {
        auto startIterator = element->structValue->find("s");
        if (startIterator == element->structValue->end()) continue;

        auto endIterator = element->structValue->find("e");
        if (endIterator == element->structValue->end()) continue;

        auto invIterator = element->structValue->find("inv");
        if (invIterator == element->structValue->end()) continue;

        int32_t start = Flows::Math::getNumber(startIterator->second->stringValue);
        int32_t end = Flows::Math::getNumber(endIterator->second->stringValue);

        if (start < 0 || end < 0 || end < start) continue;

        modbusSettings->readInputRegisters.emplace_back(std::make_tuple(start, end, invIterator->second->booleanValue));
      }
    }

    settingsIterator = _nodeInfo->info->structValue->find("readcoils");
    if (settingsIterator != _nodeInfo->info->structValue->end()) {
      for (auto &element: *settingsIterator->second->arrayValue) {
        auto startIterator = element->structValue->find("s");
        if (startIterator == element->structValue->end()) continue;

        auto endIterator = element->structValue->find("e");
        if (endIterator == element->structValue->end()) continue;

        int32_t start = Flows::Math::getNumber(startIterator->second->stringValue);
        int32_t end = Flows::Math::getNumber(endIterator->second->stringValue);

        if (start < 0 || end < 0 || end < start) continue;

        modbusSettings->readCoils.emplace_back(std::make_tuple(start, end));
      }
    }

    settingsIterator = _nodeInfo->info->structValue->find("writecoils");
    if (settingsIterator != _nodeInfo->info->structValue->end()) {
      for (auto &element: *settingsIterator->second->arrayValue) {
        auto startIterator = element->structValue->find("s");
        if (startIterator == element->structValue->end()) continue;

        auto endIterator = element->structValue->find("e");
        if (endIterator == element->structValue->end()) continue;

        auto rocIterator = element->structValue->find("roc");
        if (rocIterator == element->structValue->end()) continue;

        int32_t start = Flows::Math::getNumber(startIterator->second->stringValue);
        int32_t end = Flows::Math::getNumber(endIterator->second->stringValue);

        if (start < 0 || end < 0 || end < start) continue;

        modbusSettings->writeCoils.emplace_back(std::make_tuple(start, end, rocIterator->second->booleanValue));
      }
    }

    settingsIterator = _nodeInfo->info->structValue->find("readdiscreteinputs");
    if (settingsIterator != _nodeInfo->info->structValue->end()) {
      for (auto &element: *settingsIterator->second->arrayValue) {
        auto startIterator = element->structValue->find("s");
        if (startIterator == element->structValue->end()) continue;

        auto endIterator = element->structValue->find("e");
        if (endIterator == element->structValue->end()) continue;

        int32_t start = Flows::Math::getNumber(startIterator->second->stringValue);
        int32_t end = Flows::Math::getNumber(endIterator->second->stringValue);

        if (start < 0 || end < 0 || end < start) continue;

        modbusSettings->readDiscreteInputs.emplace_back(std::make_tuple(start, end));
      }
    }

    std::shared_ptr<BaseLib::SharedObjects> bl = std::make_shared<BaseLib::SharedObjects>();
    _modbus = std::make_unique<Modbus>(bl, _out, modbusSettings);
    _modbus->setInvoke(std::function<Flows::PVariable(std::string, std::string, Flows::PArray &, bool)>(std::bind(&ModbusHost::invokeNodeMethod, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
    if (modbusSettings->interval > 0) _modbus->start();

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

void ModbusHost::stop() {
  try {
    if (_modbus) _modbus->stop();
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void ModbusHost::waitForStop() {
  try {
    if (_modbus) _modbus->waitForStop();
    _modbus.reset();
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

Flows::PVariable ModbusHost::getConfigParameterIncoming(const std::string &name) {
  try {
    auto settingsIterator = _nodeInfo->info->structValue->find(name);
    if (settingsIterator != _nodeInfo->info->structValue->end()) return settingsIterator->second;
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
  return std::make_shared<Flows::Variable>();
}

//{{{ RPC methods
Flows::PVariable ModbusHost::registerNode(const Flows::PArray &parameters) {
  try {
    if (parameters->size() != 1 && parameters->size() != 2) return Flows::Variable::createError(-1, "Method expects exactly three parameters. " + std::to_string(parameters->size()) + " given.");
    if (parameters->at(0)->type != Flows::VariableType::tString) return Flows::Variable::createError(-1, "Parameter 1 is not of type string.");
    if (parameters->size() == 2 && parameters->at(1)->type != Flows::VariableType::tArray) return Flows::Variable::createError(-1, "Parameter 2 is not of type array.");

    if (!_modbus) return Flows::Variable::createError(-32500, "Unknown application error.");
    if (parameters->size() == 1) {
      //modbus-out, modbus-trigger
      _modbus->registerNode(parameters->at(0)->stringValue);
    } else {
      //modbus-in
      for (auto &element: *parameters->at(1)->arrayValue) {
        if (element->arrayValue->size() == 6) {
          _modbus->registerNode(parameters->at(0)->stringValue,
                                (Modbus::ModbusType)element->arrayValue->at(0)->integerValue,
                                element->arrayValue->at(1)->integerValue,
                                element->arrayValue->at(2)->integerValue,
                                element->arrayValue->at(3)->booleanValue,
                                element->arrayValue->at(4)->booleanValue,
                                element->arrayValue->at(5)->booleanValue);
        } else if (element->arrayValue->size() == 4) {
          _modbus->registerNode(parameters->at(0)->stringValue,
                                (Modbus::ModbusType)element->arrayValue->at(0)->integerValue,
                                element->arrayValue->at(1)->integerValue,
                                element->arrayValue->at(2)->integerValue,
                                element->arrayValue->at(3)->booleanValue);
        }
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

Flows::PVariable ModbusHost::triggerPoll(const Flows::PArray &parameters) {
  try {
    if (!_modbus) return Flows::Variable::createError(-32500, "Unknown application error.");

    _modbus->start();

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

Flows::PVariable ModbusHost::writeRegisters(const Flows::PArray &parameters) {
  try {
    if (parameters->size() != 4 && parameters->size() != 6) return Flows::Variable::createError(-1, "Method expects four or six parameters. " + std::to_string(parameters->size()) + " given.");
    if (!_modbus) return Flows::Variable::createError(-32500, "Unknown application error.");

    if ((Modbus::ModbusType)parameters->at(0)->integerValue == Modbus::ModbusType::tHoldingRegister && parameters->size() == 6) {
      if (parameters->at(1)->type != Flows::VariableType::tInteger && parameters->at(1)->type != Flows::VariableType::tInteger64) return Flows::Variable::createError(-1, "Parameter 2 is not of type integer.");
      if (parameters->at(2)->type != Flows::VariableType::tInteger && parameters->at(2)->type != Flows::VariableType::tInteger64) return Flows::Variable::createError(-1, "Parameter 3 is not of type integer.");
      if (parameters->at(3)->type != Flows::VariableType::tBoolean) return Flows::Variable::createError(-1, "Parameter 4 is not of type boolean.");
      if (parameters->at(4)->type != Flows::VariableType::tBoolean) return Flows::Variable::createError(-1, "Parameter 5 is not of type boolean.");
      if (parameters->at(5)->type != Flows::VariableType::tBinary) return Flows::Variable::createError(-1, "Parameter 6 is not of type binary.");

      _modbus->writeRegisters(parameters->at(1)->integerValue, parameters->at(2)->integerValue, parameters->at(3)->booleanValue, parameters->at(4)->booleanValue, false, parameters->at(5)->binaryValue);
    } else {
      if (parameters->at(1)->type != Flows::VariableType::tInteger && parameters->at(1)->type != Flows::VariableType::tInteger64) return Flows::Variable::createError(-1, "Parameter 2 is not of type integer.");
      if (parameters->at(2)->type != Flows::VariableType::tInteger && parameters->at(2)->type != Flows::VariableType::tInteger64) return Flows::Variable::createError(-1, "Parameter 3 is not of type integer.");
      if (parameters->at(3)->type != Flows::VariableType::tBinary) return Flows::Variable::createError(-1, "Parameter 4 is not of type binary.");

      _modbus->writeCoils(parameters->at(1)->integerValue, parameters->at(2)->integerValue, false, parameters->at(3)->binaryValue);
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
//}}}

}
