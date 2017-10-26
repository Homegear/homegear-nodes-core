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
		std::string variableType = "device";
		auto settingsIterator = info->info->structValue->find("variabletype");
		if(settingsIterator != info->info->structValue->end()) variableType = settingsIterator->second->stringValue;

		if(variableType == "device" || variableType == "metadata")
		{
			settingsIterator = info->info->structValue->find("peerid");
			if(settingsIterator != info->info->structValue->end()) _peerId = Flows::Math::getNumber64(settingsIterator->second->stringValue);
		}

		if(variableType == "device")
		{
			settingsIterator = info->info->structValue->find("channel");
			if(settingsIterator != info->info->structValue->end()) _channel = Flows::Math::getNumber(settingsIterator->second->stringValue);

			settingsIterator = info->info->structValue->find("variable");
			if(settingsIterator != info->info->structValue->end()) _variable = settingsIterator->second->stringValue;
		}
		else
		{
			settingsIterator = info->info->structValue->find("variabletext");
			if(settingsIterator != info->info->structValue->end()) _variable = settingsIterator->second->stringValue;
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

void MyNode::input(Flows::PNodeInfo info, uint32_t index, Flows::PVariable message)
{
	try
	{
		Flows::PArray parameters = std::make_shared<Flows::Array>();
		parameters->reserve(4);
		parameters->push_back(std::make_shared<Flows::Variable>(_peerId));
		parameters->push_back(std::make_shared<Flows::Variable>(_channel));
		parameters->push_back(std::make_shared<Flows::Variable>(_variable));

		Flows::PVariable oldValue = invoke("getValue", parameters);
		if(oldValue->errorStruct)
		{
			Flows::Output::printError("Error getting variable value (Peer ID: " + std::to_string(_peerId) + ", channel: " + std::to_string(_channel) + ", name: " + _variable + "): " + oldValue->structValue->at("faultString")->stringValue);
			return;
		}
        oldValue->booleanValue = !oldValue->booleanValue;

		parameters->push_back(oldValue);

        Flows::PVariable result = invoke("setValue", parameters);
		if(result->errorStruct) Flows::Output::printError("Error setting variable (Peer ID: " + std::to_string(_peerId) + ", channel: " + std::to_string(_channel) + ", name: " + _variable + "): " + result->structValue->at("faultString")->stringValue);

		Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
		message->structValue->emplace("peerId", std::make_shared<Flows::Variable>(_peerId));
		message->structValue->emplace("channel", std::make_shared<Flows::Variable>(_channel));
		message->structValue->emplace("variable", std::make_shared<Flows::Variable>(_variable));
		message->structValue->emplace("payload", oldValue);

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
