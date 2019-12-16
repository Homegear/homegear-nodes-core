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
        auto settingsIterator = info->info->structValue->find("true-only");
        if(settingsIterator != info->info->structValue->end()) _trueOnly = settingsIterator->second->booleanValue;

        settingsIterator = info->info->structValue->find("outputs");
        if(settingsIterator != info->info->structValue->end()) _outputs = settingsIterator->second->integerValue == 0 ? Flows::Math::getUnsignedNumber(settingsIterator->second->stringValue) : settingsIterator->second->integerValue;

        _currentOutputIndex = getNodeData("currentOutputIndex")->integerValue;
        auto nodeData = getNodeData("forward");
        if(nodeData->type == Flows::VariableType::tBoolean) _forward = nodeData->booleanValue;

		return true;
	}
	catch(const std::exception& ex)
	{
		_out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	return false;
}

void MyNode::input(const Flows::PNodeInfo info, uint32_t index, const Flows::PVariable message)
{
	try
    {
	    if(index == 0 || index == 1) //Toggle forward
        {
            if(_trueOnly && !(bool)(*message->structValue->at("payload"))) return;

            Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
            message->structValue->emplace("payload", std::make_shared<Flows::Variable>(true));

            if(index == 0)
            {
                if(!_forward)
                {
                    _forward = true;
                    for(int32_t i = 0; i < 2; i++)
                    {
                        _currentOutputIndex++;
                        if (_currentOutputIndex >= _outputs) _currentOutputIndex = 0;
                    }
                    setNodeData("forward", std::make_shared<Flows::Variable>(_forward));
                }
                output(_currentOutputIndex, message);
                _currentOutputIndex++;
                if(_currentOutputIndex >= _outputs) _currentOutputIndex = 0;
            }
            else
            {
                if(_forward)
                {
                    _forward = false;
                    for(int32_t i = 0; i < 2; i++)
                    {
                        _currentOutputIndex--;
                        if (_currentOutputIndex < 0) _currentOutputIndex = _outputs - 1;
                    }
                    setNodeData("forward", std::make_shared<Flows::Variable>(_forward));
                }
                output(_currentOutputIndex, message);
                _currentOutputIndex--;
                if(_currentOutputIndex < 0) _currentOutputIndex = _outputs - 1;
            }
            setNodeData("currentOutputIndex", std::make_shared<Flows::Variable>(_currentOutputIndex));
        }
	    else //Reset
        {
	        if(message->structValue->at("payload")->type == Flows::VariableType::tInteger64) _currentOutputIndex = message->structValue->at("payload")->integerValue64;
	        else
            {
                if (!(bool) (*message->structValue->at("payload"))) return;
                _currentOutputIndex = 0;
            }
            setNodeData("currentOutputIndex", std::make_shared<Flows::Variable>(_currentOutputIndex));
        }
	}
	catch(const std::exception& ex)
	{
		_out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
}

}
