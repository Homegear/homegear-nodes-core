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

MyNode::MyNode(std::string path, std::string name, const std::atomic_bool* frontendConnected) : Flows::INode(path, name, frontendConnected)
{
	_localRpcMethods.emplace("setConnectionState", std::bind(&MyNode::setConnectionState, this, std::placeholders::_1));
}

MyNode::~MyNode()
{
}

bool MyNode::init(Flows::PNodeInfo info)
{
	try
	{
		auto settingsIterator = info->info->structValue->find("broker");
		if(settingsIterator != info->info->structValue->end()) _broker = settingsIterator->second->stringValue;

		settingsIterator = info->info->structValue->find("topic");
		if(settingsIterator != info->info->structValue->end()) _topic = settingsIterator->second->stringValue;

		settingsIterator = info->info->structValue->find("retain");
		if(settingsIterator != info->info->structValue->end()) _retain = settingsIterator->second->booleanValue;

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

void MyNode::configNodesStarted()
{
	try
	{
		if(_broker.empty())
		{
			Flows::Output::printError("Error: This node has no broker assigned.");
			return;
		}
		Flows::PArray parameters = std::make_shared<Flows::Array>();
		parameters->push_back(std::make_shared<Flows::Variable>(_id));
		Flows::PVariable result = invokeNodeMethod(_broker, "registerNode", parameters);
		if(result->errorStruct) Flows::Output::printError("Error: Could not register node: " + result->structValue->at("faultString")->stringValue);
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

void MyNode::input(Flows::PNodeInfo info, uint32_t index, Flows::PVariable message)
{
	try
	{
		std::string topic;
		auto messageIterator = message->structValue->find("topic");
		if(messageIterator != message->structValue->end()) topic = messageIterator->second->stringValue;
		else topic = _topic;

		bool retain;
		messageIterator = message->structValue->find("retain");
		if(messageIterator != message->structValue->end()) retain = messageIterator->second->booleanValue;
		else retain = _retain;

		Flows::PVariable payload = message->structValue->at("payload");
		if(payload->type == Flows::VariableType::tArray || payload->type == Flows::VariableType::tStruct) payload->stringValue = _jsonEncoder.getString(payload);
		else if(payload->type != Flows::VariableType::tString) payload->stringValue = payload->toString();
		payload->setType(Flows::VariableType::tString);

		Flows::PArray parameters = std::make_shared<Flows::Array>();
		parameters->reserve(3);
		parameters->push_back(std::make_shared<Flows::Variable>(topic));
		parameters->push_back(payload);
		parameters->push_back(std::make_shared<Flows::Variable>(retain));

		Flows::PVariable result = invokeNodeMethod(_broker, "publish", parameters);
		if(result->errorStruct) Flows::Output::printError("Error publishing topic: " + topic);
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

//{{{ RPC methods
	Flows::PVariable MyNode::setConnectionState(Flows::PArray& parameters)
	{
		try
		{
			if(parameters->size() != 1) return Flows::Variable::createError(-1, "Method expects exactly one parameter. " + std::to_string(parameters->size()) + " given.");
			if(parameters->at(0)->type != Flows::VariableType::tBoolean) return Flows::Variable::createError(-1, "Parameter is not of type boolean.");

			Flows::PVariable status = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
			if(parameters->at(0)->booleanValue)
			{
				status->structValue->emplace("text", std::make_shared<Flows::Variable>("connected"));
				status->structValue->emplace("fill", std::make_shared<Flows::Variable>("green"));
				status->structValue->emplace("shape", std::make_shared<Flows::Variable>("dot"));
			}
			else
			{
				status->structValue->emplace("text", std::make_shared<Flows::Variable>("disconnected"));
				status->structValue->emplace("fill", std::make_shared<Flows::Variable>("red"));
				status->structValue->emplace("shape", std::make_shared<Flows::Variable>("dot"));
			}
			nodeEvent("statusBottom/" + _id, status);

			return std::make_shared<Flows::Variable>();
		}
		catch(const std::exception& ex)
		{
			Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
		}
		catch(...)
		{
			Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
		}
		return Flows::Variable::createError(-32500, "Unknown application error.");
	}
//}}}

}
