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

#include <homegear-base/HelperFunctions/HelperFunctions.h>
#include "MyNode.h"
#include "../switch/MyNode.h"

namespace MyNode
{

MyNode::MyNode(std::string path, std::string nodeNamespace, std::string type, const std::atomic_bool* frontendConnected) : Flows::INode(path, nodeNamespace, type, frontendConnected)
{
}

MyNode::~MyNode()
{
}

std::string& MyNode::stringReplace(std::string& haystack, const std::string& search, const std::string& replace)
{
    if(search.empty()) return haystack;
    int32_t pos = 0;
    while(true)
    {
        pos = haystack.find(search, pos);
        if (pos == (signed)std::string::npos) break;
        haystack.replace(pos, search.size(), replace);
        pos += replace.size();
    }
    return haystack;
}

MyNode::RuleType MyNode::getRuleTypeFromString(std::string& t)
{
	MyNode::RuleType ruleType = MyNode::RuleType::tSet;
	if(t == "set") ruleType = MyNode::RuleType::tSet;
	else if(t == "change") ruleType = MyNode::RuleType::tChange;
	else if(t == "move") ruleType = MyNode::RuleType::tMove;
	else if(t == "delete") ruleType = MyNode::RuleType::tDelete;
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
    if(vt == Flows::VariableType::tBoolean)
    {
        if(value->type == Flows::VariableType::tString)
        {
            value->setType(Flows::VariableType::tBoolean);
            value->booleanValue = (value->stringValue == "true");
        }
    }
    else if(vt == Flows::VariableType::tInteger)
	{
		value->setType(Flows::VariableType::tInteger64);
		value->integerValue = Flows::Math::getNumber(value->stringValue);
		value->integerValue64 = value->integerValue;
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
		auto flowIdIterator = info->info->structValue->find("z");
		if(flowIdIterator != info->info->structValue->end()) _flowId = flowIdIterator->second->stringValue;
		else _flowId = "g";

		Flows::PArray rules;
		auto settingsIterator = info->info->structValue->find("rules");
		if(settingsIterator != info->info->structValue->end()) rules = settingsIterator->second->arrayValue;

		_rules.clear();
		if(rules)
		{
			_rules.reserve(rules->size());
			uint32_t index = 0;
			for(auto& ruleStruct : *rules)
			{
                Rule rule;

				auto typeIterator = ruleStruct->structValue->find("t");
				auto valueIterator = ruleStruct->structValue->find("p");
				auto valueTypeIterator = ruleStruct->structValue->find("pt");
				if(typeIterator == ruleStruct->structValue->end()) continue;
				rule.t = getRuleTypeFromString(typeIterator->second->stringValue);
				if(valueIterator != ruleStruct->structValue->end() && valueTypeIterator != ruleStruct->structValue->end())
				{
					if(valueTypeIterator->second->stringValue == "message") rule.messageProperty = valueIterator->second->stringValue;
					else if(valueTypeIterator->second->stringValue == "flow") rule.flowVariable = valueIterator->second->stringValue;
					else if(valueTypeIterator->second->stringValue == "global") rule.globalVariable = valueIterator->second->stringValue;
				}


                valueIterator = ruleStruct->structValue->find("from");
                valueTypeIterator = ruleStruct->structValue->find("fromt");
                if(valueIterator != ruleStruct->structValue->end() && valueTypeIterator != ruleStruct->structValue->end())
                {
                    rule.from = valueIterator->second;
                    rule.fromt = getValueTypeFromString(valueTypeIterator->second->stringValue);
                    if(valueTypeIterator->second->stringValue == "message") rule.messagePropertyFrom = rule.from->stringValue;
                    else if(valueTypeIterator->second->stringValue == "flow") rule.flowVariableFrom = rule.from->stringValue;
                    else if(valueTypeIterator->second->stringValue == "global") rule.globalVariableFrom = rule.from->stringValue;
                    convertType(rule.from, rule.fromt);
                }
                else rule.from = std::make_shared<Flows::Variable>();
                if(valueTypeIterator->second->stringValue == "regex")
                {
                    rule.fromRegexSet = true;
                    rule.fromRegex = std::regex(rule.from->stringValue, std::regex::ECMAScript);
                }


				valueIterator = ruleStruct->structValue->find("to");
				valueTypeIterator = ruleStruct->structValue->find("tot");
				if(valueIterator != ruleStruct->structValue->end() && valueTypeIterator != ruleStruct->structValue->end())
				{
					rule.to = valueIterator->second;
					rule.tot = getValueTypeFromString(valueTypeIterator->second->stringValue);
                    if(valueTypeIterator->second->stringValue == "message") rule.messagePropertyTo = rule.to->stringValue;
                    else if(valueTypeIterator->second->stringValue == "flow") rule.flowVariableTo = rule.to->stringValue;
                    else if(valueTypeIterator->second->stringValue == "global") rule.globalVariableTo = rule.to->stringValue;
					convertType(rule.to, rule.tot);
				}
				else rule.to = std::make_shared<Flows::Variable>();

				_rules.push_back(rule);
				index++;
			}
		}

		return true;
	}
	catch(const std::exception& ex)
	{
		_out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	return false;
}

void MyNode::applyRule(Rule& rule, Flows::PVariable& value)
{
    try
    {
        if(!rule.messageProperty.empty())
        {
            if(rule.t == RuleType::tDelete)
            {
                if(!rule.flowVariable.empty()) setFlowData(rule.flowVariable, std::make_shared<Flows::Variable>());
                else if(!rule.globalVariable.empty()) setFlowData(rule.globalVariable, std::make_shared<Flows::Variable>());
                else if(!rule.messageProperty.empty()) value->structValue->erase(rule.messageProperty);
            }
            else if(rule.t == RuleType::tSet)
            {
                if(!rule.flowVariableTo.empty()) rule.to = getFlowData(rule.flowVariableTo);
                else if(!rule.globalVariableTo.empty()) rule.to = getGlobalData(rule.globalVariableTo);
                else if(!rule.messagePropertyTo.empty())
                {
                    auto propertyIterator = value->structValue->find(rule.messagePropertyTo);
                    if(propertyIterator != value->structValue->end()) rule.to = propertyIterator->second;
                    else rule.to = std::make_shared<Flows::Variable>();
                }

                if(!rule.flowVariable.empty()) setFlowData(rule.flowVariable, rule.to);
                else if(!rule.globalVariable.empty()) setFlowData(rule.globalVariable, rule.to);
                else if(!rule.messageProperty.empty()) (*value->structValue)[rule.messageProperty] = rule.to;
            }
            else if(rule.t == RuleType::tMove)
            {
                Flows::PVariable currentValue;

                //{{{ Set
                    if(!rule.flowVariable.empty()) currentValue = getFlowData(rule.flowVariable);
                    else if(!rule.globalVariable.empty()) currentValue = getGlobalData(rule.globalVariable);
                    else if(!rule.messageProperty.empty())
                    {
                        auto propertyIterator = value->structValue->find(rule.messageProperty);
                        if(propertyIterator != value->structValue->end()) currentValue = propertyIterator->second;
                        else currentValue = std::make_shared<Flows::Variable>();
                    }
                    else currentValue = std::make_shared<Flows::Variable>();

                    if(!rule.flowVariableTo.empty()) setFlowData(rule.flowVariableTo, currentValue);
                    else if(!rule.globalVariableTo.empty()) setFlowData(rule.globalVariableTo, currentValue);
                    else if(!rule.messagePropertyTo.empty()) (*value->structValue)[rule.messagePropertyTo] = currentValue;
                //}}}

                //{{{ Delete
                    if(!rule.flowVariable.empty()) setFlowData(rule.flowVariable, std::make_shared<Flows::Variable>());
                    else if(!rule.globalVariable.empty()) setFlowData(rule.globalVariable, std::make_shared<Flows::Variable>());
                    else if(!rule.messageProperty.empty()) value->structValue->erase(rule.messageProperty);
                //}}}
            }
            else if(rule.t == RuleType::tChange)
            {
                Flows::PVariable currentValue;

                if(!rule.flowVariable.empty()) currentValue = getFlowData(rule.flowVariable);
                else if(!rule.globalVariable.empty()) currentValue = getGlobalData(rule.globalVariable);
                else if(!rule.messageProperty.empty())
                {
                    auto propertyIterator = value->structValue->find(rule.messageProperty);
                    if(propertyIterator != value->structValue->end()) currentValue = propertyIterator->second;
                    else currentValue = std::make_shared<Flows::Variable>();
                }
                else currentValue = std::make_shared<Flows::Variable>();

                if(rule.fromRegexSet)
                {
                    if(currentValue->type != Flows::VariableType::tString)
                    {
                        currentValue->stringValue = currentValue->toString();
                        currentValue->type = Flows::VariableType::tString;
                    }

                    if(rule.tot != Flows::VariableType::tString)
                    {
                        if(!std::regex_match(currentValue->stringValue, rule.fromRegex)) return;
                        currentValue = rule.to;
                    }
                    else currentValue->stringValue = std::regex_replace(currentValue->stringValue, rule.fromRegex, rule.to->toString());
                }
                else
                {
                    if(rule.fromt == Flows::VariableType::tString)
                    {
                        if(currentValue->type != Flows::VariableType::tString)
                        {
                            currentValue->stringValue = currentValue->toString();
                            currentValue->type = Flows::VariableType::tString;
                        }

                        if(currentValue->stringValue.find(rule.from->stringValue) == std::string::npos) return;

                        if(rule.tot != Flows::VariableType::tString) currentValue = rule.to;
                        else stringReplace(currentValue->stringValue, rule.from->stringValue, rule.to->toString());
                    }
                    else
                    {
                        if(currentValue->type != rule.from->type) return;

                        if(currentValue->type == Flows::VariableType::tBoolean && currentValue->booleanValue == rule.from->booleanValue) currentValue = rule.to;
                        else if(currentValue->type == Flows::VariableType::tInteger64 && currentValue->integerValue64 == rule.from->integerValue64) currentValue = rule.to;
                        else if(currentValue->type == Flows::VariableType::tFloat && currentValue->floatValue == rule.from->floatValue) currentValue = rule.to;
                    }
                }

                if(!rule.flowVariable.empty()) setFlowData(rule.flowVariable, currentValue);
                else if(!rule.globalVariable.empty()) setFlowData(rule.globalVariable, currentValue);
                else if(!rule.messageProperty.empty()) (*value->structValue)[rule.messageProperty] = currentValue;
            }
        }
    }
    catch(const std::exception& ex)
    {
        _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
}

void MyNode::input(const Flows::PNodeInfo info, uint32_t index, const Flows::PVariable message)
{
	try
	{
		Flows::PVariable myMessage = std::make_shared<Flows::Variable>();
		*myMessage = *message;

		for(auto& rule : _rules)
		{
            applyRule(rule, myMessage);
			output(0, myMessage);
		}
	}
	catch(const std::exception& ex)
	{
		_out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
}

}
