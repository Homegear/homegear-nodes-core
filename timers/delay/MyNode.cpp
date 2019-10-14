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
	_stopThreads = true;
	_currentTimerThreadIndex = 0;
	_currentTimerThreadCount = 0;
}

MyNode::~MyNode()
{
	_stopThreads = true;
	waitForStop();
}


bool MyNode::init(Flows::PNodeInfo info)
{
	try
	{
		auto settingsIterator = info->info->structValue->find("delay");
		if(settingsIterator != info->info->structValue->end()) _delay = Flows::Math::getUnsignedNumber(settingsIterator->second->stringValue);

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
		_stopThreads = false;
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
		_stopThreads = true;
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
		std::lock_guard<std::mutex> timerGuard(_timerThreadsMutex);
		_stopThreads = true;
		for(auto& timerThread : _timerThreads)
		{
			if(timerThread.joinable()) timerThread.join();
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

void MyNode::timer(int64_t inputTime, Flows::PVariable message)
{
	int32_t sleepingTime = _delay - (Flows::HelperFunctions::getTime() - inputTime);
	if(sleepingTime < 1) sleepingTime = 1;
	else if(sleepingTime > _delay) sleepingTime = _delay;

	try
	{
		if(sleepingTime > 1000 && sleepingTime < 30000)
		{
			int32_t iterations = sleepingTime / 100;
			for(int32_t j = 0; j < iterations; j++)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				if(_stopThreads)
				{
					_currentTimerThreadCount--;
					return;
				}
			}
			if(sleepingTime % 100) std::this_thread::sleep_for(std::chrono::milliseconds(sleepingTime % 100));
		}
		else if(sleepingTime >= 30000)
		{
			int32_t iterations = sleepingTime / 1000;
			for(int32_t j = 0; j < iterations; j++)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
				if(_stopThreads)
				{
					_currentTimerThreadCount--;
					return;
				}
			}
			if(sleepingTime % 1000) std::this_thread::sleep_for(std::chrono::milliseconds(sleepingTime % 1000));
		}
		else std::this_thread::sleep_for(std::chrono::milliseconds(sleepingTime));
		if(_stopThreads)
		{
			_currentTimerThreadCount--;
			return;
		}

		output(0, message);
	}
	catch(const std::exception& ex)
	{
		_out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		_out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
	_currentTimerThreadCount--;
}

void MyNode::input(const Flows::PNodeInfo info, uint32_t index, const Flows::PVariable message)
{
	try
	{
		int64_t inputTime = Flows::HelperFunctions::getTime();
		std::lock_guard<std::mutex> timerGuard(_timerThreadsMutex);

		if(_currentTimerThreadCount == 10) return;

		_currentTimerThreadCount++;
		if(_timerThreads.at(_currentTimerThreadIndex).joinable()) _timerThreads.at(_currentTimerThreadIndex).join();
		_timerThreads.at(_currentTimerThreadIndex) = std::thread(&MyNode::timer, this, inputTime, message);

		_currentTimerThreadIndex++;
		if((unsigned)_currentTimerThreadIndex >= _timerThreads.size()) _currentTimerThreadIndex = 0;
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
