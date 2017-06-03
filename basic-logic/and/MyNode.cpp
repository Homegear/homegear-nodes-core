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
	_lastAnd = false;
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
		if(settingsIterator != info->info->structValue->end()) inputs = Flows::Math::getNumber(settingsIterator->second->stringValue);

		_inputs.resize(inputs);

		for(uint32_t i = 0; i < _inputs.size(); i++)
		{
			_inputs.at(i) = getNodeData("input" + std::to_string(i));
		}

		_lastAnd = doAnd();

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

bool MyNode::doAnd()
{
	try
	{
		std::lock_guard<std::mutex> inputGuard(_inputMutex);
		for(uint32_t i = 0; i < _inputs.size(); i++)
		{
			if(!_inputs[i]->booleanValue) return false;
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
		Flows::PVariable& input = message->structValue->at("payload");
		if(input->type != Flows::VariableType::tBoolean)
		{
			switch(input->type)
			{
			case Flows::VariableType::tArray:
				input->booleanValue = !input->arrayValue->empty();
				break;
			case Flows::VariableType::tBase64:
				input->booleanValue = !input->stringValue.empty();
				break;
			case Flows::VariableType::tBinary:
				input->booleanValue = !input->binaryValue.empty();
				break;
			case Flows::VariableType::tBoolean:
				break;
			case Flows::VariableType::tFloat:
				input->booleanValue = input->floatValue;
				break;
			case Flows::VariableType::tInteger:
				input->booleanValue = input->integerValue;
				break;
			case Flows::VariableType::tInteger64:
				input->booleanValue = input->integerValue64;
				break;
			case Flows::VariableType::tString:
				input->booleanValue = !input->stringValue.empty();
				break;
			case Flows::VariableType::tStruct:
				input->booleanValue = !input->structValue->empty();
				break;
			case Flows::VariableType::tVariant:
				break;
			case Flows::VariableType::tVoid:
				input->booleanValue = false;
				break;
			}
			input->setType(Flows::VariableType::tBoolean);
		}
		_inputs.at(index) = input;
		setNodeData("input" + std::to_string(index), _inputs.at(index));

		bool lastAnd = doAnd();
		bool doOutput = !_outputChangesOnly;
		if(!lastAnd && !_outputFalse) doOutput = false;
		else if(lastAnd != _lastAnd) doOutput = true;
		_lastAnd = lastAnd;
		if(doOutput)
		{
			Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
			outputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>(lastAnd));
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
