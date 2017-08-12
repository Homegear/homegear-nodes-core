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
	_threadRunning = false;
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
		auto settingsIterator = info->info->structValue->find("on-delay");
		if(settingsIterator != info->info->structValue->end()) _delay = Flows::Math::getUnsignedNumber(settingsIterator->second->stringValue);

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
		int64_t delayTo = getNodeData("delayTo")->integerValue64;
		if (delayTo > 0)
		{
			std::lock_guard<std::mutex> timerGuard(_timerThreadMutex);
			_stopThread = true;
			if (_timerThread.joinable())_timerThread.join();
			_stopThread = false;
			_timerThread = std::thread(&MyNode::timer, this, delayTo);
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

void MyNode::stop()
{
	try
	{
		std::lock_guard<std::mutex> timerGuard(_timerThreadMutex);
		_stopThread = true;
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

void MyNode::waitForStop()
{
	try
	{
		std::lock_guard<std::mutex> timerGuard(_timerThreadMutex);
		_stopThread = true;
		if (_timerThread.joinable()) _timerThread.join();
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

void MyNode::timer(int64_t delayTo)
{
	try
	{
		int64_t restTime = delayTo - Flows::HelperFunctions::getTime();
		int64_t sleepingTime = 10;
		if(_delay >= 1000) sleepingTime = 100;
		else if(_delay >= 30000) sleepingTime = 1000;

		while (restTime > 0)
		{
			if ((restTime / sleepingTime) % (1000 / sleepingTime) == 0)
			{
				Flows::PVariable outputMessage2 = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
				outputMessage2->structValue->emplace("payload", std::make_shared<Flows::Variable>((restTime / sleepingTime) * sleepingTime));
				output(1, outputMessage2); //rest time
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(sleepingTime));
			if(_stopThread)
			{
				Flows::PVariable outputMessage3 = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
				outputMessage3->structValue->emplace("payload", std::make_shared<Flows::Variable>(-1));
				output(1, outputMessage3); //rest time
				setNodeData("delayTo", std::make_shared<Flows::Variable>(0));
				_threadRunning = false;
				return;
			}

			restTime = delayTo - Flows::HelperFunctions::getTime();
		}
		_threadRunning = false;

		Flows::PVariable outputMessage2 = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
		outputMessage2->structValue->emplace("payload", std::make_shared<Flows::Variable>(0));
		output(1, outputMessage2); //rest time

		Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
		outputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>(true));
		output(0, outputMessage); //true
		_lastOutputState = true;

		setNodeData("delayTo", std::make_shared<Flows::Variable>(0));
	}
	catch(const std::exception& ex)
	{
		_threadRunning = false;
		Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		_threadRunning = false;
		Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
}

void MyNode::input(Flows::PNodeInfo info, uint32_t index, Flows::PVariable message)
{
	try
	{
		std::lock_guard<std::mutex> timerGuard(_timerThreadMutex);
		Flows::PVariable& input = message->structValue->at("payload");
		if (*input)
		{
			if(!_threadRunning && !_lastOutputState)
			{
				int64_t delayTo = _delay + Flows::HelperFunctions::getTime();
				setNodeData("delayTo", std::make_shared<Flows::Variable>(delayTo));
				if (_timerThread.joinable())_timerThread.join();
				_stopThread = false;
				_threadRunning = true;
				_timerThread = std::thread(&MyNode::timer, this, delayTo);
			}
		}
		else
		{
			_stopThread = true;
			if (_lastOutputState == true)  //Only fire "false" once
			{
				Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
				outputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>(false));
				output(0, outputMessage); //false
			}
			_lastOutputState = false;
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

