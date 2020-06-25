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
#include "Change.h"
#include "../switch/Switch.h"

namespace Change
{

Change::Change(std::string path, std::string nodeNamespace, std::string type, const std::atomic_bool* frontendConnected) : Flows::INode(path, nodeNamespace, type, frontendConnected)
{
}

Change::~Change()
{
}

std::string& Change::stringReplace(std::string& haystack, const std::string& search, const std::string& replace)
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

Change::RuleType Change::getRuleTypeFromString(std::string& t)
{
	Change::RuleType ruleType = Change::RuleType::tSet;
	if(t == "set") ruleType = Change::RuleType::tSet;
	else if(t == "change") ruleType = Change::RuleType::tChange;
	else if(t == "move") ruleType = Change::RuleType::tMove;
	else if(t == "delete") ruleType = Change::RuleType::tDelete;
	return ruleType;
}

Flows::VariableType Change::getValueTypeFromString(std::string& vt)
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

void Change::convertType(Flows::PVariable& value, Flows::VariableType vt)
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

bool Change::init(Flows::PNodeInfo info)
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
					if(valueTypeIterator->second->stringValue == "message") rule.messageProperty = Flows::MessageProperty(valueIterator->second->stringValue);
					else if(valueTypeIterator->second->stringValue == "flow") rule.flowVariable = valueIterator->second->stringValue;
					else if(valueTypeIterator->second->stringValue == "global") rule.globalVariable = valueIterator->second->stringValue;
				}


                valueIterator = ruleStruct->structValue->find("from");
                valueTypeIterator = ruleStruct->structValue->find("fromt");
                if(valueIterator != ruleStruct->structValue->end() && valueTypeIterator != ruleStruct->structValue->end())
                {
                    rule.from = valueIterator->second;
                    rule.fromt = getValueTypeFromString(valueTypeIterator->second->stringValue);
                    if(valueTypeIterator->second->stringValue == "message") rule.messagePropertyFrom = Flows::MessageProperty(rule.from->stringValue);
                    else if(valueTypeIterator->second->stringValue == "flow") rule.flowVariableFrom = rule.from->stringValue;
                    else if(valueTypeIterator->second->stringValue == "global") rule.globalVariableFrom = rule.from->stringValue;
                    convertType(rule.from, rule.fromt);
                }
                else rule.from = std::make_shared<Flows::Variable>();
                if(valueTypeIterator != ruleStruct->structValue->end() && valueTypeIterator->second->stringValue == "regex")
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
                    if(valueTypeIterator->second->stringValue == "message") rule.messagePropertyTo = Flows::MessageProperty(rule.to->stringValue);
                    else if(valueTypeIterator->second->stringValue == "flow") rule.flowVariableTo = rule.to->stringValue;
                    else if(valueTypeIterator->second->stringValue == "global") rule.globalVariableTo = rule.to->stringValue;
                    else if(valueTypeIterator->second->stringValue == "env") rule.envVariableTo = rule.to->stringValue;
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

void Change::applyRule(const Flows::PNodeInfo& nodeInfo, Rule& rule, Flows::PVariable& value)
{
    try
    {
        if(!rule.messageProperty.empty() || !rule.flowVariable.empty() || !rule.globalVariable.empty())
        {
            if(rule.t == RuleType::tDelete)
            {
                if(!rule.flowVariable.empty()) setFlowData(rule.flowVariable, std::make_shared<Flows::Variable>());
                else if(!rule.globalVariable.empty()) setFlowData(rule.globalVariable, std::make_shared<Flows::Variable>());
                else if(!rule.messageProperty.empty()) rule.messageProperty.erase(value);
            }
            else if(rule.t == RuleType::tSet)
            {
                if(!rule.flowVariableTo.empty()) rule.to = getFlowData(rule.flowVariableTo);
                else if(!rule.globalVariableTo.empty()) rule.to = getGlobalData(rule.globalVariableTo);
                else if(!rule.envVariableTo.empty())
                {
                    auto envIterator = nodeInfo->info->structValue->find("env");
                    if(envIterator == nodeInfo->info->structValue->end()) return;
                    auto envIterator2 = envIterator->second->structValue->find(rule.envVariableTo);
                    if(envIterator2 == envIterator->second->structValue->end()) return;
                    rule.to = envIterator2->second;
                }
                else if(!rule.messagePropertyTo.empty())
                {
                    rule.to = rule.messagePropertyTo.match(value);
                    if(!rule.to) rule.to = std::make_shared<Flows::Variable>();
                }

                if(!rule.flowVariable.empty()) setFlowData(rule.flowVariable, rule.to);
                else if(!rule.globalVariable.empty()) setGlobalData(rule.globalVariable, rule.to);
                else if(!rule.messageProperty.empty()) rule.messageProperty.set(value, rule.to);
            }
            else if(rule.t == RuleType::tMove)
            {
                Flows::PVariable currentValue;

                //{{{ Set
                    if(!rule.flowVariable.empty()) currentValue = getFlowData(rule.flowVariable);
                    else if(!rule.globalVariable.empty()) currentValue = getGlobalData(rule.globalVariable);
                    else if(!rule.messageProperty.empty())
                    {
                        currentValue = rule.messageProperty.match(value);
                        if(!currentValue) currentValue = std::make_shared<Flows::Variable>();
                    }
                    else currentValue = std::make_shared<Flows::Variable>();

                    if(!rule.flowVariableTo.empty()) setFlowData(rule.flowVariableTo, currentValue);
                    else if(!rule.globalVariableTo.empty()) setFlowData(rule.globalVariableTo, currentValue);
                    else if(!rule.messagePropertyTo.empty()) rule.messagePropertyTo.set(value, currentValue);
                //}}}

                //{{{ Delete
                    if(!rule.flowVariable.empty()) setFlowData(rule.flowVariable, std::make_shared<Flows::Variable>());
                    else if(!rule.globalVariable.empty()) setFlowData(rule.globalVariable, std::make_shared<Flows::Variable>());
                    else if(!rule.messageProperty.empty()) rule.messageProperty.erase(value);
                //}}}
            }
            else if(rule.t == RuleType::tChange)
            {
                Flows::PVariable haystack;
                Flows::PVariable fromValue;
                Flows::PVariable toValue;

                if(!rule.flowVariable.empty()) haystack = getFlowData(rule.flowVariable);
                else if(!rule.globalVariable.empty()) haystack = getGlobalData(rule.globalVariable);
                else if(!rule.messageProperty.empty())
                {
                    haystack = rule.messageProperty.match(value);
                    if(!haystack) haystack = std::make_shared<Flows::Variable>();
                }
                else haystack = std::make_shared<Flows::Variable>();

                if(!rule.flowVariableFrom.empty()) fromValue = getFlowData(rule.flowVariableFrom);
                else if(!rule.globalVariableFrom.empty()) fromValue = getGlobalData(rule.globalVariableFrom);
                else if(!rule.messagePropertyFrom.empty())
                {
                    fromValue = rule.messagePropertyFrom.match(value);
                    if(!fromValue) fromValue = std::make_shared<Flows::Variable>();
                }
                else if(rule.from) fromValue = rule.from;
                else fromValue = std::make_shared<Flows::Variable>();

                if(!rule.flowVariableTo.empty()) toValue = getFlowData(rule.flowVariableTo);
                else if(!rule.globalVariableTo.empty()) toValue = getGlobalData(rule.globalVariableTo);
                else if(!rule.messagePropertyTo.empty())
                {
                    toValue = rule.messagePropertyTo.match(value);
                    if(!toValue) toValue = std::make_shared<Flows::Variable>();
                }
                else if(rule.to) toValue = rule.to;
                else toValue = std::make_shared<Flows::Variable>();

                if(rule.fromRegexSet)
                {
                    if(haystack->type != Flows::VariableType::tString)
                    {
                        haystack->stringValue = haystack->toString();
                        haystack->type = Flows::VariableType::tString;
                    }

                    if(rule.tot != Flows::VariableType::tString)
                    {
                        if(!std::regex_match(haystack->stringValue, rule.fromRegex)) return;
                        haystack = toValue;
                    }
                    else haystack->stringValue = std::regex_replace(haystack->stringValue, rule.fromRegex, toValue->toString());
                }
                else
                {
                    if(fromValue->type == Flows::VariableType::tString)
                    {
                        if(haystack->type != Flows::VariableType::tString)
                        {
                            haystack->stringValue = haystack->toString();
                            haystack->type = Flows::VariableType::tString;
                        }

                        if(haystack->stringValue.find(fromValue->stringValue) == std::string::npos) return;

                        if(rule.tot != Flows::VariableType::tString) haystack = toValue;
                        else stringReplace(haystack->stringValue, fromValue->stringValue, toValue->toString());
                    }
                    else
                    {
                        if(haystack->type != fromValue->type) return;

                        if(haystack->type == Flows::VariableType::tBoolean && haystack->booleanValue == fromValue->booleanValue) haystack = toValue;
                        else if(haystack->type == Flows::VariableType::tInteger64 && haystack->integerValue64 == fromValue->integerValue64) haystack = toValue;
                        else if(haystack->type == Flows::VariableType::tFloat && haystack->floatValue == fromValue->floatValue) haystack = toValue;
                    }
                }

                if(!rule.flowVariable.empty()) setFlowData(rule.flowVariable, haystack);
                else if(!rule.globalVariable.empty()) setFlowData(rule.globalVariable, haystack);
                else if(!rule.messageProperty.empty()) rule.messageProperty.set(value, haystack);
            }
        }
    }
    catch(const std::exception& ex)
    {
        _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
}

void Change::input(const Flows::PNodeInfo info, uint32_t index, const Flows::PVariable message)
{
	try
	{
		Flows::PVariable myMessage = std::make_shared<Flows::Variable>();
		*myMessage = *message;

		for(auto& rule : _rules)
		{
            applyRule(info, rule, myMessage);
		}

        output(0, myMessage);
	}
	catch(const std::exception& ex)
	{
		_out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
}

}
