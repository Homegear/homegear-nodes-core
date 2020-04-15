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

        std::string variableType = "device";
        auto settingsIterator = info->info->structValue->find("variabletype");
        if(settingsIterator != info->info->structValue->end()) variableType = settingsIterator->second->stringValue;

        if(variableType == "device") _variableType = VariableType::device;
        else if(variableType == "metadata") _variableType = VariableType::metadata;
        else if(variableType == "system") _variableType = VariableType::system;
        else if(variableType == "flow") _variableType = VariableType::flow;
        else if(variableType == "global") _variableType = VariableType::global;

        if(_variableType == VariableType::device || _variableType == VariableType::metadata)
        {
            settingsIterator = info->info->structValue->find("peerid");
            if(settingsIterator != info->info->structValue->end()) _peerId = Flows::Math::getNumber64(settingsIterator->second->stringValue);
        }

        if(_variableType == VariableType::device)
        {
            settingsIterator = info->info->structValue->find("channel");
            if(settingsIterator != info->info->structValue->end()) _channel = Flows::Math::getNumber(settingsIterator->second->stringValue);
        }

        settingsIterator = info->info->structValue->find("variable");
        if(settingsIterator != info->info->structValue->end()) _variable = settingsIterator->second->stringValue;

        if(variableType == "device") _variableType = VariableType::device;
        else if(variableType == "metadata") _variableType = VariableType::metadata;
        else if(variableType == "system") _variableType = VariableType::system;
        else if(variableType == "flow") _variableType = VariableType::flow;
        else if(variableType == "global") _variableType = VariableType::global;

		settingsIterator = info->info->structValue->find("checkall");
		if(settingsIterator != info->info->structValue->end()) _checkAll = settingsIterator->second->stringValue == "true" || settingsIterator->second->booleanValue;

		settingsIterator = info->info->structValue->find("output-true");
		if(settingsIterator != info->info->structValue->end()) _outputTrue = settingsIterator->second->booleanValue;

		if(_outputTrue)
		{
			settingsIterator = info->info->structValue->find("output-false");
			if(settingsIterator != info->info->structValue->end()) _outputFalse = settingsIterator->second->booleanValue;
		}
		else _outputFalse = false;

		settingsIterator = info->info->structValue->find("changes-only");
		if(settingsIterator != info->info->structValue->end()) _changesOnly = settingsIterator->second->booleanValue;

		settingsIterator = info->info->structValue->find("static-only");
		if(settingsIterator != info->info->structValue->end()) _staticOnly = settingsIterator->second->booleanValue;

		std::string staticValue;
		settingsIterator = info->info->structValue->find("static-value");
		if(settingsIterator != info->info->structValue->end()) staticValue = settingsIterator->second->stringValue;

		_payloadType = "int";
		settingsIterator = info->info->structValue->find("payloadType");
		if(settingsIterator != info->info->structValue->end()) _payloadType = settingsIterator->second->stringValue;

		_value = std::make_shared<Flows::Variable>(_payloadType, staticValue);

		Flows::PArray rules;
		settingsIterator = info->info->structValue->find("rules");
		if(settingsIterator != info->info->structValue->end()) rules = settingsIterator->second->arrayValue;

		_rules.clear();
		if(rules)
		{
			_rules.reserve(rules->size());
			uint32_t index = 0;
			for(auto& ruleStruct : *rules)
			{
				auto typeIterator = ruleStruct->structValue->find("t");
				auto valueIterator = ruleStruct->structValue->find("v");
				auto valueTypeIterator = ruleStruct->structValue->find("vt");

				if(typeIterator == ruleStruct->structValue->end()) continue;

				Rule rule;
				rule.previousOutput = getNodeData("previousOutputValue" + std::to_string(index));
				rule.t = getRuleTypeFromString(typeIterator->second->stringValue);

				if(valueIterator != ruleStruct->structValue->end() && valueTypeIterator != ruleStruct->structValue->end())
				{
					rule.v = valueIterator->second;
					rule.vt = getValueTypeFromString(valueTypeIterator->second->stringValue);
					if(valueTypeIterator->second->stringValue == "prev") rule.previousValue = true;
					else if(valueTypeIterator->second->stringValue == "input") rule.input = true;
					else if(valueTypeIterator->second->stringValue == "flow") rule.flowVariable = rule.v->stringValue;
					else if(valueTypeIterator->second->stringValue == "global") rule.globalVariable = rule.v->stringValue;
                    else if(valueTypeIterator->second->stringValue == "env") rule.envVariable = rule.v->stringValue;
					convertType(rule.v, rule.vt);
				}
				else rule.v = std::make_shared<Flows::Variable>();

				valueIterator = ruleStruct->structValue->find("v2");
				valueTypeIterator = ruleStruct->structValue->find("v2t");

				if(valueIterator != ruleStruct->structValue->end() && valueTypeIterator != ruleStruct->structValue->end())
				{
					rule.v2 = valueIterator->second;
					rule.v2t = getValueTypeFromString(valueTypeIterator->second->stringValue);
					if(valueTypeIterator->second->stringValue == "prev") rule.previousValue2 = true;
					else if(valueTypeIterator->second->stringValue == "input") rule.input2 = true;
					else if(valueTypeIterator->second->stringValue == "flow") rule.flowVariable2 = rule.v2->stringValue;
					else if(valueTypeIterator->second->stringValue == "global") rule.globalVariable2 = rule.v2->stringValue;
                    else if(valueTypeIterator->second->stringValue == "env") rule.envVariable2 = rule.v2->stringValue;
					convertType(rule.v2, rule.v2t);
				}
				else rule.v2 = std::make_shared<Flows::Variable>();

				if(rule.t == RuleType::tRegex)
				{
					auto caseIterator = ruleStruct->structValue->find("case");
					if(caseIterator != ruleStruct->structValue->end()) rule.ignoreCase = caseIterator->second->booleanValue;

					rule.regex = std::regex(rule.v->stringValue, rule.ignoreCase ? std::regex::icase : std::regex::ECMAScript);
				}

				_rules.emplace_back(std::move(rule));
				index++;
			}
		}

        _previousValue = getNodeData("previousValue");
		_previousInputValue = getNodeData("previousInputValue");

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

bool MyNode::match(const Flows::PNodeInfo& nodeInfo, Rule& rule, Flows::PVariable& value)
{
	try
	{
		if(value->type == Flows::VariableType::tInteger)
		{
			value->setType(Flows::VariableType::tInteger64);
			value->integerValue64 = value->integerValue;
		}
		if(rule.previousValue)
		{
			rule.v = _previousValue;
			rule.vt = _previousValue->type;
		}
		if(rule.previousValue2)
		{
			rule.v2 = _previousValue;
			rule.v2t = _previousValue->type;
		}
		if(rule.input)
		{
			rule.v = _previousInputValue;
			rule.vt = _previousInputValue->type;

			if(rule.t == RuleType::tRegex) rule.regex = std::regex(rule.v->stringValue, rule.ignoreCase ? std::regex::icase : std::regex::ECMAScript);
		}
		if(rule.input2)
		{
			rule.v2 = _previousInputValue;
			rule.v2t = _previousInputValue->type;
		}
		if(!rule.flowVariable.empty())
		{
			rule.v = getFlowData(rule.flowVariable);
			rule.vt = rule.v->type;
		}
		if(!rule.flowVariable2.empty())
		{
			rule.v2 = getFlowData(rule.flowVariable2);
			rule.v2t = rule.v->type;
		}
		if(!rule.globalVariable.empty())
		{
			rule.v = getGlobalData(rule.globalVariable);
			rule.vt = rule.v->type;
		}
		if(!rule.globalVariable2.empty())
		{
			rule.v2 = getGlobalData(rule.globalVariable2);
			rule.v2t = rule.v2->type;
		}
        if(!rule.envVariable.empty())
        {
            auto envIterator = nodeInfo->info->structValue->find("env");
            if(envIterator == nodeInfo->info->structValue->end()) return false;
            auto envIterator2 = envIterator->second->structValue->find(rule.envVariable);
            if(envIterator2 == envIterator->second->structValue->end()) return false;
            rule.v = envIterator2->second;
            rule.vt = rule.v->type;
        }
        if(!rule.envVariable2.empty())
        {
            auto envIterator = nodeInfo->info->structValue->find("env");
            if(envIterator == nodeInfo->info->structValue->end()) return false;
            auto envIterator2 = envIterator->second->structValue->find(rule.envVariable);
            if(envIterator2 == envIterator->second->structValue->end()) return false;
            rule.v2 = envIterator2->second;
            rule.v2t = rule.v2->type;
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
		_out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		_out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
	return false;
}

Flows::PVariable MyNode::getCurrentValue()
{
    try
    {
        if(_variableType == VariableType::device || _variableType == VariableType::metadata || _variableType == VariableType::system)
        {
            Flows::PArray parameters = std::make_shared<Flows::Array>();
            parameters->reserve(3);
            parameters->push_back(std::make_shared<Flows::Variable>(_peerId));
            parameters->push_back(std::make_shared<Flows::Variable>(_channel));
            parameters->push_back(std::make_shared<Flows::Variable>(_variable));
            auto payload = invoke("getValue", parameters);
            if(payload->errorStruct)
            {
                _out->printError("Error: Could not get value of variable: (Peer ID: " + std::to_string(_peerId) + ", channel: " + std::to_string(_channel) + ", name: " + _variable + ").");
            }
            else return payload;
        }
        else if(_variableType == VariableType::flow)
        {
            auto result = getFlowData(_variable);
            if(result->errorStruct)
            {
                _out->printError("Error: Could not get value of flow variable " + _variable + ".");
            }
            else return result;
        }
        else if(_variableType == VariableType::global)
        {
            auto result = getGlobalData(_variable);
            if(result->errorStruct)
            {
                _out->printError("Error: Could not get value of global variable " + _variable + ".");
            }
            else return result;
        }
    }
    catch(const std::exception& ex)
    {
        _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return Flows::PVariable();
}

void MyNode::input(const Flows::PNodeInfo info, uint32_t index, const Flows::PVariable message)
{
	try
	{
		Flows::PVariable inputValue = message->structValue->at("payload");
		Flows::PVariable currentValue = getCurrentValue();
		if(!currentValue) return;

		for(uint32_t i = 0; i < _rules.size(); i++)
		{
			if(match(info, _rules.at(i), currentValue))
			{
				if(_outputTrue)
				{
					if(!_changesOnly || !_rules.at(i).previousOutput->booleanValue)
					{
						Flows::PVariable trueMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
						Flows::PVariable trueValue = std::make_shared<Flows::Variable>(true);
						trueMessage->structValue->emplace("payload", trueValue);
						_rules.at(i).previousOutput = trueValue;
						setNodeData("previousOutputValue" + std::to_string(i), trueValue);
<<<<<<< HEAD
					    if(_staticOnly){
					    	message->structValue->emplace("payload", _value);
						   	output(i, message);
					    }else{
					    	output(i, trueMessage);
					    }
=======
						output(i, trueMessage);
>>>>>>> upstream/dev
					}
				}
				else
				{
					if(!_changesOnly || *(_rules.at(i).previousOutput) != *inputValue)
					{
						_rules.at(i).previousOutput = inputValue;
						setNodeData("previousOutputValue" + std::to_string(i), inputValue);
					    if(_staticOnly){
					    	message->structValue->emplace("payload", _value);
						   	output(i, message);
					    }else{
					    	output(i, message);
					    }
					}
				}
				if(!_checkAll) break;
			}
			else if(_outputFalse)
			{
				if(!_changesOnly || _rules.at(i).previousOutput->booleanValue)
				{
					Flows::PVariable falseValue = std::make_shared<Flows::Variable>(false);
					_rules.at(i).previousOutput = falseValue;
					setNodeData("previousOutputValue" + std::to_string(i), falseValue);
					Flows::PVariable falseMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
					falseMessage->structValue->emplace("payload", falseValue);
					output(i, falseMessage);
				}
			}
			else
			{
				_rules.at(i).previousOutput = std::make_shared<Flows::Variable>();
				setNodeData("previousOutputValue" + std::to_string(i), _rules.at(i).previousOutput);
			}
		}

        _previousValue = currentValue;
        setNodeData("previousValue", _previousInputValue);

		if(index == 0)
		{
            _previousInputValue = inputValue;
            setNodeData("previousInputValue", _previousInputValue);
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
