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
	_stopThread = true;
}

MyNode::~MyNode()
{
	stop();
}

uint32_t MyNode::getNumber(std::string& s, bool isHex)
{
	int32_t xpos = s.find('x');
	int32_t number = 0;
	if(xpos == -1 && !isHex) try { number = std::stoull(s, 0, 10); } catch(...) {}
	else try { number = std::stoull(s, 0, 16); } catch(...) {}
	return number;
}


bool MyNode::init(Flows::PNodeInfo info)
{
	try
	{
		auto settingsIterator = info->info->structValue->find("interval");
		if(settingsIterator != info->info->structValue->end()) _interval = getNumber(settingsIterator->second->stringValue);

		settingsIterator = info->info->structValue->find("resetafter");
		if(settingsIterator != info->info->structValue->end()) _resetAfter = getNumber(settingsIterator->second->stringValue);

		if(_interval < 10) _interval = 10;

		_enabled = getNodeData("enabled")->booleanValue;

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

bool MyNode::start()
{
	try
	{
		if(!_enabled) return true;
		std::lock_guard<std::mutex> timerGuard(_timerMutex);
		_stopThread = false;
		if(_timerThread.joinable()) _timerThread.join();
		_timerThread = std::thread(&MyNode::timer, this);

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

void MyNode::stop()
{
	try
	{
		std::lock_guard<std::mutex> timerGuard(_timerMutex);
		_stopThread = true;
		if(_timerThread.joinable()) _timerThread.join();
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

void MyNode::timer()
{
	uint32_t i = 0;
	int32_t sleepingTime = _interval;

	Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
	message->structValue->emplace("payload", std::make_shared<Flows::Variable>(i));

	int64_t startTimeAll = Flows::HelperFunctions::getTime();
	int64_t startTime = Flows::HelperFunctions::getTime();

	while(!_stopThread)
	{
		try
		{
			i++;
			if(sleepingTime > 1000 && sleepingTime < 30000)
			{
				int32_t iterations = sleepingTime / 100;
				for(int32_t j = 0; j < iterations; j++)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
				}
				if(sleepingTime % 100) std::this_thread::sleep_for(std::chrono::milliseconds(sleepingTime % 100));
			}
			else if(sleepingTime >= 30000)
			{
				int32_t iterations = sleepingTime / 1000;
				for(int32_t j = 0; j < iterations; j++)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(1000));
				}
				if(sleepingTime % 1000) std::this_thread::sleep_for(std::chrono::milliseconds(sleepingTime % 1000));
			}
			else std::this_thread::sleep_for(std::chrono::milliseconds(sleepingTime));
			message->structValue->at("payload")->integerValue = i;
			if(_resetAfter > 0 && Flows::HelperFunctions::getTime() - startTimeAll >= _resetAfter)
			{
				_stopThread = true;
				_enabled = false;
				setNodeData("enabled", std::make_shared<Flows::Variable>(_enabled));
				Flows::PVariable status = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
				status->structValue->emplace("text", std::make_shared<Flows::Variable>("disabled"));
				nodeEvent("statusTop/" + _id, status);
				break;
			}
			output(0, message);
			int64_t diff = Flows::HelperFunctions::getTime() - startTime;
			if(diff <= _interval) sleepingTime = _interval;
			else sleepingTime = _interval - (diff - _interval);
			if(sleepingTime < 10) sleepingTime = 10;
			startTime = Flows::HelperFunctions::getTime();
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

void MyNode::input(Flows::PNodeInfo info, uint32_t index, Flows::PVariable message)
{
	try
	{
		_enabled = message->structValue->at("payload")->booleanValue;
		setNodeData("enabled", std::make_shared<Flows::Variable>(_enabled));
		std::lock_guard<std::mutex> timerGuard(_timerMutex);
		if(_enabled)
		{
			if(_stopThread)
			{
				_stopThread = false;
				if(_timerThread.joinable()) _timerThread.join();
				_timerThread = std::thread(&MyNode::timer, this);
			}
		}
		else
		{
			_stopThread = true;
			if(_timerThread.joinable()) _timerThread.join();
		}
		Flows::PVariable status = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
		status->structValue->emplace("text", std::make_shared<Flows::Variable>(_enabled ? "enabled" : "disabled"));
		nodeEvent("statusTop/" + _id, status);
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
