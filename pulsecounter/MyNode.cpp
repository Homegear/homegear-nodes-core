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

#include <homegear-base/HelperFunctions/HelperFunctions.h>
#include "MyNode.h"

namespace MyNode
{

MyNode::MyNode(std::string path, std::string nodeNamespace, std::string type, const std::atomic_bool* frontendConnected) : Flows::INode(path, nodeNamespace, type, frontendConnected)
{
	_stopThread = true;
}

MyNode::~MyNode()
{
	_stopThread = true;
	waitForStop();
}


bool MyNode::init(Flows::PNodeInfo info)
{
	try
	{
		auto settingsIterator = info->info->structValue->find("maxgap");
		if(settingsIterator != info->info->structValue->end()) _maxgap = Flows::Math::getNumber(settingsIterator->second->stringValue);

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

bool MyNode::start()
{
	try
	{
        {
            std::lock_guard<std::mutex> queueGuard(_queueMutex);
            auto pulses = getNodeData("pulses");
            for (auto& pulse : *pulses->arrayValue)
            {
                _pulses.push(pulse->integerValue64);
            }
        }

        std::lock_guard<std::mutex> workerGuard(_workerThreadMutex);
        _stopThread = true;
		if (_workerThread.joinable())_workerThread.join();
		_stopThread = false;
		_workerThread = std::thread(&MyNode::worker, this);
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

void MyNode::stop()
{
	try
	{
		std::lock_guard<std::mutex> workerGuard(_workerThreadMutex);
		_stopThread = true;
        std::lock_guard<std::mutex> queueGuard(_queueMutex);
        Flows::PVariable pulses = std::make_shared<Flows::Variable>(Flows::VariableType::tArray);
        pulses->arrayValue->reserve(_pulses.size());
        while(!_pulses.empty())
        {
            pulses->arrayValue->push_back(std::make_shared<Flows::Variable>(_pulses.front()));
            _pulses.pop();
        }
        setNodeData("pulses", pulses);
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

void MyNode::waitForStop()
{
	try
	{
		std::lock_guard<std::mutex> workerGuard(_workerThreadMutex);
		_stopThread = true;
		if (_workerThread.joinable()) _workerThread.join();
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

void MyNode::worker()
{
	try
	{
        int64_t now = 0;
        int64_t size = 0;
		while (!_stopThread)
		{
            {
                now = BaseLib::HelperFunctions::getTime();
                std::lock_guard<std::mutex> queueGuard(_queueMutex);
                while(!_pulses.empty() && now - _pulses.front() > _maxgap) _pulses.pop();
                size = _pulses.size();
            }

            Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
            outputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>(size));
            output(0, outputMessage); //countsPerMinute

			std::this_thread::sleep_for(std::chrono::milliseconds(1000)); //Limit to one output per second
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

void MyNode::input(Flows::PNodeInfo info, uint32_t index, Flows::PVariable message)
{
	try
	{
		Flows::PVariable& input = message->structValue->at("payload");
        if(!*input) return;
		std::lock_guard<std::mutex> queueGuard(_queueMutex);
		_pulses.push(BaseLib::HelperFunctions::getTime());
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

