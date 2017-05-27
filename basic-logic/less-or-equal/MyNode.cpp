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

MyNode::MyNode(std::string path, std::string name, const std::atomic_bool* nodeEventsEnabled) : Flows::INode(path, name, nodeEventsEnabled)
{
	_lastLessOrEqual = false;
}

MyNode::~MyNode()
{
}

bool MyNode::start(Flows::PNodeInfo info)
{
	try
	{
		auto changesOnlyIterator = info->info->structValue->find("changes-only");
		if(changesOnlyIterator != info->info->structValue->end())
		{
			_outputChangesOnly = changesOnlyIterator->second->booleanValue;
		}
		auto falseIterator = info->info->structValue->find("output-false");
		if(falseIterator != info->info->structValue->end())
		{
			_outputFalse = falseIterator->second->booleanValue;
		}

		Flows::PArray parameters = std::make_shared<Flows::Array>();
		parameters->reserve(2);
		parameters->push_back(std::make_shared<Flows::Variable>(_id));
		parameters->push_back(std::make_shared<Flows::Variable>("input1"));
		Flows::PVariable result = invoke("getNodeData", parameters);
		if(!result->errorStruct) _input1 = result;

		parameters->at(1) = std::make_shared<Flows::Variable>("input2");
		result = invoke("getNodeData", parameters);
		if(!result->errorStruct) _input2 = result;

		_lastLessOrEqual = isLessOrEqual(_input1, _input2);

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

bool MyNode::isLessOrEqual(Flows::PVariable& input1, Flows::PVariable& input2)
{
	try
	{
		bool lessOrEqual = false;
		std::lock_guard<std::mutex> inputGuard(_inputMutex);
		if(input1->type == Flows::VariableType::tInteger || input1->type == Flows::VariableType::tInteger64)
		{
			if(input2->type == Flows::VariableType::tInteger || input2->type == Flows::VariableType::tInteger64)
			{
				lessOrEqual = input1->integerValue64 < input2->integerValue64;
			}
			else if(input2->type == Flows::VariableType::tFloat || input2->type == Flows::VariableType::tFloat)
			{
				lessOrEqual = input1->integerValue64 < input2->floatValue;
			}
		}
		else if(input1->type == Flows::VariableType::tFloat || input1->type == Flows::VariableType::tFloat)
		{
			if(input2->type == Flows::VariableType::tFloat || input2->type == Flows::VariableType::tFloat)
			{
				lessOrEqual = input1->floatValue < input2->floatValue;
			}
			else if(input2->type == Flows::VariableType::tInteger || input2->type == Flows::VariableType::tInteger64)
			{
				lessOrEqual = input1->floatValue < input2->integerValue64;
			}
		}
		return lessOrEqual;
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
		if(index == 0)
		{
			_input1 = message->structValue->at("payload");
			Flows::PArray parameters = std::make_shared<Flows::Array>();
			parameters->reserve(3);
			parameters->push_back(std::make_shared<Flows::Variable>(_id));
			parameters->push_back(std::make_shared<Flows::Variable>("input1"));
			parameters->push_back(_input1);
			invoke("setNodeData", parameters);
		}
		else if(index == 1)
		{
			_input2 = message->structValue->at("payload");
			Flows::PArray parameters = std::make_shared<Flows::Array>();
			parameters->reserve(3);
			parameters->push_back(std::make_shared<Flows::Variable>(_id));
			parameters->push_back(std::make_shared<Flows::Variable>("input2"));
			parameters->push_back(_input2);
			invoke("setNodeData", parameters);
		}

		bool lessOrEqual = isLessOrEqual(_input1, _input2);
		bool doOutput = !_outputChangesOnly;
		if(!lessOrEqual && !_outputFalse) doOutput = false;
		else if(lessOrEqual != _lastLessOrEqual) doOutput = true;
		_lastLessOrEqual = lessOrEqual;
		if(doOutput)
		{
			Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
			outputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>(lessOrEqual));
			output(0, outputMessage);
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

}
