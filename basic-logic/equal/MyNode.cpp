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
	_lastEqual = false;
}

MyNode::~MyNode()
{
}

bool MyNode::init(Flows::PNodeInfo info)
{
	try
	{
		auto settingsIterator = info->info->structValue->find("changes-only");
		if(settingsIterator != info->info->structValue->end()) _outputChangesOnly = settingsIterator->second->booleanValue;

		settingsIterator = info->info->structValue->find("output-false");
		if(settingsIterator != info->info->structValue->end()) _outputFalse = settingsIterator->second->booleanValue;

		int32_t inputs = 2;
		settingsIterator = info->info->structValue->find("inputs");
		if(settingsIterator != info->info->structValue->end()) inputs = settingsIterator->second->integerValue == 0 ? Flows::Math::getNumber(settingsIterator->second->stringValue) : settingsIterator->second->integerValue;

		_inputs.resize(inputs);

		for(uint32_t i = 0; i < _inputs.size(); i++)
		{
			_inputs.at(i) = getNodeData("input" + std::to_string(i));
		}

		_lastEqual = isEqual();

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

bool MyNode::isEqual()
{
	try
	{
		bool equal = false;
		std::lock_guard<std::mutex> inputGuard(_inputMutex);
		for(uint32_t i = 1; i < _inputs.size(); i++)
		{
			equal = false;
			if(_inputs[i - 1]->type == Flows::VariableType::tInteger || _inputs[i - 1]->type == Flows::VariableType::tInteger64)
			{
				if(_inputs[i]->type == Flows::VariableType::tInteger || _inputs[i]->type == Flows::VariableType::tInteger64)
				{
					equal = _inputs[i - 1]->integerValue64 == _inputs[i]->integerValue64;
				}
				else if(_inputs[i]->type == Flows::VariableType::tFloat || _inputs[i]->type == Flows::VariableType::tFloat)
				{
					equal = _inputs[i - 1]->integerValue64 == _inputs[i]->floatValue;
				}
			}
			else if(_inputs[i - 1]->type == Flows::VariableType::tFloat || _inputs[i - 1]->type == Flows::VariableType::tFloat)
			{
				if(_inputs[i]->type == Flows::VariableType::tFloat || _inputs[i]->type == Flows::VariableType::tFloat)
				{
					equal = _inputs[i - 1]->floatValue == _inputs[i]->floatValue;
				}
				else if(_inputs[i]->type == Flows::VariableType::tInteger || _inputs[i]->type == Flows::VariableType::tInteger64)
				{
					equal = _inputs[i - 1]->floatValue == _inputs[i]->integerValue64;
				}
			}
			if(!equal) return false;
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

void MyNode::input(Flows::PNodeInfo info, uint32_t index, Flows::PVariable message)
{
	try
	{
		if(index >= _inputs.size()) return;
		_inputs.at(index) = message->structValue->at("payload");
		setNodeData("input" + std::to_string(index), _inputs.at(index));

		bool lastEqual = isEqual();
		bool doOutput = !_outputChangesOnly;
		if(!lastEqual && !_outputFalse) doOutput = false;
		else if(lastEqual != _lastEqual) doOutput = true;
		_lastEqual = lastEqual;
		if(doOutput)
		{
			Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
			outputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>(lastEqual));
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
