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

        for(auto& element : settings->writeRegisters)
        {
            std::shared_ptr<RegisterInfo> info = std::make_shared<RegisterInfo>();
            info->newData = false;
            info->start = (uint32_t)std::get<0>(element);
            info->end = (uint32_t)std::get<1>(element);
            info->count = info->end - info->start + 1;
            info->invert = std::get<2>(element);
            info->buffer1.resize(info->count, 0);
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

            std::list<std::shared_ptr<RegisterInfo>> registers;
            {
                std::lock_guard<std::mutex> readRegistersGuard(_readRegistersMutex);
                registers = _readRegisters;
            }

            for(auto& registerElement : registers)
            {
                result = modbus_read_registers(_modbus, registerElement->start, registerElement->buffer2.size(), registerElement->buffer2.data());

                if (result == -1)
                {
                    Flows::Output::printError("Error reading from Modbus device: " + std::string(modbus_strerror(errno)) + " Disconnecting...");
                    disconnect();
                    continue;
                }

                if (!std::equal(registerElement->buffer2.begin(), registerElement->buffer2.end(), registerElement->buffer1.begin()))
                {
                    registerElement->buffer1 = registerElement->buffer2;
                    std::cout << "Moin2 " << BaseLib::HelperFunctions::getHexString(registerElement->buffer1) << std::endl;

                    std::vector<uint16_t> destinationData;
                    std::vector<uint8_t> destinationData2;
                    std::unordered_map<std::string, Flows::PVariable> data;

                    for (auto& node : registerElement->nodes)
                    {
                        destinationData.clear();
                        destinationData.insert(destinationData.end(), registerElement->buffer1.begin() + (node.startRegister - registerElement->start), registerElement->buffer1.begin() + (node.startRegister - registerElement->start) + node.count);
                        destinationData2.resize(destinationData.size() * 2);

                        if (node.invertRegisters)
                        {
                            for (uint32_t i = 0; i < destinationData.size(); i++)
                            {
                                if (registerElement->invert)
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
                                if (registerElement->invert)
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

                        Flows::PVariable dataElement = std::make_shared<Flows::Variable>(Flows::VariableType::tArray);
                        dataElement->arrayValue->reserve(3);
                        dataElement->arrayValue->push_back(std::make_shared<Flows::Variable>(node.startRegister));
                        dataElement->arrayValue->push_back(std::make_shared<Flows::Variable>(node.count));
                        dataElement->arrayValue->push_back(std::make_shared<Flows::Variable>(destinationData2));
                        auto dataIterator = data.find(node.id);
                        if (dataIterator == data.end() || !dataIterator->second) data.emplace(node.id, std::make_shared<Flows::Variable>(Flows::PArray(new Flows::Array({dataElement}))));
                        else dataIterator->second->arrayValue->push_back(dataElement);
                    }

                    Flows::PArray parameters = std::make_shared<Flows::Array>();
                    parameters->push_back(std::make_shared<Flows::Variable>());
                    for (auto& element : data)
                    {
                        parameters->at(0) = element.second;
                        _invoke(element.first, "packetReceived", parameters, false);
                    }
                }

                if (_settings->delay > 0)
                {
                    if (_settings->delay < 1000) std::this_thread::sleep_for(std::chrono::milliseconds(_settings->delay));
                    else
                    {
                        int32_t maxIndex = _settings->delay / 1000;
                        int32_t rest = _settings->delay % 1000;
                        for (int32_t i = 0; i < maxIndex; i++)
                        {
                            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                            if (!_started) break;
                        }
                        if (!_started) break;
                        if (rest > 0) std::this_thread::sleep_for(std::chrono::milliseconds(rest));
                    }
                    if (!_started) break;
                }
            }

            {
                std::lock_guard<std::mutex> writeRegistersGuard(_writeRegistersMutex);
                registers = _writeRegisters;
            }

            for(auto& registerElement : registers)
            {
                if(!registerElement->newData) continue;
                registerElement->newData = false;
                std::cout << "Moin " << BaseLib::HelperFunctions::getHexString(registerElement->buffer1) << std::endl;
                result = modbus_write_registers(_modbus, registerElement->start, registerElement->buffer1.size(), registerElement->buffer1.data());

                if (result == -1)
                {
                    Flows::Output::printError("Error writing to Modbus device: " + std::string(modbus_strerror(errno)) + " Disconnecting...");
                    disconnect();
                    continue;
                }

                if(_settings->delay > 0)
                {
                    if(_settings->delay <= 1000) std::this_thread::sleep_for(std::chrono::milliseconds(_settings->delay));
                    else
                    {
                        int32_t maxIndex = _settings->delay / 1000;
                        int32_t rest = _settings->delay % 1000;
                        for(int32_t i = 0; i < maxIndex; i++)
                        {
                            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                            if(!_started) break;
                        }
                        if(!_started) break;
                        if(rest > 0) std::this_thread::sleep_for(std::chrono::milliseconds(rest));
                    }
                    if(!_started) break;
                }
            }

            endTime = BaseLib::HelperFunctions::getTimeMicroseconds();
            timeToSleep = (_settings->interval * 1000) - (endTime - startTime);
            if(timeToSleep < 500) timeToSleep = 500;
            if(timeToSleep <= 1000000) std::this_thread::sleep_for(std::chrono::microseconds(timeToSleep));
            else
            {
                timeToSleep /= 1000;
                int32_t maxIndex = timeToSleep / 1000;
                int32_t rest = timeToSleep % 1000;
                for(int32_t i = 0; i < maxIndex; i++)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                    if(!_started) break;
                }
                if(!_started) break;
                if(rest > 0) std::this_thread::sleep_for(std::chrono::milliseconds(rest));
            }
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

        {
            std::lock_guard<std::mutex> registersGuard(_readRegistersMutex);
            for (auto& element : _readRegisters)
            {
                for (auto& node : element->nodes)
                {
                    _invoke(node.id, "setConnectionState", parameters, false);
                }
            }
        }

        {
            std::lock_guard<std::mutex> registersGuard(_writeRegistersMutex);
            for (auto& element : _writeRegisters)
            {
                for (auto& node : element->nodes)
                {
                    _invoke(node.id, "setConnectionState", parameters, false);
                }
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
			_out.printError("Error: Could not connect to Modbus device: Please set a host name.");
            setConnectionState(false);
			return;
		}
		_modbus = modbus_new_tcp(_settings->server.c_str(), BaseLib::Math::getNumber(_settings->port));
		if(!_modbus)
		{
			_out.printError("Error: Could not connect to Modbus device: Could not create modbus handle. Are hostname and port set correctly?");
            setConnectionState(false);
			return;
		}
		int result = modbus_connect(_modbus);
		if(result == -1)
        {
            _out.printError("Error: Could not connect to Modbus device: " + std::string(modbus_strerror(errno)));
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

void Modbus::registerNode(std::string& node, uint32_t startRegister, uint32_t count, bool invertBytes, bool invertRegisters)
{
	try
	{
        NodeInfo info;
        info.id = node;
        info.startRegister = startRegister;
        info.count = count;
        info.invertBytes = invertBytes;
        info.invertRegisters = invertRegisters;

        {
            std::lock_guard<std::mutex> registersGuard(_readRegistersMutex);
            for (auto& element : _readRegisters)
            {
                if (startRegister >= element->start && (startRegister + count - 1) <= element->end)
                {
                    element->nodes.emplace_back(info);
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

void Modbus::writeRegisters(uint32_t startRegister, uint32_t count, bool invertBytes, bool invertRegisters, std::vector<uint8_t> value)
{
    try
    {
        if(value.size() < count * 2)
        {
            std::vector<uint8_t> value2;
            value2.reserve(count * 2);
            value2.resize((count * 2) - value.size(), 0);
            value2.insert(value2.end(), value.begin(), value.end());
            value.swap(value2);
        }

        std::lock_guard<std::mutex> registersGuard(_writeRegistersMutex);
        for(auto& element : _writeRegisters)
        {
            if(startRegister >= element->start && (startRegister + count - 1) <= element->end)
            {
                element->newData = true;
                if (invertRegisters)
                {
                    for(uint32_t i = startRegister - element->start; i < (startRegister - element->start) + count; i++)
                    {
                        if(element->invert)
                        {
                            if(invertBytes) element->buffer1[((startRegister - element->start) + count) - i - 1] = (((uint16_t)value[i * 2]) << 8) | value[i * 2 + 1];
                            else element->buffer1[((startRegister - element->start) + count) - i - 1] = (((uint16_t)value[i * 2 + 1]) << 8) | value[i * 2];
                        }
                        else
                        {
                            if(invertBytes) element->buffer1[((startRegister - element->start) + count) - i - 1] = (((uint16_t)value[i * 2 + 1]) << 8) | value[i * 2];
                            else element->buffer1[((startRegister - element->start) + count) - i - 1] = (((uint16_t)value[i * 2]) << 8) | value[i * 2 + 1];
                        }
                    }
                }
                else
                {
                    for(uint32_t i = startRegister - element->start; i < (startRegister - element->start) + count; i++)
                    {
                        if(element->invert)
                        {
                            if(invertBytes) element->buffer1[i] = (((uint16_t)value[i * 2]) << 8) | value[i * 2 + 1];
                            else element->buffer1[i] = (((uint16_t)value[i * 2 + 1]) << 8) | value[i * 2];
                        }
                        else
                        {
                            if(invertBytes) element->buffer1[i] = (((uint16_t)value[i * 2 + 1]) << 8) | value[i * 2];
                            else element->buffer1[i] = (((uint16_t)value[i * 2]) << 8) | value[i * 2 + 1];
                        }
                    }
                }
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