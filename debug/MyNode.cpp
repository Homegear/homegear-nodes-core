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
		std::string console;
		auto settingsIterator = info->info->structValue->find("console");
		if(settingsIterator != info->info->structValue->end()) console = settingsIterator->second->stringValue;
		_hg = console == "hg" || console == "debtabhg";
		_debTabHg = console != "hg";

		settingsIterator = info->info->structValue->find("loglevel");
		if(settingsIterator != info->info->structValue->end()) _logLevel = Flows::Math::getNumber(settingsIterator->second->stringValue);

		settingsIterator = info->info->structValue->find("active");
		if(settingsIterator != info->info->structValue->end()) _active = settingsIterator->second->stringValue == "true" || settingsIterator->second->booleanValue;

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

void MyNode::setNodeVariable(const std::string& variable, Flows::PVariable value)
{
	try
	{
		if(variable == "active" && value && value->type == Flows::VariableType::tBoolean) _active = value->booleanValue;
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

std::string MyNode::stripNonPrintable(const std::string& s)
{
	std::string strippedString;
	strippedString.reserve(s.size());
	for(std::string::const_iterator i = s.begin(); i != s.end(); ++i)
	{
		if(std::isprint(*i, std::locale("en_US.UTF-8"))) strippedString.push_back(*i);
	}
	return strippedString;
}

void MyNode::input(const Flows::PNodeInfo info, uint32_t index, const Flows::PVariable message)
{
	try
	{
		if(!_active) return;
		if(!_hg && !*_frontendConnected) return;
		std::string property;
        Flows::PVariable myMessage = std::make_shared<Flows::Variable>();
        *myMessage = *message;
		auto completeIterator = info->info->structValue->find("complete");
		if(completeIterator == info->info->structValue->end() || completeIterator->second->stringValue == "false" || completeIterator->second->stringValue.empty())
		{
			auto payloadIterator = myMessage->structValue->find("payload");
			if(payloadIterator == myMessage->structValue->end()) return;
			property = "payload";
            myMessage = payloadIterator->second;
		}
		else if(completeIterator->second->stringValue != "true")
		{
			auto payloadIterator = myMessage->structValue->find(completeIterator->second->stringValue);
			if(payloadIterator == myMessage->structValue->end()) return;
			property = completeIterator->second->stringValue;
            myMessage = payloadIterator->second;
		}

		if(_hg)
		{
			std::string logLevel;
			if(_logLevel == 1) logLevel = "critical";
			else if(_logLevel == 2) logLevel = "error";
			else if(_logLevel == 3) logLevel = "warning";
			else if(_logLevel == 4) logLevel = "info";
			else if(_logLevel == 5) logLevel = "debug";

			_out->printMessage("Debug node output (" + logLevel + "): " + myMessage->print(false, false, true), _logLevel);
			if(!*_frontendConnected) return;
		}

		if(_debTabHg)
		{
			Flows::PVariable object = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
			object->structValue->emplace("id", std::make_shared<Flows::Variable>(_id));
			object->structValue->emplace("name", std::make_shared<Flows::Variable>(_type));
			object->structValue->emplace("msg", myMessage);

			std::string format;
			switch(myMessage->type)
			{
			case Flows::VariableType::tArray:
				format = "array[" + std::to_string(myMessage->arrayValue->size()) + "]";
				break;
			case Flows::VariableType::tBoolean:
				format = "boolean";
				break;
			case Flows::VariableType::tFloat:
				format = "number";
				break;
			case Flows::VariableType::tInteger:
				format = "number";
				break;
			case Flows::VariableType::tInteger64:
				format = "number";
				break;
			case Flows::VariableType::tString:
				format = "string[" + std::to_string(myMessage->stringValue.size()) + "]";
				if(myMessage->stringValue.size() > 1000) myMessage->stringValue = myMessage->stringValue.substr(0, 1000) + "...";
                myMessage->stringValue = stripNonPrintable(myMessage->stringValue);
				break;
			case Flows::VariableType::tStruct:
				format = "Object";
				break;
			case Flows::VariableType::tBase64:
				format = "string[" + std::to_string(myMessage->stringValue.size()) + "]";
				if(myMessage->stringValue.size() > 1000) myMessage->stringValue = myMessage->stringValue.substr(0, 1000) + "...";
                myMessage->stringValue = stripNonPrintable(myMessage->stringValue);
				break;
			case Flows::VariableType::tVariant:
				break;
			case Flows::VariableType::tBinary:
				break;
			case Flows::VariableType::tVoid:
				format = "null";
				break;
			}

			if(!format.empty()) object->structValue->emplace("format", std::make_shared<Flows::Variable>(format));
			if(!property.empty()) object->structValue->emplace("property", std::make_shared<Flows::Variable>(property));
			nodeEvent("debug", object);
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
