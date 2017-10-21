/* Copyright 2013-2017 Sathya Laufer
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

Modbus::Modbus(std::shared_ptr<BaseLib::SharedObjects> bl, std::shared_ptr<ModbusSettings> settings)
{
	try
	{
		_bl = bl;
		_settings = settings;
		_started = false;
	}
	catch(const std::exception& ex)
	{
		Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(BaseLib::Exception& ex)
	{
		Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
}

Modbus::~Modbus()
{
	try
	{
		waitForStop();
		_bl.reset();
	}
	catch(const std::exception& ex)
	{
		Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(BaseLib::Exception& ex)
	{
		Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
}

void Modbus::start()
{
	try
	{
		if(_started) return;
		_started = true;

		_bl->threadManager.start(_listenThread, true, &Modbus::listen, this);
	}
	catch(const std::exception& ex)
	{
		Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(BaseLib::Exception& ex)
	{
		Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
}

void Modbus::stop()
{
	try
	{
		_started = false;
	}
	catch(const std::exception& ex)
	{
		Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(BaseLib::Exception& ex)
	{
		Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
}

void Modbus::waitForStop()
{
	try
	{
		_started = false;
		_bl->threadManager.join(_listenThread);
		disconnect();
	}
	catch(const std::exception& ex)
	{
		Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(BaseLib::Exception& ex)
	{
		Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
}

void Modbus::listen()
{
	while(_started)
	{
		try
		{
			if(!_modbus)
			{
				if(!_started) return;
				connect();
				std::this_thread::sleep_for(std::chrono::milliseconds(2000));
				if(!_started) return;
				continue;
			}

			//Todo: Continue here
		}
		catch(const std::exception& ex)
		{
			Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
		}
		catch(BaseLib::Exception& ex)
		{
			Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
		}
		catch(...)
		{
			Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
		}
	}
}

void Modbus::connect()
{
	try
    {
		std::lock_guard<std::mutex> modbusGuard(_modbusMutex);
		if(_modbus)
		{
			modbus_close(_modbus);
			modbus_free(_modbus);
			_modbus = nullptr;
		}
		if(_settings->server.empty())
		{
			_out.printError("Error: Could not connect to BK90x0: Please set \"host\" in \"beckhoffbk90x0.conf\".");
			return;
		}
		_modbus = modbus_new_tcp(_settings->server.c_str(), BaseLib::Math::getNumber(_settings->port));
		if(!_modbus)
		{
			_out.printError("Error: Could not connect to BK90x0: Could not create modbus handle. Are hostname and port set correctly?");
			return;
		}
		int result = modbus_connect(_modbus);
		if(result == -1)
		{
			_out.printError("Error: Could not connect to BK90x0: " + std::string(modbus_strerror(errno)));
			modbus_free(_modbus);
			_modbus = nullptr;
			return;
		}

		_readBuffer.clear();
    }
    catch(const std::exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(BaseLib::Exception& ex)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
        _out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void Modbus::disconnect()
{
	try
	{
		std::lock_guard<std::mutex> modbusGuard(_modbusMutex);
		if(_modbus)
		{
			modbus_close(_modbus);
			modbus_free(_modbus);
			_modbus = nullptr;
		}
	}
	catch(const std::exception& ex)
	{
		Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(BaseLib::Exception& ex)
	{
		Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
}

void Modbus::registerNode(std::string& node, uint32_t startRegister, uint32_t count)
{
	try
	{
		std::lock_guard<std::mutex> nodesGuard(_nodesMutex);
		_nodes.emplace(node);

		//Todo: Set registers start address and count, lock and then resize buffers
	}
	catch(const std::exception& ex)
	{
		Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(BaseLib::Exception& ex)
	{
		Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
}
