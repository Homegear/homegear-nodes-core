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

#ifndef MODBUSIN_H_
#define MODBUSIN_H_

#include <homegear-base/BaseLib.h>
#include <homegear-node/INode.h>
#include <unordered_map>

namespace ModbusIn {

class ModbusIn : public Flows::INode {
 public:
  ModbusIn(const std::string &path, const std::string &type, const std::atomic_bool *frontendConnected);
  ~ModbusIn() override;

  bool init(const Flows::PNodeInfo &info) override;
  void configNodesStarted() override;
 private:
  enum class ModbusType {
    tHoldingRegister = 0,
    tCoil = 1,
    tDiscreteInput = 2,
    tInputRegister = 3
  };

  enum class RegisterType {
    tBin,
    tBool,
    tInt,
    tUInt,
    tFloat,
    tString
  };

  struct RegisterInfo {
    ModbusType modbusType = ModbusType::tHoldingRegister;
    uint32_t outputIndex = 0;
    uint32_t index = 0;
    uint32_t count = 0;
    RegisterType type = RegisterType::tBin;
    bool invertBytes = false;
    bool invertRegisters = false;
    std::vector<uint8_t> lastValue;
    std::string name;
  };

  std::string _socket;
  bool _outputChangesOnly = true;
  uint32_t _outputs = 0;
  std::unordered_map<uint32_t, std::unordered_map<uint32_t, std::shared_ptr<RegisterInfo>>> _registers;
  std::unordered_map<uint32_t, std::unordered_map<uint32_t, std::shared_ptr<RegisterInfo>>> _coils;
  std::unordered_map<uint32_t, std::unordered_map<uint32_t, std::shared_ptr<RegisterInfo>>> _discreteInputs;
  std::unordered_map<uint32_t, std::unordered_map<uint32_t, std::shared_ptr<RegisterInfo>>> _inputRegisters;

  //{{{ RPC methods
  Flows::PVariable packetReceived(const Flows::PArray& parameters);
  Flows::PVariable setConnectionState(const Flows::PArray& parameters);
  //}}}
};

}

#endif
