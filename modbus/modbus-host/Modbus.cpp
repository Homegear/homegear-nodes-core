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
        _modbus = nullptr;

        for(auto& element : settings->readRegisters)
        {
            RegisterInfo info;
            info.start = (uint32_t)std::get<0>(element);
            info.end = (uint32_t)std::get<1>(element);
            info.count = info.end - info.start + 1;
            info.invert = std::get<2>(element);
            info.buffer1.resize(info.count, 0);
            info.buffer2.resize(info.count, 0);
            _readRegisters.emplace_back(info);
        }

        for(auto& element : settings->writeRegisters)
        {
            RegisterInfo info;
            info.start = (uint32_t)std::get<0>(element);
            info.end = (uint32_t)std::get<1>(element);
            info.count = info.end - info.start + 1;
            info.invert = std::get<2>(element);
            info.buffer1.resize(info.count, 0);
            _writeRegisters.emplace_back(info);
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
    int64_t startTime = BaseLib::HelperFunctions::getTimeMicroseconds();
    int64_t endTime;
    int64_t timeToSleep;
    int result = 0;

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

            {
                std::lock_guard<std::mutex> registersGuard(_registersMutex);
                for(auto& registerElement : _readRegisters)
                {
                    result = modbus_read_registers(_modbus, registerElement.start, registerElement.buffer2.size(), registerElement.buffer2.data());

                    if (result == -1)
                    {
                        Flows::Output::printError("Error reading from Modbus device: " + std::string(modbus_strerror(errno)) + " Disconnecting...");
                        disconnect();
                        continue;
                    }

                    if (!std::equal(registerElement.buffer2.begin(), registerElement.buffer2.end(), registerElement.buffer1.begin()))
                    {
                        registerElement.buffer1 = registerElement.buffer2;

                        std::vector<uint16_t> destinationData;
                        std::vector<uint8_t> destinationData2;

                        Flows::PArray parameters = std::make_shared<Flows::Array>();
                        parameters->push_back(std::make_shared<Flows::Variable>());

                        for(auto& node : registerElement.nodes)
                        {
                            destinationData.clear();
                            destinationData.insert(destinationData.end(), registerElement.buffer1.begin() + (node.startRegister - registerElement.start), registerElement.buffer1.begin() + (node.startRegister - registerElement.start) + node.count);
                            destinationData2.resize(destinationData.size() * 2);

                            if(node.invertRegisters)
                            {
                                for (uint32_t i = 0; i < destinationData.size(); i++)
                                {
                                    if (registerElement.invert)
                                    {
                                        if (node.invertBytes)
                                        {
                                            destinationData2[i * 2] = destinationData[destinationData.size() - i - 1] >> 8;
                                            destinationData2[(i * 2) + 1] = destinationData[destinationData.size() - i - 1] & 0xFF;
                                        }
                                        else
                                        {
                                            destinationData2[i * 2] = destinationData[destinationData.size() - i - 1] & 0xFF;
                                            destinationData2[(i * 2) + 1] = destinationData[destinationData.size() - i - 1] >> 8;
                                        }
                                    }
                                    else
                                    {
                                        if (node.invertBytes)
                                        {
                                            destinationData2[i * 2] = destinationData[destinationData.size() - i - 1] & 0xFF;
                                            destinationData2[(i * 2) + 1] = destinationData[destinationData.size() - i - 1] >> 8;
                                        }
                                        else
                                        {
                                            destinationData2[i * 2] = destinationData[destinationData.size() - i - 1] >> 8;
                                            destinationData2[(i * 2) + 1] = destinationData[destinationData.size() - i - 1] & 0xFF;
                                        }
                                    }
                                }
                            }
                            else
                            {
                                for (uint32_t i = 0; i < destinationData.size(); i++)
                                {
                                    if (registerElement.invert)
                                    {
                                        if (node.invertBytes)
                                        {
                                            destinationData2[i * 2] = destinationData[i] >> 8;
                                            destinationData2[(i * 2) + 1] = destinationData[i] & 0xFF;
                                        }
                                        else
                                        {
                                            destinationData2[i * 2] = destinationData[i] & 0xFF;
                                            destinationData2[(i * 2) + 1] = destinationData[i] >> 8;
                                        }
                                    }
                                    else
                                    {
                                        if (node.invertBytes)
                                        {
                                            destinationData2[i * 2] = destinationData[i] & 0xFF;
                                            destinationData2[(i * 2) + 1] = destinationData[i] >> 8;
                                        }
                                        else
                                        {
                                            destinationData2[i * 2] = destinationData[i] >> 8;
                                            destinationData2[(i * 2) + 1] = destinationData[i] & 0xFF;
                                        }
                                    }
                                }
                            }

                            parameters->at(0) = std::make_shared<Flows::Variable>(destinationData2);

                            _invoke(node.id, "packetReceived", parameters, false);
                        }
                    }

                    if(_settings->delay > 0) std::this_thread::sleep_for(std::chrono::milliseconds(_settings->delay));
                }

                for(auto& registerElement : _writeRegisters)
                {
                    if(!registerElement.newData) continue;
                    result = modbus_write_registers(_modbus, registerElement.start, registerElement.buffer1.size(), registerElement.buffer1.data());

                    if (result == -1)
                    {
                        Flows::Output::printError("Error writing to Modbus device: " + std::string(modbus_strerror(errno)) + " Disconnecting...");
                        disconnect();
                        continue;
                    }

                    if(_settings->delay > 0) std::this_thread::sleep_for(std::chrono::milliseconds(_settings->delay));
                }
            }

            endTime = BaseLib::HelperFunctions::getTimeMicroseconds();
            timeToSleep = (_settings->interval * 1000) - (endTime - startTime);
            if(timeToSleep < 500) timeToSleep = 500;
            std::this_thread::sleep_for(std::chrono::microseconds(timeToSleep));
            startTime = BaseLib::HelperFunctions::getTimeMicroseconds();
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

void Modbus::setConnectionState(bool connected)
{
    try
    {
        Flows::PArray parameters = std::make_shared<Flows::Array>();
        parameters->push_back(std::make_shared<Flows::Variable>(connected));

        std::lock_guard<std::mutex> registersGuard(_registersMutex);
        for(auto& element : _readRegisters)
        {
            for(auto& node : element.nodes)
            {
                _invoke(node.id, "setConnectionState", parameters, false);
            }
        }
        for(auto& element : _writeRegisters)
        {
            for(auto& node : element.nodes)
            {
                _invoke(node.id, "setConnectionState", parameters, false);
            }
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

void Modbus::connect()
{
    std::lock_guard<std::mutex> modbusGuard(_modbusMutex);
	try
    {
		if(_modbus)
		{
			modbus_close(_modbus);
			modbus_free(_modbus);
			_modbus = nullptr;
		}
		if(_settings->server.empty())
		{
			_out.printError("Error: Could not connect to BK90x0: Please set \"host\" in \"beckhoffbk90x0.conf\".");
            setConnectionState(false);
			return;
		}
		_modbus = modbus_new_tcp(_settings->server.c_str(), BaseLib::Math::getNumber(_settings->port));
		if(!_modbus)
		{
			_out.printError("Error: Could not connect to BK90x0: Could not create modbus handle. Are hostname and port set correctly?");
            setConnectionState(false);
			return;
		}
		int result = modbus_connect(_modbus);
		if(result == -1)
        {
            _out.printError("Error: Could not connect to BK90x0: " + std::string(modbus_strerror(errno)));
            modbus_free(_modbus);
            _modbus = nullptr;
            setConnectionState(false);
            return;
        }

        setConnectionState(true);
		return;
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
    if(_modbus)
    {
        modbus_free(_modbus);
        _modbus = nullptr;
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

void Modbus::registerNode(std::string& node, uint32_t startRegister, uint32_t count, bool in, bool invertBytes, bool invertRegisters)
{
	try
	{
        NodeInfo info;
        info.id = node;
        info.startRegister = startRegister;
        info.count = count;
        info.invertBytes = invertBytes;
        info.invertRegisters = invertRegisters;

        if(in)
        {
            std::lock_guard<std::mutex> registersGuard(_registersMutex);
            for(auto& element : _readRegisters)
            {
                if(startRegister >= element.start && (startRegister + count - 1) <= element.end)
                {
                    element.nodes.emplace_back(info);
                }
            }
        }
        else
        {
            std::lock_guard<std::mutex> registersGuard(_registersMutex);
            for(auto& element : _writeRegisters)
            {
                if(startRegister >= element.start && (startRegister + count - 1) <= element.end)
                {
                    element.nodes.emplace_back(info);
                }
            }
        }

        Flows::PArray parameters = std::make_shared<Flows::Array>();
        parameters->push_back(std::make_shared<Flows::Variable>((bool)_modbus));
        _invoke(parameters->at(0)->stringValue, "setConnectionState", parameters, false);
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
