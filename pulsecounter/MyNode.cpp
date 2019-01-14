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
            std::lock_guard<std::mutex> listGuard(_listMutex);
            auto pulses = getNodeData("pulses");
            for (auto& pulse : *pulses->arrayValue)
            {
                _pulses.push_back(pulse->integerValue64);
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
        std::lock_guard<std::mutex> listGuard(_listMutex);
        Flows::PVariable pulses = std::make_shared<Flows::Variable>(Flows::VariableType::tArray);
        pulses->arrayValue->reserve(_pulses.size());
        while(!_pulses.empty())
        {
            pulses->arrayValue->push_back(std::make_shared<Flows::Variable>(_pulses.front()));
            _pulses.pop_front();
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
        float duration = 0;
		while (!_stopThread)
		{
            {
                now = BaseLib::HelperFunctions::getTime();
                std::lock_guard<std::mutex> listGuard(_listMutex);
                while(!_pulses.empty() && now - _pulses.front() > _maxgap) _pulses.pop_front();
                
                int64_t x_last = 0;
                int64_t durationAll = 0;
                for (auto it = _pulses.begin(); it != _pulses.end(); ++it)
                {
				    int64_t x = *it;
				    if (x_last != 0)
				    {
				    	duration = x - x_last;
				    	durationAll += duration;
				    	//_out->printInfo(std::to_string(duration) + "ms");
					}
				    x_last = x;
				}
				size = _pulses.size();
				if (size > 1)
					duration = (60000.0 / ((float)durationAll / (float)(size - 1)));
				else
					duration = 0.0;
				//_out->printInfo("---- AVG: " + std::to_string(duration) + " cnt/min");
            }

            Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
            outputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>(duration));
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
		std::lock_guard<std::mutex> listGuard(_listMutex);
		_pulses.push_back(BaseLib::HelperFunctions::getTime());
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

