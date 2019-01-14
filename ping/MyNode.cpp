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
		auto settingsIterator = info->info->structValue->find("host");
		if(settingsIterator != info->info->structValue->end()) _host = settingsIterator->second->stringValue;
		settingsIterator = info->info->structValue->find("interval");
		if(settingsIterator != info->info->structValue->end()) _interval = Flows::Math::getNumber(settingsIterator->second->stringValue);
		_enabled = true;

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
		//_statusLast = getNodeData("statusLast");
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
		//setNodeData("statusLast", _statusLast);
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
	try
	{
        int64_t cycleEndTime = 0;
        int64_t cycleTime = 0;
        int64_t cycleStartTime = Flows::HelperFunctions::getTime();
        int64_t timeToSleep = 1000 * _interval;
        int64_t sleepingTime = timeToSleep;
		while (!_stopThread)
		{
			if (sleepingTime >= timeToSleep)
			{
				sleepingTime -= timeToSleep;
	            bool reachable = false;
	                
				std::string pingOutput;
				std::string command = "ping -c 1 " + _host;
			    //_out->printInfo("Ping start for host " + _host);

				FILE* pipe = popen(command.c_str(), "r");
			    if (pipe)
				{
				    char buffer[128];
				    int32_t bytesRead = 0;
				    pingOutput.reserve(1024);
				    while(!feof(pipe))
				    {
				    	if(fgets(buffer, 128, pipe) != 0)
				    	{
				    		if(pingOutput.size() + bytesRead > pingOutput.capacity()) pingOutput.reserve(pingOutput.capacity() + 1024);
				    		pingOutput.insert(pingOutput.end(), buffer, buffer + strlen(buffer));
				    	}
				    }
				    pclose(pipe);
				    //_out->printInfo(pingOutput);
				    std::string findstr = "1 received";
				    std::size_t found = pingOutput.find(findstr);
				    reachable =  (found!=std::string::npos);
	  				//_out->printInfo("reachable: " + std::to_string(reachable));
				}
				else
			    	_out->printInfo("no pipe");
	            
			    if (reachable != _statusLast)
			    {
		            Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
		            outputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>(reachable));
		            output(0, outputMessage);
					_statusLast = reachable;
				}
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(100)); 
			cycleEndTime = Flows::HelperFunctions::getTime();
			cycleTime = cycleEndTime - cycleStartTime;
			sleepingTime += cycleTime;
			cycleStartTime = cycleEndTime;
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
        _enabled = *input;
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

