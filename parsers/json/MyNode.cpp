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

void MyNode::input(const Flows::PNodeInfo info, uint32_t index, const Flows::PVariable message)
{
	try
	{
		Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);

		if(message->structValue->at("payload")->type == Flows::VariableType::tStruct)
		{
			outputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>(_jsonEncoder.getString(message->structValue->at("payload"))));
			output(0, outputMessage);
		}
		else if(message->structValue->at("payload")->type == Flows::VariableType::tString)
		{
			std::string jsonString = message->structValue->at("payload")->stringValue;
			Flows::HelperFunctions::trim(jsonString);
			if(jsonString.empty()) return;
			Flows::PVariable json;
			if(jsonString.front() != '{' && jsonString.front() != '[')
			{
				if(jsonString.front() == '"' && jsonString.back() == '"') jsonString = jsonString.substr(1, jsonString.length() - 2);
				jsonString = '[' + jsonString + ']';
				json = _jsonDecoder.decode(jsonString);
				if(json->arrayValue->empty()) return;
				json = json->arrayValue->front();
			}
			else json = _jsonDecoder.decode(jsonString);
			outputMessage->structValue->emplace("payload", json);
			output(0, outputMessage);
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
