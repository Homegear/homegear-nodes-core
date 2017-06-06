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

MyNode::RuleType MyNode::getRuleTypeFromString(std::string& t)
{
	MyNode::RuleType ruleType = MyNode::RuleType::tEq;
	if(t == "eq") ruleType = MyNode::RuleType::tEq;
	else if(t == "neq") ruleType = MyNode::RuleType::tNeq;
	else if(t == "lt") ruleType = MyNode::RuleType::tLt;
	else if(t == "lte") ruleType = MyNode::RuleType::tLte;
	else if(t == "gt") ruleType = MyNode::RuleType::tGt;
	else if(t == "gte") ruleType = MyNode::RuleType::tGte;
	else if(t == "btwn") ruleType = MyNode::RuleType::tBtwn;
	else if(t == "cont") ruleType = MyNode::RuleType::tCont;
	else if(t == "regex") ruleType = MyNode::RuleType::tRegex;
	else if(t == "true") ruleType = MyNode::RuleType::tTrue;
	else if(t == "false") ruleType = MyNode::RuleType::tFalse;
	else if(t == "null") ruleType = MyNode::RuleType::tNull;
	else if(t == "nnull") ruleType = MyNode::RuleType::tNnull;
	else if(t == "else") ruleType = MyNode::RuleType::tElse;
	return ruleType;
}

Flows::VariableType MyNode::getValueTypeFromString(std::string& vt)
{
	Flows::VariableType variableType = Flows::VariableType::tVoid;
	if(vt == "bool") variableType = Flows::VariableType::tBoolean;
	else if(vt == "int") variableType = Flows::VariableType::tInteger64;
	else if(vt == "float") variableType = Flows::VariableType::tFloat;
	else if(vt == "string") variableType = Flows::VariableType::tString;
	else if(vt == "array") variableType = Flows::VariableType::tArray;
	else if(vt == "struct") variableType = Flows::VariableType::tStruct;
	return variableType;
}

void MyNode::convertType(Flows::PVariable& value, Flows::VariableType vt)
{
	if(vt == Flows::VariableType::tInteger)
	{
		value->setType(vt);
		value->integerValue = Flows::Math::getNumber(value->stringValue);
	}
	else if(vt == Flows::VariableType::tInteger64)
	{
		value->setType(vt);
		value->integerValue64 = Flows::Math::getNumber64(value->stringValue);
	}
	else if(vt == Flows::VariableType::tFloat)
	{
		value->setType(vt);
		value->floatValue = Flows::Math::getDouble(value->stringValue);
	}
	else if(vt == Flows::VariableType::tArray || vt == Flows::VariableType::tStruct)
	{
		Flows::JsonDecoder jsonDecoder;
		value = jsonDecoder.decode(value->stringValue);
	}
}

bool MyNode::init(Flows::PNodeInfo info)
{
	try
	{
		auto settingsIterator = info->info->structValue->find("checkall");
		if(settingsIterator != info->info->structValue->end()) _checkAll = settingsIterator->second->stringValue == "true" || settingsIterator->second->booleanValue;

		settingsIterator = info->info->structValue->find("property");
		if(settingsIterator != info->info->structValue->end()) _property = settingsIterator->second->stringValue;

		Flows::PArray rules;
		settingsIterator = info->info->structValue->find("rules");
		if(settingsIterator != info->info->structValue->end()) rules = settingsIterator->second->arrayValue;

		_rules.clear();
		if(rules)
		{
			_rules.reserve(rules->size());
			for(auto& ruleStruct : *rules)
			{
				auto typeIterator = ruleStruct->structValue->find("t");
				auto valueIterator = ruleStruct->structValue->find("v");
				auto valueTypeIterator = ruleStruct->structValue->find("vt");

				if(typeIterator == ruleStruct->structValue->end()) continue;

				Rule rule;
				rule.t = getRuleTypeFromString(typeIterator->second->stringValue);

				if(valueIterator != ruleStruct->structValue->end() && valueTypeIterator != ruleStruct->structValue->end())
				{
					rule.v = valueIterator->second;
					rule.vt = getValueTypeFromString(valueTypeIterator->second->stringValue);
					rule.previousValue = valueTypeIterator->second->stringValue == "prev";
					convertType(rule.v, rule.vt);
				}
				else rule.v = std::make_shared<Flows::Variable>();


				valueIterator = ruleStruct->structValue->find("v2");
				valueTypeIterator = ruleStruct->structValue->find("v2t");

				if(valueIterator != ruleStruct->structValue->end() && valueTypeIterator != ruleStruct->structValue->end())
				{
					rule.v2 = valueIterator->second;
					rule.v2t = getValueTypeFromString(valueTypeIterator->second->stringValue);
					convertType(rule.v2, rule.v2t);
				}
				else rule.v2 = std::make_shared<Flows::Variable>();

				if(rule.t == RuleType::tRegex)
				{
					bool ignoreCase = false;
					auto caseIterator = ruleStruct->structValue->find("case");
					if(caseIterator != ruleStruct->structValue->end()) ignoreCase = caseIterator->second->booleanValue;

					rule.regex = std::regex(rule.v->stringValue, ignoreCase ? std::regex::icase : std::regex::ECMAScript);
				}

				_rules.push_back(rule);
			}
		}

		_previousValue = getNodeData("previousValue");

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

bool MyNode::isTrue(Flows::PVariable& value)
{
	if(value->type != Flows::VariableType::tBoolean)
	{
		switch(value->type)
		{
		case Flows::VariableType::tArray:
			return !value->arrayValue->empty();
		case Flows::VariableType::tBase64:
			return !value->stringValue.empty();
		case Flows::VariableType::tBinary:
			return !value->binaryValue.empty();
		case Flows::VariableType::tBoolean:
			break;
		case Flows::VariableType::tFloat:
			return value->floatValue;
		case Flows::VariableType::tInteger:
			return value->integerValue;
		case Flows::VariableType::tInteger64:
			return value->integerValue64;
		case Flows::VariableType::tString:
			return !value->stringValue.empty();
		case Flows::VariableType::tStruct:
			return !value->structValue->empty();
		case Flows::VariableType::tVariant:
			break;
		case Flows::VariableType::tVoid:
			return false;
		}
	}
	return value->booleanValue;
}

bool MyNode::match(Rule& rule, Flows::PVariable& value)
{
	try
	{
		if(rule.previousValue)
		{
			rule.v = _previousValue;
			rule.vt = _previousValue->type;
		}
		switch(rule.t)
		{
		case RuleType::tEq:
			if(value->type == rule.vt && *value == *rule.v) return true;
			return false;
		case RuleType::tNeq:
			if(value->type == rule.vt && *value == *rule.v) return false;
			return true;
		case RuleType::tLt:
			if(value->type == rule.vt && *value < *rule.v) return true;
			return false;
		case RuleType::tLte:
			if(value->type == rule.vt && *value <= *rule.v) return true;
			return false;
		case RuleType::tGt:
			if(value->type == rule.vt && *value > *rule.v) return true;
			return false;
		case RuleType::tGte:
			if(value->type == rule.vt && *value >= *rule.v) return true;
			return false;
		case RuleType::tBtwn:
			if(rule.vt == rule.v2t && rule.vt == value->type)
			{
				if(rule.vt == Flows::VariableType::tInteger) return value->integerValue >= rule.v->integerValue && value->integerValue <= rule.v2->integerValue;
				else if(rule.vt == Flows::VariableType::tInteger64) return value->integerValue64 >= rule.v->integerValue64 && value->integerValue64 <= rule.v2->integerValue64;
				else if(rule.vt == Flows::VariableType::tFloat) return value->floatValue >= rule.v->floatValue && value->floatValue <= rule.v2->floatValue;
			}
			else
			{
				if(rule.vt == Flows::VariableType::tInteger) rule.v->floatValue = rule.v->integerValue;
				if(rule.vt == Flows::VariableType::tInteger64) rule.v->floatValue = rule.v->integerValue64;
				if(rule.v2t == Flows::VariableType::tInteger) rule.v2->floatValue = rule.v2->integerValue;
				if(rule.v2t == Flows::VariableType::tInteger64) rule.v2->floatValue = rule.v2->integerValue64;
				if(value->type == Flows::VariableType::tInteger) value->floatValue = value->integerValue;
				if(value->type == Flows::VariableType::tInteger64) value->floatValue = value->integerValue64;
				return value->floatValue >= rule.v->floatValue && value->floatValue <= rule.v2->floatValue;
			}
			return false;
		case RuleType::tCont:
			return value->stringValue.find(rule.v->toString()) != std::string::npos;
		case RuleType::tRegex:
			if(value->type != Flows::VariableType::tString) value->stringValue = value->toString();
			return std::regex_match(value->stringValue, rule.regex);
		case RuleType::tTrue:
			return isTrue(value);
		case RuleType::tFalse:
			return !isTrue(value);
		case RuleType::tNull:
			return value->type == Flows::VariableType::tVoid;
		case RuleType::tNnull:
			return value->type != Flows::VariableType::tVoid;
		case RuleType::tElse:
			return true;
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
	return false;
}

void MyNode::input(Flows::PNodeInfo info, uint32_t index, Flows::PVariable message)
{
	try
	{
		auto messageIterator = message->structValue->find(_property);
		if(messageIterator == message->structValue->end()) return;

		for(uint32_t i = 0; i < _rules.size(); i++)
		{
			if(match(_rules.at(i), messageIterator->second))
			{
				output(i, message);
				if(!_checkAll) break;
			}
		}

		_previousValue = messageIterator->second;
		setNodeData("peviousValue", _previousValue);
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
