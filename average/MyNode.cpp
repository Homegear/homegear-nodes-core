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
	_inputIsDouble = false;
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
		auto settingsIterator = info->info->structValue->find("interval");
		if(settingsIterator != info->info->structValue->end()) _interval = Flows::Math::getNumber(settingsIterator->second->stringValue) * 1000;

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
	int32_t sleepingTime = _interval;
	if(sleepingTime < 1000) sleepingTime = 1000;

	int64_t startTime = Flows::HelperFunctions::getTime();
	while(!_stopThread)
	{
		try
		{
			if(sleepingTime > 1000 && sleepingTime < 30000)
			{
				int32_t iterations = sleepingTime / 100;
				for(int32_t j = 0; j < iterations; j++)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
					if(_stopThread) break;
				}
				if(sleepingTime % 100) std::this_thread::sleep_for(std::chrono::milliseconds(sleepingTime % 100));
			}
			else if(sleepingTime >= 30000)
			{
				int32_t iterations = sleepingTime / 1000;
				for(int32_t j = 0; j < iterations; j++)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(1000));
					if(_stopThread) break;
				}
				if(sleepingTime % 1000) std::this_thread::sleep_for(std::chrono::milliseconds(sleepingTime % 1000));
			}
			if(_stopThread) break;

			double average = 0.0;

			{
				std::lock_guard<std::mutex> valuesGuard(_valuesMutex);
				for(auto value : _values)
				{
					average += value;
				}
				if(!_values.empty()) average /= _values.size();
				_values.clear();
			}

			Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
			message->structValue->emplace("payload", std::make_shared<Flows::Variable>(_inputIsDouble ? average : std::lround(average)));
			output(0, message);
			int64_t diff = Flows::HelperFunctions::getTime() - startTime;
			if(diff <= _interval) sleepingTime = _interval;
			else sleepingTime = _interval - (diff - _interval);
			if(sleepingTime < 1000) sleepingTime = 1000;
			startTime = Flows::HelperFunctions::getTime();
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

void MyNode::input(Flows::PNodeInfo info, uint32_t index, Flows::PVariable message)
{
	try
	{
		Flows::PVariable& input = message->structValue->at("payload");
        std::lock_guard<std::mutex> valuesGuard(_valuesMutex);
		if(input->type == Flows::VariableType::tInteger || input->type == Flows::VariableType::tInteger64)
		{
			_values.emplace_back(input->integerValue64);
			_inputIsDouble = false;
		}
		else if(input->type == Flows::VariableType::tFloat)
		{
			_values.emplace_back(input->floatValue);
			_inputIsDouble = true;
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

