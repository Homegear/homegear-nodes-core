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

    std::vector<uint16_t> readBuffer(_readBuffer.size(), 0);

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
                std::lock_guard<std::mutex> bufferGuard(_bufferMutex);
                if (_readBuffer.empty() || _inStartRegister == -1)
                {
                    if (!_writeBuffer.empty() && _outStartRegister != -1) result = modbus_write_registers(_modbus, _outStartRegister, _writeBuffer.size(), _writeBuffer.data());
                    else result = 0;

                    if (result == -1)
                    {
                        disconnect();
                        continue;
                    }
                }
                else
                {
                    if (readBuffer.size() != _readBuffer.size()) readBuffer.resize(_readBuffer.size(), 0);

                    //std::cerr << 'W' << BaseLib::HelperFunctions::getHexString(_writeBuffer) << std::endl;
                    if (!_writeBuffer.empty() && _outStartRegister != -1) result = modbus_write_and_read_registers(_modbus, _outStartRegister, _writeBuffer.size(), _writeBuffer.data(), _inStartRegister, readBuffer.size(), readBuffer.data());
                    else result = modbus_read_registers(_modbus, _inStartRegister, _readBuffer.size(), readBuffer.data());

                    if (result == -1)
                    {
                        disconnect();
                        continue;
                    }

                    if (!std::equal(readBuffer.begin(), readBuffer.end(), _readBuffer.begin()))
                    {
                        _readBuffer = readBuffer;

                        std::vector<uint16_t> destinationData;
                        std::vector<uint8_t> destinationData2;

                        Flows::PArray parameters = std::make_shared<Flows::Array>();
                        parameters->push_back(std::make_shared<Flows::Variable>());

                        std::lock_guard<std::mutex> nodesGuard(_nodesMutex);
                        for(auto& node : _inNodes)
                        {
                            destinationData.clear();
                            destinationData.insert(destinationData.end(), readBuffer.begin() + node.startRegister, readBuffer.begin() + node.startRegister + node.count);
                            destinationData2.resize(destinationData.size() * 2);

                            for(uint32_t i = 0; i < destinationData.size(); i++)
                            {
                                destinationData2[i * 2] = destinationData[i] >> 8;
                                destinationData2[(i * 2) + 1] = destinationData[i] & 0xFF;
                            }

                            _out.printInfo("Moin " + BaseLib::HelperFunctions::getHexString(destinationData2) + " " + BaseLib::HelperFunctions::getHexString(destinationData));

                            parameters->at(0) = std::make_shared<Flows::Variable>(destinationData2);

                            _invoke(node.id, "packetReceived", parameters, false);
                        }
                    }
                }
            }

            endTime = BaseLib::HelperFunctions::getTimeMicroseconds();
            timeToSleep = (_settings->interval * 1000) - (endTime - startTime);
            if(timeToSleep < 500) timeToSleep = 500;
            std::this_thread::sleep_for(std::chrono::microseconds(timeToSleep));
            startTime = endTime;
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

        std::lock_guard<std::mutex> nodesGuard(_nodesMutex);
        for(auto& node : _inNodes)
        {
            _invoke(node.id, "setConnectionState", parameters, false);
        }
        for(auto& node : _outNodes)
        {
            _invoke(node.id, "setConnectionState", parameters, false);
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

        {
            std::lock_guard<std::mutex> bufferGuard(_bufferMutex);
            _readBuffer.clear();
            _readBuffer.resize(_inRegisterCount);
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

void Modbus::registerNode(std::string& node, uint32_t startRegister, uint32_t count, bool in)
{
	try
	{
		std::lock_guard<std::mutex> nodesGuard(_nodesMutex);
        NodeInfo info;
        info.id = node;
        info.startRegister = startRegister;
        info.count = count;
		if(in) _inNodes.emplace_back(info);
        else _outNodes.emplace_back(info);

        {
            std::lock_guard<std::mutex> bufferGuard(_bufferMutex);
            if (in)
            {
                if (_inStartRegister == -1 || (int32_t)startRegister < _inStartRegister) _inStartRegister = startRegister;
                if (_inEndRegister == -1 || (int32_t)startRegister + (int32_t)count - 1 > _inEndRegister) _inEndRegister = startRegister + count - 1;
                _inRegisterCount = (_inEndRegister - _inStartRegister) + 1;
                _readBuffer.resize(_inRegisterCount);
            }
            else
            {
                if (_outStartRegister == -1 || (int32_t)startRegister < _outStartRegister) _outStartRegister = startRegister;
                if (_outEndRegister == -1 || (int32_t)startRegister + (int32_t)count - 1 > _outEndRegister) _outEndRegister = startRegister + count - 1;
                _outRegisterCount = (_outEndRegister - _outStartRegister) + 1;
                _writeBuffer.resize(_outRegisterCount);
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
