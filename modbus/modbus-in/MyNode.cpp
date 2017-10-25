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

#include <homegear-base/HelperFunctions/HelperFunctions.h>
#include "MyNode.h"

namespace MyNode
{

MyNode::MyNode(std::string path, std::string nodeNamespace, std::string type, const std::atomic_bool* frontendConnected) : Flows::INode(path, nodeNamespace, type, frontendConnected)
{
	_localRpcMethods.emplace("packetReceived", std::bind(&MyNode::packetReceived, this, std::placeholders::_1));
	_localRpcMethods.emplace("setConnectionState", std::bind(&MyNode::setConnectionState, this, std::placeholders::_1));
}

MyNode::~MyNode()
{
}

bool MyNode::init(Flows::PNodeInfo info)
{
	try
	{
		auto settingsIterator = info->info->structValue->find("server");
		if(settingsIterator != info->info->structValue->end()) _server = settingsIterator->second->stringValue;

		settingsIterator = info->info->structValue->find("register");
		if(settingsIterator != info->info->structValue->end()) _register = Flows::Math::getNumber(settingsIterator->second->stringValue);

		settingsIterator = info->info->structValue->find("count");
		if(settingsIterator != info->info->structValue->end()) _count = Flows::Math::getNumber(settingsIterator->second->stringValue);

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
		if(_server.empty())
		{
			Flows::Output::printError("Error: This node has no Modbus server assigned.");
			return;
		}
		Flows::PArray parameters = std::make_shared<Flows::Array>();
		parameters->reserve(4);
		parameters->push_back(std::make_shared<Flows::Variable>(_id));
		parameters->push_back(std::make_shared<Flows::Variable>(_register));
		parameters->push_back(std::make_shared<Flows::Variable>(_count));
        parameters->push_back(std::make_shared<Flows::Variable>(true));
		Flows::PVariable result = invokeNodeMethod(_server, "registerNode", parameters, true);
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

//{{{ RPC methods
	Flows::PVariable MyNode::packetReceived(Flows::PArray parameters)
	{
		try
		{
            if(parameters->size() != 1) return Flows::Variable::createError(-1, "Method expects exactly one parameter. " + std::to_string(parameters->size()) + " given.");
			if(parameters->at(0)->type != Flows::VariableType::tBinary) return Flows::Variable::createError(-1, "Parameter 1 is not of type binary.");

            Flows::Output::printInfo("Moin a " + BaseLib::HelperFunctions::getHexString(parameters->at(0)->binaryValue));

			Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
			message->structValue->emplace("register", std::make_shared<Flows::Variable>(_register));
			message->structValue->emplace("count", std::make_shared<Flows::Variable>(_count));
			message->structValue->emplace("payload", parameters->at(0));

			output(0, message);

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

	Flows::PVariable MyNode::setConnectionState(Flows::PArray parameters)
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
