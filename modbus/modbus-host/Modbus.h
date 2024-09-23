/* Copyright 2013-2019 Homegear GmbH
 *
 * Homegear is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * Homegear is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Homegear.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU Lesser General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
*/

#ifndef MODBUS_H_
#define MODBUS_H_

#include <homegear-node/Variable.h>
#include <homegear-node/Output.h>
#include <homegear-node/HelperFunctions.h>
#include <homegear-base/BaseLib.h>

namespace ModbusHost {

class Modbus {
 public:
  enum class ModbusType {
    tHoldingRegister = 0,
    tCoil = 1,
    tDiscreteInput = 2,
    tInputRegister = 3
  };

  struct ModbusSettings {
    std::string server;
    int32_t port = 502;
    uint32_t interval = 100;
    uint32_t delay = 0;
    bool keepAlive = true;
    uint8_t slaveId = 255;
    bool debug = false;
    std::vector<std::tuple<int32_t, int32_t, bool>> readRegisters;
    std::vector<std::tuple<int32_t, int32_t, bool>> readInputRegisters;
    std::vector<std::tuple<int32_t, int32_t, bool, bool>> writeRegisters;
    std::vector<std::tuple<int32_t, int32_t>> readCoils;
    std::vector<std::tuple<int32_t, int32_t, bool>> writeCoils;
    std::vector<std::tuple<int32_t, int32_t>> readDiscreteInputs;
  };

  Modbus(std::shared_ptr<BaseLib::SharedObjects> bl, std::shared_ptr<Flows::Output> output, std::shared_ptr<ModbusSettings> settings);

  ~Modbus();

  void start();

  void stop();

  void waitForStop();

  void setInvoke(std::function<Flows::PVariable(std::string, std::string, Flows::PArray &, bool)> value) { _invoke.swap(value); }

  void registerNode(std::string &node);

  void registerNode(std::string &node, ModbusType type, uint32_t startRegister, uint32_t count, bool invertBytes, bool invertRegisters, bool changesOnly);

  void registerNode(std::string &node, ModbusType type, uint32_t startCoil, uint32_t count, bool changesOnly);

  void writeRegisters(uint32_t startRegister, uint32_t count, bool invertBytes, bool invertRegisters, bool retry, std::vector<uint8_t> &value);

  void writeCoils(uint32_t startCoil, uint32_t count, bool retry, std::vector<uint8_t> &value);

 private:
  struct NodeInfo {
    ModbusType type = ModbusType::tHoldingRegister;
    std::string id;
    uint32_t startRegister = 0;
    uint32_t count = 0;
    bool invertBytes = false;
    bool invertRegisters = false;
  };

  struct RegisterInfo {
    std::atomic_bool newData;
    uint32_t start = 0;
    uint32_t end = 0;
    uint32_t count = 0;
    bool invert = false;
    bool readOnConnect = false;
    std::list<NodeInfo> nodes;
    std::vector<uint16_t> buffer1;
    std::vector<uint16_t> buffer2;
  };

  struct CoilInfo {
    std::atomic_bool newData;
    uint32_t start = 0;
    uint32_t end = 0;
    uint32_t count = 0;
    uint32_t byteCount = 0;
    bool readOnConnect = false;
    std::list<NodeInfo> nodes;
    std::vector<uint8_t> buffer1;
    std::vector<uint8_t> buffer2;
  };

  struct DiscreteInputInfo {
    uint32_t start = 0;
    uint32_t end = 0;
    uint32_t count = 0;
    uint32_t byteCount = 0;
    std::list<NodeInfo> nodes;
    std::vector<uint8_t> buffer1;
    std::vector<uint8_t> buffer2;
  };

  struct WriteInfo {
    uint32_t start = 0;
    uint32_t count = 0;
    bool invertBytes = false;
    bool invertRegisters = false;
    std::vector<uint8_t> value;
  };

  std::shared_ptr<BaseLib::SharedObjects> _bl;
  std::shared_ptr<Flows::Output> _out;
  std::shared_ptr<ModbusSettings> _settings;
  std::function<Flows::PVariable(std::string, std::string, Flows::PArray &, bool)> _invoke;

  std::mutex _modbusMutex;
  std::shared_ptr<BaseLib::Modbus> _modbus;
  std::atomic_bool _connected;

  std::thread _listenThread;
  std::atomic_bool _started;
  std::atomic_bool _outputChangesOnly{true};
  std::mutex _outputNodesMutex;
  std::unordered_set<std::string> _outputNodes;
  std::mutex _readRegistersMutex;
  std::list<std::shared_ptr<RegisterInfo>> _readRegisters;
  std::mutex _writeRegistersMutex;
  std::list<std::shared_ptr<RegisterInfo>> _writeRegisters;
  std::mutex _registerWriteBufferMutex;
  std::list<std::shared_ptr<WriteInfo>> _registerWriteBuffer;
  std::mutex _readInputRegistersMutex;
  std::list<std::shared_ptr<RegisterInfo>> _readInputRegisters;
  std::mutex _readCoilsMutex;
  std::list<std::shared_ptr<CoilInfo>> _readCoils;
  std::mutex _writeCoilsMutex;
  std::list<std::shared_ptr<CoilInfo>> _writeCoils;
  std::mutex _coilWriteBufferMutex;
  std::list<std::shared_ptr<WriteInfo>> _coilWriteBuffer;
  std::mutex _readDiscreteInputsMutex;
  std::list<std::shared_ptr<DiscreteInputInfo>> _readDiscreteInputs;

  Modbus(const Modbus &);

  Modbus &operator=(const Modbus &);

  void connect();

  void disconnect();

  void setConnectionState(bool connected);

  void listen();

  void readWriteRegister(std::shared_ptr<RegisterInfo> &info);

  void readWriteCoil(std::shared_ptr<CoilInfo> &info);

  void packetSent(const std::vector<char> &packet);

  void packetReceived(const std::vector<char> &packet);
};

}

#endif
