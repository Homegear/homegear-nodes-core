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
	_localRpcMethods.emplace("setConnectionState", std::bind(&MyNode::setConnectionState, this, std::placeholders::_1));
}

MyNode::~MyNode()
{
}

bool MyNode::init(Flows::PNodeInfo info)
{
	try
	{
        int32_t inputIndex = -1;

        auto settingsIterator = info->info->structValue->find("server");
        if(settingsIterator != info->info->structValue->end()) _server = settingsIterator->second->stringValue;

        settingsIterator = info->info->structValue->find("registers");
        if(settingsIterator != info->info->structValue->end())
        {
            for(auto& element : *settingsIterator->second->arrayValue)
            {
                inputIndex++;

                auto indexIterator = element->structValue->find("r");
                if(indexIterator == element->structValue->end()) continue;

                auto countIterator = element->structValue->find("c");
                if(countIterator == element->structValue->end()) continue;

                auto ibIterator = element->structValue->find("ib");
                if(ibIterator == element->structValue->end()) continue;

                auto irIterator = element->structValue->find("ir");
                if(irIterator == element->structValue->end()) continue;

                int32_t index = Flows::Math::getNumber(indexIterator->second->stringValue);
                int32_t count = Flows::Math::getNumber(countIterator->second->stringValue);

                if(index < 0 || count < 1) continue;

                auto registerInfo = std::make_shared<RegisterInfo>();
                registerInfo->inputIndex = (uint32_t)inputIndex;
                registerInfo->index = (uint32_t)index;
                registerInfo->count = (uint32_t)count;
                registerInfo->invertBytes = ibIterator->second->booleanValue;
                registerInfo->invertRegisters = irIterator->second->booleanValue;
                _registers.emplace(inputIndex, registerInfo);
            }
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
		Flows::PVariable payload = message->structValue->at("payload");
		if(payload->type == Flows::VariableType::tString) payload->binaryValue.insert(payload->binaryValue.end(), payload->stringValue.begin(), payload->stringValue.end());
        else if(payload->type == Flows::VariableType::tBoolean)
        {

        }
        else if(payload->type == Flows::VariableType::tInteger)
        {

        }
        else if(payload->type == Flows::VariableType::tFloat)
        {

        }
        payload->setType(Flows::VariableType::tBinary);
        if(payload->binaryValue.empty()) return;

        auto registersIterator = _registers.find(index);
        if(registersIterator == _registers.end()) return;

		Flows::PArray parameters = std::make_shared<Flows::Array>();
		parameters->reserve(5);
		parameters->push_back(std::make_shared<Flows::Variable>(registersIterator->second->index));
        parameters->push_back(std::make_shared<Flows::Variable>(registersIterator->second->count));
        parameters->push_back(std::make_shared<Flows::Variable>(registersIterator->second->invertBytes));
        parameters->push_back(std::make_shared<Flows::Variable>(registersIterator->second->invertRegisters));
		parameters->push_back(payload);

		invokeNodeMethod(_server, "writeRegisters", parameters, false);
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

//{{{ RPC methods
	Flows::PVariable MyNode::setConnectionState(Flows::PArray parameters)
	{
		try
		{
			if(parameters->size() != 1) return Flows::Variable::createError(-1, "Method expects exactly one parameter. " + std::to_string(parameters->size()) + " given.");
			if(parameters->at(0)->type != Flows::VariableType::tBoolean) return Flows::Variable::createError(-1, "Parameter is not of type boolean.");

			Flows::PVariable status = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
			if(parameters->at(0)->booleanValue)
			{
				status->structValue->emplace("text", std::make_shared<Flows::Variable>("connected"));
				status->structValue->emplace("fill", std::make_shared<Flows::Variable>("green"));
				status->structValue->emplace("shape", std::make_shared<Flows::Variable>("dot"));
			}
			else
			{
				status->structValue->emplace("text", std::make_shared<Flows::Variable>("disconnected"));
				status->structValue->emplace("fill", std::make_shared<Flows::Variable>("red"));
				status->structValue->emplace("shape", std::make_shared<Flows::Variable>("dot"));
			}
			nodeEvent("statusBottom/" + _id, status);

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
