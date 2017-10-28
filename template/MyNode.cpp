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
		_nodeInfo = info;

		auto settingsIterator = info->info->structValue->find("template");
		if(settingsIterator != info->info->structValue->end()) _plainTemplate = settingsIterator->second->stringValue;
		_template.reset(new kainjow::mustache::mustache(_plainTemplate));

		settingsIterator = info->info->structValue->find("syntax");
		if(settingsIterator != info->info->structValue->end()) _mustache = settingsIterator->second->stringValue == "mustache";

		settingsIterator = info->info->structValue->find("output");
		if(settingsIterator != info->info->structValue->end()) _parseJson = settingsIterator->second->stringValue == "json";

		settingsIterator = info->info->structValue->find("field");
		if(settingsIterator != info->info->structValue->end()) _field = settingsIterator->second->stringValue;

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

void MyNode::addData(bool global, std::string key)
{
	try
	{
		Flows::PArray parameters = std::make_shared<Flows::Array>();
		parameters->reserve(2);
		parameters->push_back(std::make_shared<Flows::Variable>(global ? "global" : _nodeInfo->info->structValue->at("z")->stringValue));
		parameters->push_back(std::make_shared<Flows::Variable>(key));
		Flows::PVariable result = invoke("getNodeData", parameters);
		if(result->errorStruct) return;
		auto data = (mustache::data*)(_data.get(global ? "global" : "flow")); //This is dirty, but works as there is no access to _data, when addData is called.
		if(!data) return;
		setData(*data, key, result);
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

void MyNode::setData(mustache::data& data, std::string key, Flows::PVariable value)
{
	try
	{
		if(value->type == Flows::VariableType::tBoolean)
		{
			mustache::data element = value->booleanValue ? mustache::data::type::bool_true : mustache::data::type::bool_false;
			if(key.empty()) data.push_back(element); else data.set(key, element);
		}
		else if(value->type == Flows::VariableType::tArray)
		{
			mustache::data element = mustache::data::type::list;
			for(auto arrayElement : *value->arrayValue)
			{
				setData(element, "", arrayElement);
			}
			if(key.empty()) data.push_back(element); else data.set(key, element);
		}
		else if(value->type == Flows::VariableType::tStruct)
		{
			mustache::data element = mustache::data::type::object;
			for(auto pair : *value->structValue)
			{
				setData(element, pair.first, pair.second);
			}
			if(key.empty()) data.push_back(element); else data.set(key, element);
		}
		else
		{
			if(key.empty()) data.push_back(value->toString()); else data.set(key, value->toString());
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

void MyNode::input(const Flows::PNodeInfo info, uint32_t index, const Flows::PVariable message)
{
	try
	{
		std::lock_guard<std::mutex> inputGuard(_inputMutex);
		_data = mustache::data();
		for(auto& element : *message->structValue)
		{
			setData(_data, element.first, element.second);
		}
		_data.set("flow", mustache::data(mustache::data::type::object));
		_data.set("global", mustache::data(mustache::data::type::object));

		Flows::PVariable result;
		if(_mustache) result = std::make_shared<Flows::Variable>(_template->render(_data, std::function<void(bool, std::string)>(std::bind(&MyNode::addData, this, std::placeholders::_1, std::placeholders::_2))));
		else result = std::make_shared<Flows::Variable>(_plainTemplate);

		Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
		if(_parseJson)
		{
			Flows::PVariable json;
			result->stringValue = Flows::HelperFunctions::trim(result->stringValue);
			if(result->stringValue.empty()) json = std::make_shared<Flows::Variable>();
			else if(result->stringValue.front() != '[' && result->stringValue.front() != '{')
			{
				result->stringValue = '[' + result->stringValue + ']';
				json = _jsonDecoder.decode(result->stringValue)->arrayValue->at(0);
			}
			else json = _jsonDecoder.decode(result->stringValue);
			if(_field.empty() && json->type == Flows::VariableType::tStruct)
			{
				outputMessage = json;
			}
			else
			{
				if(_field.empty()) _field = "payload";
				outputMessage->structValue->emplace(_field, json);
			}
			output(0, outputMessage);
		}
		else
		{
			if(_field.empty()) _field = "payload";
			outputMessage->structValue->emplace(_field, result);
			output(0, outputMessage);
		}
	}
	catch(const Flows::JsonDecoderException& ex)
	{
		_out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
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
