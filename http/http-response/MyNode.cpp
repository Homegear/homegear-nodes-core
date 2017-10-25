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
		auto settingsIterator = info->info->structValue->find("server");
		if(settingsIterator != info->info->structValue->end()) _server = settingsIterator->second->stringValue;

		settingsIterator = info->info->structValue->find("statusCode");
		if(settingsIterator != info->info->structValue->end()) _statusCode = Flows::Math::getNumber(settingsIterator->second->stringValue);

		_headers = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);

		settingsIterator = info->info->structValue->find("headers");
		if(settingsIterator != info->info->structValue->end())
		{
			for(auto& header : *(settingsIterator->second->structValue))
			{
				std::string key = header.first;
				Flows::HelperFunctions::toLower(key);
				_headers->structValue->emplace(key, header.second);
			}
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

void MyNode::configNodesStarted()
{
	try
	{
		if(_server.empty())
		{
			Flows::Output::printError("Error: This node has no server assigned.");
			return;
		}
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
		if(_server.empty())
		{
			Flows::Output::printError("Error: This node has no server assigned.");
			return;
		}

		auto messageIterator = message->structValue->find("_internal");
		if(messageIterator == message->structValue->end())
		{
			Flows::Output::printError("Error: No internal data found in message object.");
			return;
		}

		auto internalIterator = messageIterator->second->structValue->find("clientId");
		if(internalIterator == messageIterator->second->structValue->end())
		{
			Flows::Output::printError("Error: No clientId found in internal data.");
			return;
		}

		int32_t statusCode = _statusCode;
		if(statusCode == 0)
		{
			messageIterator = message->structValue->find("statusCode");
			if(messageIterator != message->structValue->end())
			{
				statusCode = messageIterator->second->integerValue;
			}
		}
		if(statusCode == 0) statusCode = 200;

		Flows::PVariable headers = std::make_shared<Flows::Variable>(Flows::VariableType::tArray);
		bool contentType = false;
		if(_headers->structValue->empty())
		{
			messageIterator = message->structValue->find("headers");
			if(messageIterator != message->structValue->end())
			{
				for(auto& header : *(messageIterator->second->structValue))
				{
					if(header.second->stringValue.empty()) continue;
					std::string key = header.first;
					Flows::HelperFunctions::toLower(key);
					if(key == "content-type") contentType = true;
					headers->arrayValue->push_back(std::make_shared<Flows::Variable>(key + ": " + header.second->stringValue));
				}
			}
		}
		else
		{
			for(auto& header : *(_headers->structValue))
			{
				if(header.second->stringValue.empty()) continue;
				std::string key = header.first;
				if(key == "content-type") contentType = true;
				headers->arrayValue->push_back(std::make_shared<Flows::Variable>(key + ": " + header.second->stringValue));
			}
		}
		if(!contentType) headers->arrayValue->push_back(std::make_shared<Flows::Variable>("Content-Type: text/plain"));

		messageIterator = message->structValue->find("cookies");
		if(messageIterator != message->structValue->end())
		{
			for(auto& cookie : *messageIterator->second->structValue)
			{
				if(cookie.second->type == Flows::VariableType::tString)
				{
					headers->arrayValue->push_back(std::make_shared<Flows::Variable>("Set-Cookie: " + cookie.first + "=" + cookie.second->stringValue));
				}
				else
				{
					auto cookieIterator = cookie.second->structValue->find("value");
					if(cookieIterator == cookie.second->structValue->end()) continue;
					std::string cookieString = "Set-Cookie: " + cookie.first + "=" + cookieIterator->second->stringValue;
					if(!cookieIterator->second->stringValue.empty())
					{
						for(auto& element : *cookie.second->structValue)
						{
							if(element.first == "value") continue;
							if(element.second->stringValue.empty()) cookieString.append("; " + element.first);
							else cookieString.append("; " + element.first + "=" + element.second->stringValue);
						}
					}
					headers->arrayValue->push_back(std::make_shared<Flows::Variable>(cookieString));
				}
			}
		}

		Flows::PArray parameters = std::make_shared<Flows::Array>();
		parameters->reserve(4);
		parameters->push_back(internalIterator->second);
		parameters->push_back(std::make_shared<Flows::Variable>(statusCode));
		parameters->push_back(headers);
		parameters->push_back(message->structValue->at("payload"));

		Flows::PVariable result = invokeNodeMethod(_server, "send", parameters, true);
		if(result->errorStruct) Flows::Output::printError("Error sending data: " + result->structValue->at("faultString")->stringValue);
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
