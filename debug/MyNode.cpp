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
}

MyNode::~MyNode()
{
}

void MyNode::setNodeVariable(std::string& variable, Flows::PVariable& value)
{
	try
	{
		if(variable == "active" && value && value->type == Flows::VariableType::tBoolean) _active = value->booleanValue;
	}
	catch(const std::exception& ex)
	{
		log(2, std::string("Error in file ") + __FILE__ + " in line " + std::to_string(__LINE__) + " and function " + __PRETTY_FUNCTION__ + ": " + ex.what());
	}
	catch(...)
	{
		log(2, std::string("Unknown error in file ") + __FILE__ + " in line " + std::to_string(__LINE__) + " and function " + __PRETTY_FUNCTION__ + ".");
	}
}

void MyNode::input(Flows::PNodeInfo info, uint32_t index, Flows::PVariable message)
{
	try
	{
		if(!*_frontendConnected || !_active) return;
		std::string property;
		auto completeIterator = info->info->structValue->find("complete");
		if(completeIterator == info->info->structValue->end() || completeIterator->second->stringValue == "false" || completeIterator->second->stringValue.empty())
		{
			auto payloadIterator = message->structValue->find("payload");
			if(payloadIterator == message->structValue->end()) return;
			property = "payload";
			message = payloadIterator->second;
		}
		else if(completeIterator->second->stringValue != "true")
		{
			auto payloadIterator = message->structValue->find(completeIterator->second->stringValue);
			if(payloadIterator == message->structValue->end()) return;
			property = completeIterator->second->stringValue;
			message = payloadIterator->second;
		}

		Flows::PVariable object = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
		object->structValue->emplace("id", std::make_shared<Flows::Variable>(_id));
		object->structValue->emplace("name", std::make_shared<Flows::Variable>(_name));
		object->structValue->emplace("msg", message);

		std::string format;
		switch(message->type)
		{
		case Flows::VariableType::tArray:
			format = "array[" + std::to_string(message->arrayValue->size()) + "]";
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
			format = "string[" + std::to_string(message->stringValue.size()) + "]";
			break;
		case Flows::VariableType::tStruct:
			format = "Object";
			break;
		case Flows::VariableType::tBase64:
			format = "string[" + std::to_string(message->stringValue.size()) + "]";
			break;
		case Flows::VariableType::tVariant:
			break;
		case Flows::VariableType::tBinary:
			break;
		case Flows::VariableType::tVoid:
			break;
		}

		if(!format.empty()) object->structValue->emplace("format", std::make_shared<Flows::Variable>(format));
		if(!property.empty()) object->structValue->emplace("property", std::make_shared<Flows::Variable>(property));
		nodeEvent("debug", object);
	}
	catch(const std::exception& ex)
	{
		log(2, std::string("Error in file ") + __FILE__ + " in line " + std::to_string(__LINE__) + " and function " + __PRETTY_FUNCTION__ + ": " + ex.what());
	}
	catch(...)
	{
		log(2, std::string("Unknown error in file ") + __FILE__ + " in line " + std::to_string(__LINE__) + " and function " + __PRETTY_FUNCTION__ + ".");
	}
}

}
