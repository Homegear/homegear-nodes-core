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
}

MyNode::~MyNode()
{
}

bool MyNode::init(Flows::PNodeInfo info)
{
	try
	{
		std::string payloadType = "int";
		auto settingsIterator = info->info->structValue->find("payloadType");
		if(settingsIterator != info->info->structValue->end()) payloadType = settingsIterator->second->stringValue;

		std::string payload;
		settingsIterator = info->info->structValue->find("payload");
		if(settingsIterator != info->info->structValue->end()) payload = settingsIterator->second->stringValue;

		_value = std::make_shared<Flows::Variable>();
		if(payloadType == "bool")
		{
			_value->setType(Flows::VariableType::tBoolean);
			_value->booleanValue = payload == "true";
		}
		else if(payloadType == "int")
		{
			_value->setType(Flows::VariableType::tInteger64);
			_value->integerValue64 = Flows::Math::getNumber64(payload);
			_value->integerValue = (int32_t)_value->integerValue64;
			_value->floatValue = _value->integerValue64;
		}
		else if(payloadType == "float")
		{
			_value->setType(Flows::VariableType::tFloat);
			_value->floatValue = Flows::Math::getDouble(payload);
			_value->integerValue = _value->floatValue;
			_value->integerValue64 = _value->floatValue;
		}
		else if(payloadType == "string")
		{
			_value->setType(Flows::VariableType::tString);
			_value->stringValue = payload;
		}
		else if(payloadType == "array" || payloadType == "struct")
		{
			Flows::JsonDecoder jsonDecoder;
			_value = jsonDecoder.decode(payload);
		}

		return true;
	}
	catch(const std::exception& ex)
	{
		Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
	return false;
}

void MyNode::startUpComplete()
{
	try
	{
		Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
		message->structValue->emplace("payload", _value);

		output(0, message);
	}
	catch(const std::exception& ex)
	{
		Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
}

}
