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

#include "Modbus.h"

namespace ModbusHost {

Modbus::Modbus(std::shared_ptr<BaseLib::SharedObjects> bl, std::shared_ptr<Flows::Output> output, std::shared_ptr<ModbusSettings> settings) {
  try {
    _bl = bl;
    _out = output;
    _settings = settings;
    _started = false;
    _connected = false;

    BaseLib::Modbus::ModbusInfo modbusInfo;
    modbusInfo.hostname = _settings->server;
    modbusInfo.port = _settings->port;
    modbusInfo.keepAlive = true;
    modbusInfo.timeout = 15000;
    if (settings->debug) {
      modbusInfo.packetSentCallback = std::function<void(const std::vector<char> &packet)>(std::bind(&Modbus::packetSent, this, std::placeholders::_1));
      modbusInfo.packetReceivedCallback = std::function<void(const std::vector<char> &packet)>(std::bind(&Modbus::packetReceived, this, std::placeholders::_1));
    }

    _modbus = std::make_shared<BaseLib::Modbus>(_bl.get(), modbusInfo);

    for (auto &element: settings->readRegisters) {
      std::shared_ptr<RegisterInfo> info = std::make_shared<RegisterInfo>();
      info->newData = false;
      info->start = (uint32_t)std::get<0>(element);
      info->end = (uint32_t)std::get<1>(element);
      info->count = info->end - info->start + 1;
      info->invert = std::get<2>(element);
      info->buffer1.resize(info->count, 0);
      info->buffer2.resize(info->count, 0);
      _readRegisters.emplace_back(info);
    }

    for (auto &element: settings->writeRegisters) {
      std::shared_ptr<RegisterInfo> info = std::make_shared<RegisterInfo>();
      info->newData = false;
      info->start = (uint32_t)std::get<0>(element);
      info->end = (uint32_t)std::get<1>(element);
      info->count = info->end - info->start + 1;
      info->invert = std::get<2>(element);
      info->readOnConnect = std::get<3>(element);
      info->buffer1.resize(info->count, 0);
      _writeRegisters.emplace_back(info);
    }

    for (auto &element: settings->readInputRegisters) {
      std::shared_ptr<RegisterInfo> info = std::make_shared<RegisterInfo>();
      info->newData = false;
      info->start = (uint32_t)std::get<0>(element);
      info->end = (uint32_t)std::get<1>(element);
      info->count = info->end - info->start + 1;
      info->invert = std::get<2>(element);
      info->buffer1.resize(info->count, 0);
      info->buffer2.resize(info->count, 0);
      _readInputRegisters.emplace_back(info);
    }

    for (auto &element: settings->readCoils) {
      std::shared_ptr<CoilInfo> info = std::make_shared<CoilInfo>();
      info->newData = false;
      info->start = (uint32_t)std::get<0>(element);
      info->end = (uint32_t)std::get<1>(element);
      info->count = info->end - info->start + 1;
      info->byteCount = info->count / 8 + (info->count % 8 == 0 ? 0 : 1);
      info->buffer1.resize(info->byteCount, 0);
      info->buffer2.resize(info->byteCount, 0);
      _readCoils.emplace_back(info);
    }

    for (auto &element: settings->writeCoils) {
      std::shared_ptr<CoilInfo> info = std::make_shared<CoilInfo>();
      info->newData = false;
      info->start = (uint32_t)std::get<0>(element);
      info->end = (uint32_t)std::get<1>(element);
      info->count = info->end - info->start + 1;
      info->byteCount = info->count / 8 + (info->count % 8 == 0 ? 0 : 1);
      info->readOnConnect = std::get<2>(element);
      info->buffer1.resize(info->byteCount, 0);
      _writeCoils.emplace_back(info);
    }

    for (auto &element: settings->readDiscreteInputs) {
      std::shared_ptr<DiscreteInputInfo> info = std::make_shared<DiscreteInputInfo>();
      info->start = (uint32_t)std::get<0>(element);
      info->end = (uint32_t)std::get<1>(element);
      info->count = info->end - info->start + 1;
      info->byteCount = info->count / 8 + (info->count % 8 == 0 ? 0 : 1);
      info->buffer1.resize(info->byteCount, 0);
      info->buffer2.resize(info->byteCount, 0);
      _readDiscreteInputs.emplace_back(info);
    }
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

Modbus::~Modbus() {
  try {
    waitForStop();
    _modbus.reset();
    _bl.reset();
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void Modbus::start() {
  try {
    if (_started) return;
    _started = true;

    _bl->threadManager.start(_listenThread, true, &Modbus::listen, this);
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void Modbus::stop() {
  try {
    _started = false;
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void Modbus::waitForStop() {
  try {
    _started = false;
    _bl->threadManager.join(_listenThread);
    disconnect();
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void Modbus::packetSent(const std::vector<char> &packet) {
  _out->printInfo("Packet sent: " + BaseLib::HelperFunctions::getHexString(packet));
}

void Modbus::packetReceived(const std::vector<char> &packet) {
  _out->printInfo("Packet received: " + BaseLib::HelperFunctions::getHexString(packet));
}

void Modbus::readWriteRegister(std::shared_ptr<RegisterInfo> &info) {
  try {
    try {
      _modbus->readHoldingRegisters(info->start, info->buffer1, info->count);
    }
    catch (std::exception &ex) {
      _out->printError("Error reading from Modbus registers " + std::to_string(info->start) + " to " + std::to_string(info->end) + ": " + ex.what());
    }

    if (_settings->delay > 0) {
      if (_settings->delay <= 1000) std::this_thread::sleep_for(std::chrono::milliseconds(_settings->delay));
      else {
        int32_t maxIndex = _settings->delay / 1000;
        int32_t rest = _settings->delay % 1000;
        for (int32_t i = 0; i < maxIndex; i++) {
          std::this_thread::sleep_for(std::chrono::milliseconds(1000));
          if (!_started) return;
        }
        if (!_started) return;
        if (rest > 0) std::this_thread::sleep_for(std::chrono::milliseconds(rest));
      }
      if (!_started) return;
    }
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void Modbus::readWriteCoil(std::shared_ptr<CoilInfo> &info) {
  try {
    try {
      _modbus->readCoils(info->start, info->buffer1, info->count);
    }
    catch (std::exception &ex) {
      _out->printError("Error reading from Modbus coils " + std::to_string(info->start) + " to " + std::to_string(info->end) + ": " + ex.what());
    }

    if (_settings->delay > 0) {
      if (_settings->delay <= 1000) std::this_thread::sleep_for(std::chrono::milliseconds(_settings->delay));
      else {
        int32_t maxIndex = _settings->delay / 1000;
        int32_t rest = _settings->delay % 1000;
        for (int32_t i = 0; i < maxIndex; i++) {
          std::this_thread::sleep_for(std::chrono::milliseconds(1000));
          if (!_started) return;
        }
        if (!_started) return;
        if (rest > 0) std::this_thread::sleep_for(std::chrono::milliseconds(rest));
      }
      if (!_started) return;
    }
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void Modbus::listen() {
  int64_t startTime = BaseLib::HelperFunctions::getTimeMicroseconds();
  int64_t endTime = 0;
  int64_t timeToSleep;

  while (_started) {
    try {
      if (!_settings->keepAlive) {
        _modbus->disconnect();
      }
      if (_settings->interval > 0) {
        endTime = BaseLib::HelperFunctions::getTimeMicroseconds();
        timeToSleep = (_settings->interval * 1000) - (endTime - startTime);
        if (timeToSleep < 10000) timeToSleep = 10000;
        if (timeToSleep <= 1000000) std::this_thread::sleep_for(std::chrono::microseconds(timeToSleep));
        else {
          timeToSleep /= 1000;
          int32_t maxIndex = timeToSleep / 1000;
          int32_t rest = timeToSleep % 1000;
          for (int32_t i = 0; i < maxIndex; i++) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            if (!_started) break;
          }
          if (!_started) break;
          if (rest > 0) std::this_thread::sleep_for(std::chrono::milliseconds(rest));
        }
        startTime = BaseLib::HelperFunctions::getTimeMicroseconds();
      } else {
        //Automatic polling is disabled
        if (endTime > 0) {
          //When endTime is not 0, this is the second loop. We got here because of errors and need to return from this
          //function.
          _started = false;
          return;
        }
        endTime = BaseLib::HelperFunctions::getTimeMicroseconds();
      }
    }
    catch (const std::exception &ex) {
      _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }

    try {
      if (!_modbus->isConnected()) {
        if (!_started) return;
        connect();
        if (!_modbus->isConnected()) {
          _out->printWarning("Warning: Could not connect to Modbus host.");
          std::this_thread::sleep_for(std::chrono::milliseconds(2000));
          continue;
        }
        if (!_started) return;
      }

      std::list<std::shared_ptr<RegisterInfo>> registers;
      {
        std::lock_guard<std::mutex> writeRegistersGuard(_writeRegistersMutex);
        registers = _writeRegisters;
      }

      for (auto &registerElement: registers) {
        if (!registerElement->newData) continue;
        registerElement->newData = false;

        try {
          _modbus->writeMultipleRegisters(registerElement->start, registerElement->buffer1, registerElement->count);
        }
        catch (std::exception &ex) {
          _out->printError("Error writing to Modbus registers " + std::to_string(registerElement->start) + " to " + std::to_string(registerElement->end) + ": " + ex.what() + " - Disconnecting...");
          disconnect();
          break;
        }

        if (_settings->delay > 0) {
          if (_settings->delay <= 1000) std::this_thread::sleep_for(std::chrono::milliseconds(_settings->delay));
          else {
            int32_t maxIndex = _settings->delay / 1000;
            int32_t rest = _settings->delay % 1000;
            for (int32_t i = 0; i < maxIndex; i++) {
              std::this_thread::sleep_for(std::chrono::milliseconds(1000));
              if (!_started) break;
            }
            if (!_started) break;
            if (rest > 0) std::this_thread::sleep_for(std::chrono::milliseconds(rest));
          }
          if (!_started) break;
        }
      }
      if (!_modbus->isConnected()) {
        _out->printError("Error: Connection to server closed. Retrying after next interval.");
        continue;
      }

      {
        std::lock_guard<std::mutex> readRegistersGuard(_readRegistersMutex);
        registers = _readRegisters;
      }

      for (auto &registerElement: registers) {
        try {
          _modbus->readHoldingRegisters(registerElement->start, registerElement->buffer2, registerElement->count);
        }
        catch (std::exception &ex) {
          _out->printError("Error reading from Modbus holding registers " + std::to_string(registerElement->start) + " to " + std::to_string(registerElement->end) + ": " + ex.what() + " - Disconnecting...");
          disconnect();
          break;
        }

        if (!_outputChangesOnly || !std::equal(registerElement->buffer2.begin(), registerElement->buffer2.end(), registerElement->buffer1.begin())) {
          registerElement->buffer1 = registerElement->buffer2;

          std::vector<uint16_t> destinationData;
          std::vector<uint8_t> destinationData2;
          std::unordered_map<std::string, Flows::PVariable> data;

          for (auto &node: registerElement->nodes) {
            destinationData.clear();
            destinationData.insert(destinationData.end(), registerElement->buffer1.begin() + (node.startRegister - registerElement->start), registerElement->buffer1.begin() + (node.startRegister - registerElement->start) + node.count);
            destinationData2.resize(destinationData.size() * 2);

            if (node.invertRegisters) {
              for (uint32_t i = 0; i < destinationData.size(); i++) {
                if (registerElement->invert) {
                  if (node.invertBytes) {
                    destinationData2[i * 2] = destinationData[destinationData.size() - i - 1] >> 8;
                    destinationData2[(i * 2) + 1] = destinationData[destinationData.size() - i - 1] & 0xFF;
                  } else {
                    destinationData2[i * 2] = destinationData[destinationData.size() - i - 1] & 0xFF;
                    destinationData2[(i * 2) + 1] =
                        destinationData[destinationData.size() - i - 1] >> 8;
                  }
                } else {
                  if (node.invertBytes) {
                    destinationData2[i * 2] = destinationData[destinationData.size() - i - 1] & 0xFF;
                    destinationData2[(i * 2) + 1] =
                        destinationData[destinationData.size() - i - 1] >> 8;
                  } else {
                    destinationData2[i * 2] = destinationData[destinationData.size() - i - 1] >> 8;
                    destinationData2[(i * 2) + 1] = destinationData[destinationData.size() - i - 1] & 0xFF;
                  }
                }
              }
            } else {
              for (uint32_t i = 0; i < destinationData.size(); i++) {
                if (registerElement->invert) {
                  if (node.invertBytes) {
                    destinationData2[i * 2] = destinationData[i] >> 8;
                    destinationData2[(i * 2) + 1] = destinationData[i] & 0xFF;
                  } else {
                    destinationData2[i * 2] = destinationData[i] & 0xFF;
                    destinationData2[(i * 2) + 1] = destinationData[i] >> 8;
                  }
                } else {
                  if (node.invertBytes) {
                    destinationData2[i * 2] = destinationData[i] & 0xFF;
                    destinationData2[(i * 2) + 1] = destinationData[i] >> 8;
                  } else {
                    destinationData2[i * 2] = destinationData[i] >> 8;
                    destinationData2[(i * 2) + 1] = destinationData[i] & 0xFF;
                  }
                }
              }
            }

            Flows::PVariable dataElement = std::make_shared<Flows::Variable>(Flows::VariableType::tArray);
            dataElement->arrayValue->reserve(4);
            dataElement->arrayValue->push_back(std::make_shared<Flows::Variable>((int32_t)ModbusType::tHoldingRegister));
            dataElement->arrayValue->push_back(std::make_shared<Flows::Variable>(node.startRegister));
            dataElement->arrayValue->push_back(std::make_shared<Flows::Variable>(node.count));
            dataElement->arrayValue->push_back(std::make_shared<Flows::Variable>(destinationData2));
            auto dataIterator = data.find(node.id);
            if (dataIterator == data.end() || !dataIterator->second) data.emplace(node.id, std::make_shared<Flows::Variable>(Flows::PArray(new Flows::Array({dataElement}))));
            else dataIterator->second->arrayValue->push_back(dataElement);
          }

          Flows::PArray parameters = std::make_shared<Flows::Array>();
          parameters->push_back(std::make_shared<Flows::Variable>());
          for (auto &element: data) {
            parameters->at(0) = element.second;
            _invoke(element.first, "packetReceived", parameters, false);
          }
        }

        if (_settings->delay > 0) {
          if (_settings->delay < 1000) std::this_thread::sleep_for(std::chrono::milliseconds(_settings->delay));
          else {
            int32_t maxIndex = _settings->delay / 1000;
            int32_t rest = _settings->delay % 1000;
            for (int32_t i = 0; i < maxIndex; i++) {
              std::this_thread::sleep_for(std::chrono::milliseconds(1000));
              if (!_started) break;
            }
            if (!_started) break;
            if (rest > 0) std::this_thread::sleep_for(std::chrono::milliseconds(rest));
          }
          if (!_started) break;
        }
      }
      if (!_modbus->isConnected()) {
        _out->printError("Error: Connection to server closed. Retrying after next interval.");
        continue;
      }
      registers.clear();

      {
        std::lock_guard<std::mutex> readInputRegistersGuard(_readInputRegistersMutex);
        registers = _readInputRegisters;
      }

      for (auto &registerElement: registers) {
        try {
          _modbus->readInputRegisters(registerElement->start, registerElement->buffer2, registerElement->count);
        }
        catch (std::exception &ex) {
          _out->printError("Error reading from Modbus input registers " + std::to_string(registerElement->start) + " to " + std::to_string(registerElement->end) + ": " + ex.what() + " - Disconnecting...");
          disconnect();
          break;
        }

        if (!_outputChangesOnly || !std::equal(registerElement->buffer2.begin(), registerElement->buffer2.end(), registerElement->buffer1.begin())) {
          registerElement->buffer1 = registerElement->buffer2;

          std::vector<uint16_t> destinationData;
          std::vector<uint8_t> destinationData2;
          std::unordered_map<std::string, Flows::PVariable> data;

          for (auto &node: registerElement->nodes) {
            destinationData.clear();
            destinationData.insert(destinationData.end(), registerElement->buffer1.begin() + (node.startRegister - registerElement->start), registerElement->buffer1.begin() + (node.startRegister - registerElement->start) + node.count);
            destinationData2.resize(destinationData.size() * 2);

            if (node.invertRegisters) {
              for (uint32_t i = 0; i < destinationData.size(); i++) {
                if (registerElement->invert) {
                  if (node.invertBytes) {
                    destinationData2[i * 2] = destinationData[destinationData.size() - i - 1] >> 8;
                    destinationData2[(i * 2) + 1] = destinationData[destinationData.size() - i - 1] & 0xFF;
                  } else {
                    destinationData2[i * 2] = destinationData[destinationData.size() - i - 1] & 0xFF;
                    destinationData2[(i * 2) + 1] =
                        destinationData[destinationData.size() - i - 1] >> 8;
                  }
                } else {
                  if (node.invertBytes) {
                    destinationData2[i * 2] = destinationData[destinationData.size() - i - 1] & 0xFF;
                    destinationData2[(i * 2) + 1] =
                        destinationData[destinationData.size() - i - 1] >> 8;
                  } else {
                    destinationData2[i * 2] = destinationData[destinationData.size() - i - 1] >> 8;
                    destinationData2[(i * 2) + 1] = destinationData[destinationData.size() - i - 1] & 0xFF;
                  }
                }
              }
            } else {
              for (uint32_t i = 0; i < destinationData.size(); i++) {
                if (registerElement->invert) {
                  if (node.invertBytes) {
                    destinationData2[i * 2] = destinationData[i] >> 8;
                    destinationData2[(i * 2) + 1] = destinationData[i] & 0xFF;
                  } else {
                    destinationData2[i * 2] = destinationData[i] & 0xFF;
                    destinationData2[(i * 2) + 1] = destinationData[i] >> 8;
                  }
                } else {
                  if (node.invertBytes) {
                    destinationData2[i * 2] = destinationData[i] & 0xFF;
                    destinationData2[(i * 2) + 1] = destinationData[i] >> 8;
                  } else {
                    destinationData2[i * 2] = destinationData[i] >> 8;
                    destinationData2[(i * 2) + 1] = destinationData[i] & 0xFF;
                  }
                }
              }
            }

            Flows::PVariable dataElement = std::make_shared<Flows::Variable>(Flows::VariableType::tArray);
            dataElement->arrayValue->reserve(4);
            dataElement->arrayValue->push_back(std::make_shared<Flows::Variable>((int32_t)ModbusType::tInputRegister));
            dataElement->arrayValue->push_back(std::make_shared<Flows::Variable>(node.startRegister));
            dataElement->arrayValue->push_back(std::make_shared<Flows::Variable>(node.count));
            dataElement->arrayValue->push_back(std::make_shared<Flows::Variable>(destinationData2));
            auto dataIterator = data.find(node.id);
            if (dataIterator == data.end() || !dataIterator->second) data.emplace(node.id, std::make_shared<Flows::Variable>(Flows::PArray(new Flows::Array({dataElement}))));
            else dataIterator->second->arrayValue->push_back(dataElement);
          }

          Flows::PArray parameters = std::make_shared<Flows::Array>();
          parameters->push_back(std::make_shared<Flows::Variable>());
          for (auto &element: data) {
            parameters->at(0) = element.second;
            _invoke(element.first, "packetReceived", parameters, false);
          }
        }

        if (_settings->delay > 0) {
          if (_settings->delay < 1000) std::this_thread::sleep_for(std::chrono::milliseconds(_settings->delay));
          else {
            int32_t maxIndex = _settings->delay / 1000;
            int32_t rest = _settings->delay % 1000;
            for (int32_t i = 0; i < maxIndex; i++) {
              std::this_thread::sleep_for(std::chrono::milliseconds(1000));
              if (!_started) break;
            }
            if (!_started) break;
            if (rest > 0) std::this_thread::sleep_for(std::chrono::milliseconds(rest));
          }
          if (!_started) break;
        }
      }
      if (!_modbus->isConnected()) {
        _out->printError("Error: Connection to server closed. Retrying after next interval.");
        continue;
      }
      registers.clear();

      std::list<std::shared_ptr<CoilInfo>> coils;
      {
        std::lock_guard<std::mutex> writeCoilsGuard(_writeCoilsMutex);
        coils = _writeCoils;
      }

      for (auto &coilElement: coils) {
        if (!coilElement->newData) continue;
        coilElement->newData = false;

        try {
          _modbus->writeMultipleCoils(coilElement->start, coilElement->buffer1, coilElement->count);
        }
        catch (std::exception &ex) {
          _out->printError("Error writing Modbus coils " + std::to_string(coilElement->start) + " to " + std::to_string(coilElement->end) + ": " + ex.what() + " - Disconnecting...");
          disconnect();
          break;
        }

        if (_settings->delay > 0) {
          if (_settings->delay <= 1000) std::this_thread::sleep_for(std::chrono::milliseconds(_settings->delay));
          else {
            int32_t maxIndex = _settings->delay / 1000;
            int32_t rest = _settings->delay % 1000;
            for (int32_t i = 0; i < maxIndex; i++) {
              std::this_thread::sleep_for(std::chrono::milliseconds(1000));
              if (!_started) break;
            }
            if (!_started) break;
            if (rest > 0) std::this_thread::sleep_for(std::chrono::milliseconds(rest));
          }
          if (!_started) break;
        }
      }
      if (!_modbus->isConnected()) {
        _out->printError("Error: Connection to server closed. Retrying after next interval.");
        continue;
      }

      {
        std::lock_guard<std::mutex> readCoilsGuard(_readCoilsMutex);
        coils = _readCoils;
      }

      for (auto &coilElement: coils) {
        try {
          _modbus->readCoils(coilElement->start, coilElement->buffer2, coilElement->count);
        }
        catch (std::exception &ex) {
          _out->printError("Error reading from Modbus coils " + std::to_string(coilElement->start) + " to " + std::to_string(coilElement->end) + ": " + ex.what() + " - Disconnecting...");
          disconnect();
          break;
        }

        if (!_outputChangesOnly || !std::equal(coilElement->buffer2.begin(), coilElement->buffer2.end(), coilElement->buffer1.begin())) {
          coilElement->buffer1 = coilElement->buffer2;

          std::vector<uint8_t> destinationData;
          std::unordered_map<std::string, Flows::PVariable> data;

          for (auto &node: coilElement->nodes) {
            destinationData = BaseLib::BitReaderWriter::getPosition(coilElement->buffer1, node.startRegister - coilElement->start, node.count);

            Flows::PVariable dataElement = std::make_shared<Flows::Variable>(Flows::VariableType::tArray);
            dataElement->arrayValue->reserve(4);
            dataElement->arrayValue->push_back(std::make_shared<Flows::Variable>((int32_t)ModbusType::tCoil));
            dataElement->arrayValue->push_back(std::make_shared<Flows::Variable>(node.startRegister));
            dataElement->arrayValue->push_back(std::make_shared<Flows::Variable>(node.count));
            dataElement->arrayValue->push_back(std::make_shared<Flows::Variable>(destinationData));
            auto dataIterator = data.find(node.id);
            if (dataIterator == data.end() || !dataIterator->second) data.emplace(node.id, std::make_shared<Flows::Variable>(Flows::PArray(new Flows::Array({dataElement}))));
            else dataIterator->second->arrayValue->push_back(dataElement);
          }

          Flows::PArray parameters = std::make_shared<Flows::Array>();
          parameters->push_back(std::make_shared<Flows::Variable>());
          for (auto &element: data) {
            parameters->at(0) = element.second;
            _invoke(element.first, "packetReceived", parameters, false);
          }
        }

        if (_settings->delay > 0) {
          if (_settings->delay < 1000) std::this_thread::sleep_for(std::chrono::milliseconds(_settings->delay));
          else {
            int32_t maxIndex = _settings->delay / 1000;
            int32_t rest = _settings->delay % 1000;
            for (int32_t i = 0; i < maxIndex; i++) {
              std::this_thread::sleep_for(std::chrono::milliseconds(1000));
              if (!_started) break;
            }
            if (!_started) break;
            if (rest > 0) std::this_thread::sleep_for(std::chrono::milliseconds(rest));
          }
          if (!_started) break;
        }
      }
      if (!_modbus->isConnected()) {
        _out->printError("Error: Connection to server closed. Retrying after next interval.");
        continue;
      }

      std::list<std::shared_ptr<DiscreteInputInfo>> discreteInputs;
      {
        std::lock_guard<std::mutex> readDiscreteInputsGuard(_readDiscreteInputsMutex);
        discreteInputs = _readDiscreteInputs;
      }

      for (auto &discreteInputElement: discreteInputs) {
        try {
          _modbus->readDiscreteInputs(discreteInputElement->start, discreteInputElement->buffer2, discreteInputElement->count);
        }
        catch (std::exception &ex) {
          _out->printError("Error reading from Modbus discrete inputs " + std::to_string(discreteInputElement->start) + " to " + std::to_string(discreteInputElement->end) + ": " + ex.what() + " - Disconnecting...");
          disconnect();
          break;
        }

        if (!_outputChangesOnly || !std::equal(discreteInputElement->buffer2.begin(), discreteInputElement->buffer2.end(), discreteInputElement->buffer1.begin())) {
          discreteInputElement->buffer1 = discreteInputElement->buffer2;

          std::vector<uint8_t> destinationData;
          std::unordered_map<std::string, Flows::PVariable> data;

          for (auto &node: discreteInputElement->nodes) {
            destinationData = BaseLib::BitReaderWriter::getPosition(discreteInputElement->buffer1, node.startRegister - discreteInputElement->start, node.count);

            Flows::PVariable dataElement = std::make_shared<Flows::Variable>(Flows::VariableType::tArray);
            dataElement->arrayValue->reserve(4);
            dataElement->arrayValue->push_back(std::make_shared<Flows::Variable>((int32_t)ModbusType::tDiscreteInput));
            dataElement->arrayValue->push_back(std::make_shared<Flows::Variable>(node.startRegister));
            dataElement->arrayValue->push_back(std::make_shared<Flows::Variable>(node.count));
            dataElement->arrayValue->push_back(std::make_shared<Flows::Variable>(destinationData));
            auto dataIterator = data.find(node.id);
            if (dataIterator == data.end() || !dataIterator->second) data.emplace(node.id, std::make_shared<Flows::Variable>(Flows::PArray(new Flows::Array({dataElement}))));
            else dataIterator->second->arrayValue->push_back(dataElement);
          }

          Flows::PArray parameters = std::make_shared<Flows::Array>();
          parameters->push_back(std::make_shared<Flows::Variable>());
          for (auto &element: data) {
            parameters->at(0) = element.second;
            _invoke(element.first, "packetReceived", parameters, false);
          }
        }

        if (_settings->delay > 0) {
          if (_settings->delay < 1000) std::this_thread::sleep_for(std::chrono::milliseconds(_settings->delay));
          else {
            int32_t maxIndex = _settings->delay / 1000;
            int32_t rest = _settings->delay % 1000;
            for (int32_t i = 0; i < maxIndex; i++) {
              std::this_thread::sleep_for(std::chrono::milliseconds(1000));
              if (!_started) break;
            }
            if (!_started) break;
            if (rest > 0) std::this_thread::sleep_for(std::chrono::milliseconds(rest));
          }
          if (!_started) break;
        }
      }
      if (!_modbus->isConnected()) {
        _out->printError("Error: Connection to server closed. Retrying after next interval.");
        continue;
      }
    }
    catch (const std::exception &ex) {
      _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }

    if (_settings->interval == 0) {
      //Automatic polling is disabled
      _started = false;
      return;
    }
  }
}

void Modbus::setConnectionState(bool connected) {
  try {
    Flows::PArray parameters = std::make_shared<Flows::Array>();
    parameters->push_back(std::make_shared<Flows::Variable>(connected));

    {
      std::lock_guard<std::mutex> registersGuard(_readRegistersMutex);
      for (auto &element: _readRegisters) {
        for (auto &node: element->nodes) {
          _invoke(node.id, "setConnectionState", parameters, false);
        }
      }
    }

    {
      std::lock_guard<std::mutex> registersGuard(_writeRegistersMutex);
      for (auto &element: _writeRegisters) {
        for (auto &node: element->nodes) {
          _invoke(node.id, "setConnectionState", parameters, false);
        }
      }
    }

    {
      std::lock_guard<std::mutex> outputNodesGuard(_outputNodesMutex);
      for (auto &node: _outputNodes) {
        _invoke(node, "setConnectionState", parameters, false);
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

void Modbus::connect() {
  try {
    {
      std::lock_guard<std::mutex> modbusGuard(_modbusMutex);
      _modbus->setSlaveId(_settings->slaveId);
      _modbus->connect();
    }

    if (_settings->keepAlive) {
      std::list<std::shared_ptr<RegisterInfo>> registers;
      {
        std::lock_guard<std::mutex> writeRegistersGuard(_writeRegistersMutex);
        registers = _writeRegisters;
      }

      for (auto &registerElement: registers) {
        if (!registerElement->readOnConnect) continue;
        readWriteRegister(registerElement);
      }

      std::list<std::shared_ptr<CoilInfo>> coils;
      {
        std::lock_guard<std::mutex> writeCoilsGuard(_writeCoilsMutex);
        coils = _writeCoils;
      }

      for (auto &coilElement: coils) {
        if (!coilElement->readOnConnect) continue;
        readWriteCoil(coilElement);
      }
    }

    _connected = true;

    if (_settings->keepAlive) {
      {
        std::list<std::shared_ptr<WriteInfo>> writeBuffer;

        {
          std::lock_guard<std::mutex> writeBufferGuard(_registerWriteBufferMutex);
          writeBuffer = _registerWriteBuffer;
        }

        for (auto &element: writeBuffer) {
          writeRegisters(element->start, element->count, element->invertBytes, element->invertRegisters, true, element->value);
        }

        {
          std::lock_guard<std::mutex> writeBufferGuard(_registerWriteBufferMutex);
          _registerWriteBuffer.clear();
        }
      }

      {
        std::list<std::shared_ptr<WriteInfo>> writeBuffer;

        {
          std::lock_guard<std::mutex> writeBufferGuard(_coilWriteBufferMutex);
          writeBuffer = _coilWriteBuffer;
        }

        for (auto &element: writeBuffer) {
          writeCoils(element->start, element->count, true, element->value);
        }

        {
          std::lock_guard<std::mutex> writeBufferGuard(_coilWriteBufferMutex);
          _coilWriteBuffer.clear();
        }
      }
    }

    setConnectionState(true);
    return;
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
  setConnectionState(false);
}

void Modbus::disconnect() {
  try {
    std::lock_guard<std::mutex> modbusGuard(_modbusMutex);
    _connected = false;
    _modbus->disconnect();
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void Modbus::registerNode(std::string &node) {
  try {
    {
      std::lock_guard<std::mutex> outputNodesGuard(_outputNodesMutex);
      _outputNodes.emplace(node);
    }

    Flows::PArray parameters = std::make_shared<Flows::Array>();
    parameters->push_back(std::make_shared<Flows::Variable>(_modbus->isConnected()));
    _invoke(parameters->at(0)->stringValue, "setConnectionState", parameters, false);
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void Modbus::registerNode(std::string &node, ModbusType type, uint32_t startRegister, uint32_t count, bool invertBytes, bool invertRegisters, bool changesOnly) {
  try {
    if (!changesOnly) _outputChangesOnly = false;

    NodeInfo info;
    info.type = type;
    info.id = node;
    info.startRegister = startRegister;
    info.count = count;
    info.invertBytes = invertBytes;
    info.invertRegisters = invertRegisters;

    if (type == ModbusType::tHoldingRegister) {
      std::lock_guard<std::mutex> registersGuard(_readRegistersMutex);
      for (auto &element: _readRegisters) {
        if (startRegister >= element->start && (startRegister + count - 1) <= element->end) {
          element->nodes.emplace_back(info);
        }
      }
    } else if (type == ModbusType::tInputRegister) {
      std::lock_guard<std::mutex> registersGuard(_readInputRegistersMutex);
      for (auto &element: _readInputRegisters) {
        if (startRegister >= element->start && (startRegister + count - 1) <= element->end) {
          element->nodes.emplace_back(info);
        }
      }
    }

    Flows::PArray parameters = std::make_shared<Flows::Array>();
    parameters->push_back(std::make_shared<Flows::Variable>(_modbus->isConnected()));
    _invoke(parameters->at(0)->stringValue, "setConnectionState", parameters, false);
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void Modbus::registerNode(std::string &node, ModbusType type, uint32_t startCoil, uint32_t count, bool changesOnly) {
  try {
    if (!changesOnly) _outputChangesOnly = false;

    NodeInfo info;
    info.type = type;
    info.id = node;
    info.startRegister = startCoil;
    info.count = count;

    if (type == ModbusType::tCoil) {
      std::lock_guard<std::mutex> registersGuard(_readCoilsMutex);
      for (auto &element: _readCoils) {
        if (startCoil >= element->start && (startCoil + count - 1) <= element->end) {
          element->nodes.emplace_back(info);
        }
      }
    } else {
      std::lock_guard<std::mutex> registersGuard(_readDiscreteInputsMutex);
      for (auto &element: _readDiscreteInputs) {
        if (startCoil >= element->start && (startCoil + count - 1) <= element->end) {
          element->nodes.emplace_back(info);
        }
      }
    }

    Flows::PArray parameters = std::make_shared<Flows::Array>();
    parameters->push_back(std::make_shared<Flows::Variable>(_modbus->isConnected()));
    _invoke(parameters->at(0)->stringValue, "setConnectionState", parameters, false);
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void Modbus::writeRegisters(uint32_t startRegister, uint32_t count, bool invertBytes, bool invertRegisters, bool retry, std::vector<uint8_t> &value) {
  try {
    if (!_connected && !retry) {
      std::lock_guard<std::mutex> writeBufferGuard(_registerWriteBufferMutex);
      if (_registerWriteBuffer.size() > 10000) return;

      std::shared_ptr<WriteInfo> writeInfo = std::make_shared<WriteInfo>();
      writeInfo->start = startRegister;
      writeInfo->count = count;
      writeInfo->invertBytes = invertBytes;
      writeInfo->invertRegisters = invertRegisters;
      writeInfo->value = value;

      _registerWriteBuffer.push_back(writeInfo);
      return;
    }

    if (value.size() < count * 2) {
      std::vector<uint8_t> value2;
      value2.reserve(count * 2);
      value2.resize((count * 2) - value.size(), 0);
      value2.insert(value2.end(), value.begin(), value.end());
      value.swap(value2);
    }

    std::lock_guard<std::mutex> registersGuard(_writeRegistersMutex);
    for (auto &element: _writeRegisters) {
      if (startRegister >= element->start && (startRegister + count - 1) <= element->end) {
        element->newData = true;
        uint32_t buffer_offset = startRegister - element->start;
        if (invertRegisters) {
          for (uint32_t i = 0; i < count; i++) {
            if (element->invert) {
              if (invertBytes) element->buffer1[(buffer_offset + count) - i - 1] = (((uint16_t)value[i * 2]) << 8) | value[i * 2 + 1];
              else element->buffer1[(buffer_offset + count) - i - 1] = (((uint16_t)value[i * 2 + 1]) << 8) | value[i * 2];
            } else {
              if (invertBytes) element->buffer1[(buffer_offset + count) - i - 1] = (((uint16_t)value[i * 2 + 1]) << 8) | value[i * 2];
              else element->buffer1[(buffer_offset + count) - i - 1] = (((uint16_t)value[i * 2]) << 8) | value[i * 2 + 1];
            }
          }
        } else {
          for (uint32_t i = 0; i < count; i++) {
            if (element->invert) {
              if (invertBytes) element->buffer1[i + buffer_offset] = (((uint16_t)value[i * 2]) << 8) | value[i * 2 + 1];
              else element->buffer1[i + buffer_offset] = (((uint16_t)value[i * 2 + 1]) << 8) | value[i * 2];
            } else {
              if (invertBytes) element->buffer1[i + buffer_offset] = (((uint16_t)value[i * 2 + 1]) << 8) | value[i * 2];
              else element->buffer1[i + buffer_offset] = (((uint16_t)value[i * 2]) << 8) | value[i * 2 + 1];
            }
          }
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

void Modbus::writeCoils(uint32_t startCoil, uint32_t count, bool retry, std::vector<uint8_t> &value) {
  try {
    if (!_connected && !retry) {
      std::lock_guard<std::mutex> writeBufferGuard(_coilWriteBufferMutex);
      if (_coilWriteBuffer.size() > 10000) return;

      std::shared_ptr<WriteInfo> writeInfo = std::make_shared<WriteInfo>();
      writeInfo->start = startCoil;
      writeInfo->count = count;
      writeInfo->value = value;

      _coilWriteBuffer.push_back(writeInfo);
      return;
    }

    std::lock_guard<std::mutex> registersGuard(_writeCoilsMutex);
    for (auto &element: _writeCoils) {
      if (startCoil >= element->start && (startCoil + count - 1) <= element->end) {
        element->newData = true;
        for (uint32_t i = startCoil - element->start; i < (startCoil - element->start) + count; i++) {
          BaseLib::BitReaderWriter::setPositionLE(startCoil - element->start, count, element->buffer1, value);
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