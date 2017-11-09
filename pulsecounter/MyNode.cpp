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
		std::lock_guard<std::mutex> workerGuard(_workerThreadMutex);
            	_stopThread = true;
		if (_workerThread.joinable())_workerThread.join();
		_stopThread = false;
		_workerThread = std::thread(&MyNode::worker, this, _maxgap);
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

void MyNode::worker(int64_t maxgap)
{
	try
	{
		int64_t cycleStartTime = Flows::HelperFunctions::getTime();
		int64_t aktTime = 0;
		int64_t triggerTimeLast = 0;
		int64_t cycleEndTime = 0;
		int64_t cycleTime = 0;
		int64_t actGapTime = 0;
		int64_t lastGapTime = 0;
		int64_t lastGapTimeSaved = getNodeData("lastGapTimeSaved")->integerValue64;
		int64_t lastGapTimeToSave = 0;
        uint32_t counts = 0;
		float countsPerMinute = 0;
		bool running = false;
		while (!_stopThread)
		{
			aktTime = Flows::HelperFunctions::getTime();

            {
                std::lock_guard<std::mutex> countsMutex(_countsMutex);
                counts = _counts;
                _counts = 0;
            }

			if (counts > 0)
			{
				triggerTimeLast = aktTime;
				lastGapTime = actGapTime;
				actGapTime = 0;
			}
			running = ((triggerTimeLast > 0) && ((aktTime - triggerTimeLast) < maxgap));
            if(running)
            {
                int64_t calcTime = lastGapTime;
                if (lastGapTimeSaved > 0)
                {
                    calcTime = lastGapTimeSaved;
                    lastGapTimeSaved = 0;
                }
                else if (actGapTime > lastGapTime)
                {
                    calcTime = actGapTime;
                }
                if(calcTime == 0) calcTime = 1;
                countsPerMinute = (60000.0 / calcTime) * ((counts == 0) ? 1 : counts);
                lastGapTimeToSave = calcTime;
            }
            else
            {
                countsPerMinute = 0.0;
            }
            Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
            outputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>(countsPerMinute));
            output(0, outputMessage); //countsPerMinute

			std::this_thread::sleep_for(std::chrono::milliseconds(1000)); //Limit to one output per second
			cycleEndTime = Flows::HelperFunctions::getTime();
			cycleTime = cycleEndTime - cycleStartTime;
			if(running) actGapTime += cycleTime;
			cycleStartTime = cycleEndTime;
		}

		setNodeData("lastGapTimeSaved", std::make_shared<Flows::Variable>(lastGapTimeToSave));
		_threadRunning = false;
	}
	catch(const std::exception& ex)
	{
		_threadRunning = false;
		_out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		_threadRunning = false;
		_out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
}

void MyNode::input(Flows::PNodeInfo info, uint32_t index, Flows::PVariable message)
{
	try
	{
		Flows::PVariable& input = message->structValue->at("payload");
        if(!*input) return;
		std::lock_guard<std::mutex> countsMutex(_countsMutex);
		_counts++;
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

