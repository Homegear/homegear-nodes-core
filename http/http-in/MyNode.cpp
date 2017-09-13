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
	_localRpcMethods.emplace("packetReceived", std::bind(&MyNode::packetReceived, this, std::placeholders::_1));
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

		settingsIterator = info->info->structValue->find("method");
		if(settingsIterator != info->info->structValue->end()) _method = settingsIterator->second->stringValue;

		settingsIterator = info->info->structValue->find("url");
		if(settingsIterator != info->info->structValue->end()) _path = settingsIterator->second->stringValue;

		settingsIterator = info->info->structValue->find("upload");
		if(settingsIterator != info->info->structValue->end()) _fileUploads = settingsIterator->second->booleanValue;

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
		Flows::PArray parameters = std::make_shared<Flows::Array>();
		parameters->reserve(3);
		parameters->push_back(std::make_shared<Flows::Variable>(_id));
		parameters->push_back(std::make_shared<Flows::Variable>(_method));
		parameters->push_back(std::make_shared<Flows::Variable>(_path));
		Flows::PVariable result = invokeNodeMethod(_server, "registerNode", parameters);
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

std::pair<std::string, std::string> MyNode::splitFirst(std::string string, char delimiter)
{
	int32_t pos = string.find_first_of(delimiter);
	if(pos == -1) return std::pair<std::string, std::string>(string, "");
	if((unsigned)pos + 1 >= string.size()) return std::pair<std::string, std::string>(string.substr(0, pos), "");
	return std::pair<std::string, std::string>(string.substr(0, pos), string.substr(pos + 1));
}

std::vector<std::string> MyNode::splitAll(std::string string, char delimiter)
{
	std::vector<std::string> elements;
	std::stringstream stringStream(string);
	std::string element;
	while (std::getline(stringStream, element, delimiter))
	{
		elements.push_back(element);
	}
	if(string.back() == delimiter) elements.push_back(std::string());
	return elements;
}

//{{{ RPC methods
	Flows::PVariable MyNode::packetReceived(Flows::PArray parameters)
	{
		try
		{
			if(parameters->size() != 7) return Flows::Variable::createError(-1, "Method expects exactly 7 parameters. " + std::to_string(parameters->size()) + " given.");
			if(parameters->at(0)->type != Flows::VariableType::tInteger) return Flows::Variable::createError(-1, "Parameter 1 is not of type integer.");
			if(parameters->at(1)->type != Flows::VariableType::tString) return Flows::Variable::createError(-1, "Parameter 2 is not of type string.");
			if(parameters->at(2)->type != Flows::VariableType::tString) return Flows::Variable::createError(-1, "Parameter 3 is not of type string.");
			if(parameters->at(3)->type != Flows::VariableType::tString) return Flows::Variable::createError(-1, "Parameter 4 is not of type string.");
			if(parameters->at(4)->type != Flows::VariableType::tString) return Flows::Variable::createError(-1, "Parameter 5 is not of type string.");
			if(parameters->at(5)->type != Flows::VariableType::tStruct) return Flows::Variable::createError(-1, "Parameter 6 is not of type struct.");
			if(parameters->at(6)->type != Flows::VariableType::tString) return Flows::Variable::createError(-1, "Parameter 7 is not of type string.");

			Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
			if(parameters->at(4)->stringValue == "application/x-www-form-urlencoded")
			{
				Flows::PVariable formData = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);

				std::vector<std::string> elements = splitAll(parameters->at(6)->stringValue, '&');
				for(auto& element : elements)
				{
					std::pair<std::string, std::string> variable = splitFirst(element, '=');
					std::string key;
					std::string value;
					BaseLib::Html::unescapeHtmlEntities(variable.first, key);
					BaseLib::Html::unescapeHtmlEntities(variable.second, value);
					formData->structValue->emplace(key, std::make_shared<Flows::Variable>(value));
				}

				message->structValue->emplace("payload", formData);
			}
			else if(parameters->at(3)->stringValue == "GET")
			{
				Flows::PVariable getData = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);

				std::vector<std::string> elements = splitAll(parameters->at(2)->stringValue, '&');
				for(auto& element : elements)
				{
					std::pair<std::string, std::string> variable = splitFirst(element, '=');
					std::string key;
					std::string value;
					BaseLib::Html::unescapeHtmlEntities(variable.first, key);
					BaseLib::Html::unescapeHtmlEntities(variable.second, value);
					getData->structValue->emplace(key, std::make_shared<Flows::Variable>(value));
				}

				message->structValue->emplace("payload", getData);
			}
			else if(parameters->at(4)->stringValue == "application/json")
			{
				message->structValue->emplace("payload", _jsonDecoder.decode(parameters->at(6)->stringValue));
			}
			else
			{
				message->structValue->emplace("payload", std::make_shared<Flows::Variable>(parameters->at(6)->stringValue));
			}

			Flows::PVariable req = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);

			req->structValue->emplace("query", std::make_shared<Flows::Variable>(parameters->at(1)->stringValue));
			req->structValue->emplace("method", std::make_shared<Flows::Variable>(parameters->at(3)->stringValue));
			req->structValue->emplace("contentType", std::make_shared<Flows::Variable>(parameters->at(4)->stringValue));
			req->structValue->emplace("headers", parameters->at(5));

			Flows::PVariable cookies = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
			auto cookieIterator = parameters->at(5)->structValue->find("cookie");
			if(cookieIterator != parameters->at(5)->structValue->end())
			{
				std::vector<std::string> elements = splitAll(cookieIterator->second->stringValue, ';');
				for(auto& element : elements)
				{
					Flows::HelperFunctions::trim(element);
					std::pair<std::string, std::string> cookie = splitFirst(element, '=');
					Flows::HelperFunctions::trim(cookie.first);
					Flows::HelperFunctions::trim(cookie.second);
					cookies->structValue->emplace(cookie.first, std::make_shared<Flows::Variable>(cookie.second));
				}
			}
			req->structValue->emplace("cookies", cookies);

			message->structValue->emplace("req", req);

			Flows::PVariable internal = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
			internal->structValue->emplace("clientId", std::make_shared<Flows::Variable>(parameters->at(0)->integerValue));

			setInternalMessage(internal);

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
//}}}

}
