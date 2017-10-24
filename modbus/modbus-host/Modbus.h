/* Copyright 2013-2017 Sathya Laufer
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
#include <modbus/modbus.h>

class Modbus
{
public:
	struct ModbusSettings
	{
		std::string server;
		std::string port;
		uint32_t interval = 100;
	};

	Modbus(std::shared_ptr<BaseLib::SharedObjects> bl, std::shared_ptr<ModbusSettings> settings);
	virtual ~Modbus();

	void start();
	void stop();
	void waitForStop();

	void setInvoke(std::function<Flows::PVariable(std::string, std::string, Flows::PArray&)> value) { _invoke.swap(value); }

	void registerNode(std::string& node, uint32_t startRegister, uint32_t count, bool in);
private:
	struct NodeInfo
    {
        std::string id;
        uint32_t startRegister;
        uint32_t count;
    };

	std::shared_ptr<BaseLib::SharedObjects> _bl;
	BaseLib::Output _out;
	std::shared_ptr<ModbusSettings> _settings;
	std::function<Flows::PVariable(std::string, std::string, Flows::PArray&)> _invoke;

	std::mutex _modbusMutex;
	std::atomic<modbus_t*> _modbus;

	std::mutex _nodesMutex;
	std::list<NodeInfo> _inNodes;
    std::list<NodeInfo> _outNodes;
	std::thread _listenThread;
	std::atomic_bool _started;
    int32_t _inStartRegister = -1;
    int32_t _inEndRegister = -1;
    int32_t _inRegisterCount = -1;
    int32_t _outStartRegister = -1;
    int32_t _outEndRegister = -1;
    int32_t _outRegisterCount = -1;

    std::mutex _bufferMutex;
	std::vector<uint16_t> _writeBuffer;
	std::vector<uint16_t> _readBuffer;

	Modbus(const Modbus&);
	Modbus& operator=(const Modbus&);
	void connect();
	void disconnect();
    void setConnectionState(bool connected);
	void listen();
};

#endif
