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

#include "MyNode.h"

namespace MyNode
{

MyNode::MyNode(std::string path, std::string nodeNamespace, std::string type, const std::atomic_bool* frontendConnected) : Flows::INode(path, nodeNamespace, type, frontendConnected)
{
    _stopThread = false;

    _bl = std::make_shared<BaseLib::SharedObjects>();

	_localRpcMethods.emplace("registerNode", std::bind(&MyNode::registerNode, this, std::placeholders::_1));
	_localRpcMethods.emplace("write", std::bind(&MyNode::write, this, std::placeholders::_1));
}

MyNode::~MyNode()
{
}

bool MyNode::init(Flows::PNodeInfo info)
{
	try
	{
		_nodeInfo = info;
		return true;
	}
	catch(const std::exception& ex)
	{
		_out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		_out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
	return false;
}

bool MyNode::start()
{
	try
	{
		auto settingsIterator = _nodeInfo->info->structValue->find("serialport");
		if(settingsIterator != _nodeInfo->info->structValue->end()) _serialPort = settingsIterator->second->stringValue;

        if(_serialPort.empty())
        {
            _out->printError("Error: No serial device specified.");
            return false;
        }

		settingsIterator = _nodeInfo->info->structValue->find("serialbaud");
		if(settingsIterator != _nodeInfo->info->structValue->end()) _baudRate = Flows::Math::getNumber(settingsIterator->second->stringValue);

        if(_baudRate <= 0)
        {
            _out->printError("Error: Invalid baudrate specified.");
            return false;
        }

        settingsIterator = _nodeInfo->info->structValue->find("databits");
        if(settingsIterator != _nodeInfo->info->structValue->end())
        {
            int32_t bits = Flows::Math::getNumber(settingsIterator->second->stringValue);

            if(bits == 8) _dataBits = BaseLib::SerialReaderWriter::CharacterSize::Eight;
            else if(bits == 7) _dataBits = BaseLib::SerialReaderWriter::CharacterSize::Seven;
            else if(bits == 6) _dataBits = BaseLib::SerialReaderWriter::CharacterSize::Six;
            else if(bits == 5) _dataBits = BaseLib::SerialReaderWriter::CharacterSize::Five;
            else
            {
                _out->printError("Error: Invalid character size specified.");
                return false;
            }
        }

        settingsIterator = _nodeInfo->info->structValue->find("parity");
        if(settingsIterator != _nodeInfo->info->structValue->end())
        {
            _evenParity = false;
            _oddParity = false;
            _evenParity = (settingsIterator->second->stringValue == "even");
            _oddParity = (settingsIterator->second->stringValue == "odd");
        }

        settingsIterator = _nodeInfo->info->structValue->find("stopbits");
        if(settingsIterator != _nodeInfo->info->structValue->end()) _stopBits = Flows::Math::getNumber(settingsIterator->second->stringValue);

        settingsIterator = _nodeInfo->info->structValue->find("newline");
        if(settingsIterator != _nodeInfo->info->structValue->end())
        {
            _newLine = settingsIterator->second->stringValue.empty() ? '\n' : settingsIterator->second->stringValue.front();
            _timeout = Flows::Math::getNumber(settingsIterator->second->stringValue);
            if(_timeout < 0) _timeout = 1;
            else if(_timeout > 5000) _timeout = 5000;
            _fixedCount = Flows::Math::getNumber(settingsIterator->second->stringValue);
            if(_fixedCount < 1) _fixedCount = 1;
        }

        settingsIterator = _nodeInfo->info->structValue->find("bin");
        if(settingsIterator != _nodeInfo->info->structValue->end()) _binaryOutput = settingsIterator->second->booleanValue;

        settingsIterator = _nodeInfo->info->structValue->find("out");
        if(settingsIterator != _nodeInfo->info->structValue->end())
        {
            std::string& splitType = settingsIterator->second->stringValue;
            if(splitType == "no") _splitType = SplitType::no;
            else if(splitType == "char") _splitType = SplitType::character;
            else if(splitType == "time") _splitType = SplitType::timeout;
            else if(splitType == "count") _splitType = SplitType::fixedLength;
        }

        settingsIterator = _nodeInfo->info->structValue->find("addchar");
        if(settingsIterator != _nodeInfo->info->structValue->end()) _addCharacter = settingsIterator->second->booleanValue;

        _serial = std::make_shared<BaseLib::SerialReaderWriter>(_bl.get(), _serialPort, _baudRate, 0, true, -1);
        reopen();

        _stopThread = false;
        _bl->threadManager.start(_readThread, true, &MyNode::listenThread, this);

		return true;
	}
	catch(const std::exception& ex)
	{
		_out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		_out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
	return false;
}

void MyNode::stop()
{
	try
	{
        _stopThread = true;
	}
	catch(const std::exception& ex)
	{
		_out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		_out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
}

void MyNode::waitForStop()
{
	try
	{
        _stopThread = true;
        _bl->threadManager.join(_readThread);
	}
	catch(const std::exception& ex)
	{
		_out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		_out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
}

Flows::PVariable MyNode::getConfigParameterIncoming(std::string name)
{
	try
	{
		auto settingsIterator = _nodeInfo->info->structValue->find(name);
		if(settingsIterator != _nodeInfo->info->structValue->end()) return settingsIterator->second;
	}
	catch(const std::exception& ex)
	{
		_out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		_out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
	return std::make_shared<Flows::Variable>();
}

void MyNode::reopen()
{
    try
    {
        _serial->closeDevice();
        _serial->openDevice(_evenParity, _oddParity, false, _dataBits, _stopBits == 2);
    }
    catch(const std::exception& ex)
    {
        _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
        _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void MyNode::packetReceived(Flows::PVariable data)
{
    try
    {
        Flows::PArray parameters = std::make_shared<Flows::Array>();
        parameters->push_back(data);
        std::lock_guard<std::mutex> nodesGuard(_nodesMutex);
        for (auto& node : _nodes)
        {
            invokeNodeMethod(node, "packetReceived", parameters, false);
        }
    }
    catch(const std::exception& ex)
    {
        _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
        _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

void MyNode::listenThread()
{
    while(!_stopThread)
    {
        try
        {
            int32_t readBytes = 0;
            if(_splitType == SplitType::character)
            {
                std::string data;
                readBytes = _serial->readLine(data);

                if(readBytes > 0)
                {
                    if(_binaryOutput)
                    {
                        std::vector<char> buffer(data.begin(), data.end());
                        packetReceived(std::make_shared<Flows::Variable>(buffer));
                    }
                    else packetReceived(std::make_shared<Flows::Variable>(std::move(data)));
                }
            }
            else if(_splitType == SplitType::no)
            {
                char data;
                readBytes = _serial->readChar(data);

                if(readBytes > 0)
                {
                    if(_binaryOutput)
                    {
                        std::vector<char> buffer{data};
                        packetReceived(std::make_shared<Flows::Variable>(buffer));
                    }
                    else packetReceived(std::make_shared<Flows::Variable>(std::string{data}));
                }
            }
            else if(_splitType == SplitType::timeout)
            {
                std::vector<char> buffer;
                buffer.reserve(1024);
                char data;
                readBytes = 1;
                while(readBytes > 0)
                {
                    readBytes = _serial->readChar(data, _timeout);
                    if(readBytes > 0)
                    {
                        if(buffer.size() + 1 > buffer.capacity()) buffer.reserve(buffer.capacity() + 1024);
                        buffer.push_back(data);
                    }
                }

                if(readBytes > 0)
                {
                    if(_binaryOutput) packetReceived(std::make_shared<Flows::Variable>(buffer));
                    else packetReceived(std::make_shared<Flows::Variable>(std::string(buffer.begin(), buffer.end())));
                }
            }
            else if(_splitType == SplitType::fixedLength)
            {
                std::vector<char> buffer(_dataBuffer.begin(), _dataBuffer.end());
                buffer.reserve(1024);
                char data;
                readBytes = 1;
                while(readBytes > 0)
                {
                    readBytes = _serial->readChar(data);
                    if(readBytes > 0)
                    {
                        if(buffer.size() + 1 > buffer.capacity()) buffer.reserve(buffer.capacity() + 1024);
                        buffer.push_back(data);

                        if(buffer.size() >= (unsigned)_fixedCount)
                        {
                            if(_binaryOutput) packetReceived(std::make_shared<Flows::Variable>(buffer));
                            else packetReceived(std::make_shared<Flows::Variable>(std::string(buffer.begin(), buffer.end())));
                        }
                    }
                    else if(readBytes == 0)
                    {
                        _dataBuffer.clear();
                        _dataBuffer.insert(_dataBuffer.end(), buffer.begin(), buffer.end());
                    }
                }
            }

            if(readBytes == -1) reopen();
        }
        catch(const std::exception& ex)
        {
            _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
        }
        catch(...)
        {
            _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
        }
    }
}

//{{{ RPC methods
Flows::PVariable MyNode::registerNode(Flows::PArray parameters)
{
	try
	{
        if(parameters->size() != 1) return Flows::Variable::createError(-1, "Method expects exactly one parameter. " + std::to_string(parameters->size()) + " given.");
        if(parameters->at(0)->type != Flows::VariableType::tString || parameters->at(0)->stringValue.empty()) return Flows::Variable::createError(-1, "Parameter is not of type string.");

        std::lock_guard<std::mutex> nodesGuard(_nodesMutex);
        _nodes.emplace(parameters->at(0)->stringValue);

		return std::make_shared<Flows::Variable>();
	}
	catch(const std::exception& ex)
	{
		_out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		_out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
	return Flows::Variable::createError(-32500, "Unknown application error.");
}

Flows::PVariable MyNode::write(Flows::PArray parameters)
{
    try
    {
        if(parameters->size() != 1) return Flows::Variable::createError(-1, "Method expects exactly one parameter.");
        if(parameters->at(0)->type != Flows::VariableType::tString && parameters->at(0)->type != Flows::VariableType::tBinary) return Flows::Variable::createError(-1, "Parameter is not of type Binary or String.");
        if(parameters->at(0)->binaryValue.empty() && parameters->at(0)->stringValue.empty()) return Flows::Variable::createError(-1, "No data given.");


        if(parameters->at(0)->type == Flows::VariableType::tString) parameters->at(0)->binaryValue.insert(parameters->at(0)->binaryValue.end(), parameters->at(0)->stringValue.begin(), parameters->at(0)->stringValue.end());
        if(_addCharacter && _splitType == SplitType::character) parameters->at(0)->binaryValue.push_back(_newLine);
        _serial->writeData(parameters->at(0)->binaryValue);

        return std::make_shared<Flows::Variable>();
    }
    catch(const std::exception& ex)
    {
        _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
        _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
    return Flows::Variable::createError(-32500, "Unknown application error.");
}
//}}}

}
