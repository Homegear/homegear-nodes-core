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

#include "MyNode.h"

namespace MyNode
{

MyNode::MyNode(std::string path, std::string nodeNamespace, std::string type, const std::atomic_bool* frontendConnected) : Flows::INode(path, nodeNamespace, type, frontendConnected)
{
}

MyNode::~MyNode()
{
}

bool MyNode::init(Flows::PNodeInfo info)
{
	try
	{
		std::string variableType = "device";
		auto settingsIterator = info->info->structValue->find("variabletype");
		if(settingsIterator != info->info->structValue->end()) variableType = settingsIterator->second->stringValue;

		if(variableType == "device") _variableType = VariableType::device;
		else if(variableType == "metadata") _variableType = VariableType::metadata;
		else if(variableType == "system") _variableType = VariableType::system;
		else if(variableType == "flow") _variableType = VariableType::flow;
		else if(variableType == "global") _variableType = VariableType::global;

		if(_variableType == VariableType::device || _variableType == VariableType::metadata)
		{
			settingsIterator = info->info->structValue->find("peerid");
			if(settingsIterator != info->info->structValue->end()) _peerId = Flows::Math::getNumber64(settingsIterator->second->stringValue);
		}

		if(_variableType == VariableType::device)
		{
			settingsIterator = info->info->structValue->find("channel");
			if(settingsIterator != info->info->structValue->end()) _channel = Flows::Math::getNumber(settingsIterator->second->stringValue);
		}
		
		settingsIterator = info->info->structValue->find("variable");
		if(settingsIterator != info->info->structValue->end()) _variable = settingsIterator->second->stringValue;

		settingsIterator = info->info->structValue->find("eventsource");
		if(settingsIterator != info->info->structValue->end())
		{
			std::string eventSource = settingsIterator->second->stringValue;
			if(eventSource == "all") _eventSource = EventSource::all;
			else if(eventSource == "device") _eventSource = EventSource::device;
			else if(eventSource == "homegear") _eventSource = EventSource::homegear;
            else if(eventSource == "scriptengine") _eventSource = EventSource::scriptEngine;
            else if(eventSource == "nodeblue") _eventSource = EventSource::nodeBlue;
            else if(eventSource == "rpcClient") _eventSource = EventSource::rpcClient;
            else if(eventSource == "ipcclient") _eventSource = EventSource::ipcClient;
            else if(eventSource == "mqtt") _eventSource = EventSource::mqtt;
		}

		if(variableType == "device") _variableType = VariableType::device;
		else if(variableType == "metadata") _variableType = VariableType::metadata;
		else if(variableType == "system") _variableType = VariableType::system;
		else if(variableType == "flow") _variableType = VariableType::flow;
		else if(variableType == "global") _variableType = VariableType::global;

		settingsIterator = info->info->structValue->find("refractoryperiod");
		if(settingsIterator != info->info->structValue->end()) _refractionPeriod = Flows::Math::getNumber(settingsIterator->second->stringValue);

		settingsIterator = info->info->structValue->find("outputonstartup");
		if(settingsIterator != info->info->structValue->end()) _outputOnStartup = settingsIterator->second->booleanValue;

        settingsIterator = info->info->structValue->find("changes-only");
        if(settingsIterator != info->info->structValue->end()) _outputChangesOnly = settingsIterator->second->booleanValue;

		settingsIterator = info->info->structValue->find("loopprevention");
		if(settingsIterator != info->info->structValue->end()) _loopPrevention = settingsIterator->second->booleanValue;

		settingsIterator = info->info->structValue->find("looppreventiongroup");
		if(settingsIterator != info->info->structValue->end()) _loopPreventionGroup = settingsIterator->second->stringValue;

		if(_variableType == VariableType::device || _variableType == VariableType::metadata || _variableType == VariableType::system)
		{
			subscribePeer(_peerId, _channel, _variable);
		}
		else if(_variableType == VariableType::flow)
		{
			subscribeFlow();
		}
		else if(_variableType == VariableType::global)
		{
			subscribeGlobal();
		}

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

void MyNode::startUpComplete()
{
    try
    {
        if(_variableType == VariableType::device || _variableType == VariableType::metadata || _variableType == VariableType::system)
        {
            Flows::PArray parameters = std::make_shared<Flows::Array>();
            parameters->reserve(3);
            parameters->push_back(std::make_shared<Flows::Variable>(_peerId));
            parameters->push_back(std::make_shared<Flows::Variable>(_channel));
            parameters->push_back(std::make_shared<Flows::Variable>(_variable));
            auto payload = invoke("getValue", parameters);
            if(payload->errorStruct)
            {
                _out->printError("Error: Could not get value of variable: (Peer ID: " + std::to_string(_peerId) + ", channel: " + std::to_string(_channel) + ", name: " + _variable + ").");
            }
            else
            {
                _type = payload->type;

				if(_variableType == VariableType::device || _variableType == VariableType::metadata)
				{
					parameters->clear();
					parameters->push_back(std::make_shared<Flows::Variable>(_peerId));
					parameters->push_back(std::make_shared<Flows::Variable>(_channel));
					auto result = invoke("getName", parameters);
					if(result->stringValue.empty())
					{
						parameters->clear();
						parameters->push_back(std::make_shared<Flows::Variable>(_peerId));
						result = invoke("getName", parameters);
					}
					_name = result->stringValue;
				}

                if(_outputOnStartup)
                {
                    Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
                    message->structValue->emplace("eventSource", std::make_shared<Flows::Variable>("nodeBlue"));
                    message->structValue->emplace("peerId", std::make_shared<Flows::Variable>(_peerId));
                    message->structValue->emplace("channel", std::make_shared<Flows::Variable>(_channel));
                    message->structValue->emplace("variable", std::make_shared<Flows::Variable>(_variable));
                    if(!_name.empty()) message->structValue->emplace("peerName", std::make_shared<Flows::Variable>(_name));
                    message->structValue->emplace("payload", payload);

                    output(0, message);
                }
            }
        }
        else if(_variableType == VariableType::flow)
        {
            auto result = getFlowData(_variable);

            _type = result->type;

            if(_outputOnStartup)
            {
                Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
                message->structValue->emplace("eventSource", std::make_shared<Flows::Variable>("nodeBlue"));
                message->structValue->emplace("peerId", std::make_shared<Flows::Variable>(0));
                message->structValue->emplace("channel", std::make_shared<Flows::Variable>(-1));
                message->structValue->emplace("variable", std::make_shared<Flows::Variable>(_variable));
                message->structValue->emplace("payload", result);
                output(0, message);
            }
        }
        else if(_variableType == VariableType::global)
        {
            auto result = getGlobalData(_variable);

            _type = result->type;

            if(_outputOnStartup)
            {
                Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
                message->structValue->emplace("eventSource", std::make_shared<Flows::Variable>("nodeBlue"));
                message->structValue->emplace("peerId", std::make_shared<Flows::Variable>(0));
                message->structValue->emplace("channel", std::make_shared<Flows::Variable>(-1));
                message->structValue->emplace("variable", std::make_shared<Flows::Variable>(_variable));
                message->structValue->emplace("payload", result);
                output(0, message);
	        }
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

void MyNode::variableEvent(std::string source, uint64_t peerId, int32_t channel, std::string variable, Flows::PVariable value)
{
	try
	{
        frontendEventLog(source + " " + std::to_string((int32_t)_eventSource));
		if(_eventSource != EventSource::all)
		{
			if(source.compare(0, 7, "device-") == 0 && _eventSource != EventSource::device) return;
			else if(source.compare(0, 12, "scriptEngine") == 0 && _eventSource != EventSource::scriptEngine) return;
			else if(source.compare(0, 8, "nodeBlue") == 0 && _eventSource != EventSource::nodeBlue) return;
            else if(source.compare(0, 9, "ipcServer") == 0 && _eventSource != EventSource::ipcClient) return;
            else if(source.compare(0, 8, "homegear") == 0 && _eventSource != EventSource::homegear) return;
            else if(source.compare(0, 7, "client-") == 0 && _eventSource != EventSource::rpcClient) return;
            else if(source.compare(0, 4, "mqtt") == 0 && _eventSource != EventSource::mqtt) return;
		}

		if(Flows::HelperFunctions::getTime() - _lastInput < _refractionPeriod) return;
		_lastInput = Flows::HelperFunctions::getTime();

		if(_outputChangesOnly && _lastValue && (*_lastValue) == (*value)) return;
		_lastValue = value;

		Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
		message->structValue->emplace("eventSource", std::make_shared<Flows::Variable>(source));
		message->structValue->emplace("peerId", std::make_shared<Flows::Variable>(peerId));
		message->structValue->emplace("channel", std::make_shared<Flows::Variable>(channel));
		message->structValue->emplace("variable", std::make_shared<Flows::Variable>(variable));
		if(!_name.empty()) message->structValue->emplace("peerName", std::make_shared<Flows::Variable>(_name));
		message->structValue->emplace("payload", value);

		if(_loopPrevention && !_loopPreventionGroup.empty())
		{
			Flows::PArray parameters = std::make_shared<Flows::Array>();
			parameters->push_back(std::make_shared<Flows::Variable>(_id));
			Flows::PVariable result = invokeNodeMethod(_loopPreventionGroup, "event", parameters, true);
			if(result->errorStruct) _out->printError("Error calling \"event\": " + result->structValue->at("faultString")->stringValue);
			if(!result->booleanValue) return;
		}

		output(0, message);
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

void MyNode::flowVariableEvent(std::string flowId, std::string variable, Flows::PVariable value)
{
	try
	{
	    if(variable != _variable) return;
		if(Flows::HelperFunctions::getTime() - _lastInput < _refractionPeriod) return;
		_lastInput = Flows::HelperFunctions::getTime();

        if(_outputChangesOnly && _lastValue && (*_lastValue) == (*value)) return;
        _lastValue = value;

		Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
		message->structValue->emplace("variable", std::make_shared<Flows::Variable>(variable));
		message->structValue->emplace("payload", value);

		if(_loopPrevention && !_loopPreventionGroup.empty())
		{
			Flows::PArray parameters = std::make_shared<Flows::Array>();
			parameters->push_back(std::make_shared<Flows::Variable>(_id));
			Flows::PVariable result = invokeNodeMethod(_loopPreventionGroup, "event", parameters, true);
			if(result->errorStruct) _out->printError("Error calling \"event\": " + result->structValue->at("faultString")->stringValue);
			if(!result->booleanValue) return;
		}

		output(0, message);
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

void MyNode::globalVariableEvent(std::string variable, Flows::PVariable value)
{
	try
	{
        if(variable != _variable) return;
		if(Flows::HelperFunctions::getTime() - _lastInput < _refractionPeriod) return;
		_lastInput = Flows::HelperFunctions::getTime();

        if(_outputChangesOnly && _lastValue && (*_lastValue) == (*value)) return;
        _lastValue = value;

		Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
		message->structValue->emplace("variable", std::make_shared<Flows::Variable>(variable));
		message->structValue->emplace("payload", value);

		if(_loopPrevention && !_loopPreventionGroup.empty())
		{
			Flows::PArray parameters = std::make_shared<Flows::Array>();
			parameters->push_back(std::make_shared<Flows::Variable>(_id));
			Flows::PVariable result = invokeNodeMethod(_loopPreventionGroup, "event", parameters, true);
			if(result->errorStruct) _out->printError("Error calling \"event\": " + result->structValue->at("faultString")->stringValue);
			if(!result->booleanValue) return;
		}

		output(0, message);
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

void MyNode::input(const Flows::PNodeInfo info, uint32_t index, const Flows::PVariable message)
{
    try
    {
        if(_variableType == VariableType::device || _variableType == VariableType::metadata || _variableType == VariableType::system)
        {
            Flows::PArray parameters = std::make_shared<Flows::Array>();
            parameters->reserve(3);
            parameters->push_back(std::make_shared<Flows::Variable>(_peerId));
            parameters->push_back(std::make_shared<Flows::Variable>(_channel));
            parameters->push_back(std::make_shared<Flows::Variable>(_variable));
            auto payload = invoke("getValue", parameters);
            if(payload->errorStruct)
            {
                _out->printError("Error: Could not get type of variable: (Peer ID: " + std::to_string(_peerId) + ", channel: " + std::to_string(_channel) + ", name: " + _variable + ").");
            }
            else
            {
                Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
                message->structValue->emplace("eventSource", std::make_shared<Flows::Variable>("nodeBlue"));
                message->structValue->emplace("peerId", std::make_shared<Flows::Variable>(_peerId));
                message->structValue->emplace("channel", std::make_shared<Flows::Variable>(_channel));
                message->structValue->emplace("variable", std::make_shared<Flows::Variable>(_variable));
                if(!_name.empty()) message->structValue->emplace("peerName", std::make_shared<Flows::Variable>(_name));
                message->structValue->emplace("payload", payload);

                output(0, message);
            }
        }
        else if(_variableType == VariableType::flow)
        {
            auto result = getFlowData(_variable);

            Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
            message->structValue->emplace("eventSource", std::make_shared<Flows::Variable>("nodeBlue"));
            message->structValue->emplace("peerId", std::make_shared<Flows::Variable>(0));
            message->structValue->emplace("channel", std::make_shared<Flows::Variable>(-1));
            message->structValue->emplace("variable", std::make_shared<Flows::Variable>(_variable));
            message->structValue->emplace("payload", result);
            output(0, message);
        }
        else if(_variableType == VariableType::global)
        {
            auto result = getGlobalData(_variable);

            Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
            message->structValue->emplace("eventSource", std::make_shared<Flows::Variable>("nodeBlue"));
            message->structValue->emplace("peerId", std::make_shared<Flows::Variable>(0));
            message->structValue->emplace("channel", std::make_shared<Flows::Variable>(-1));
            message->structValue->emplace("variable", std::make_shared<Flows::Variable>(_variable));
            message->structValue->emplace("payload", result);
            output(0, message);
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

}
