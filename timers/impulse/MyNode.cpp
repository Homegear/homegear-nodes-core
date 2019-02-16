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

MyNode::MyNode(std::string path, std::string nodeNamespace, std::string type, const std::atomic_bool* frontendConnected)
        : Flows::INode(path, nodeNamespace, type, frontendConnected)
{
    _stopThread = true;
    _stopped = true;
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
        auto settingsIterator = info->info->structValue->find("impulse");
        if(settingsIterator != info->info->structValue->end()) _delay = Flows::Math::getUnsignedNumber(settingsIterator->second->stringValue);

        settingsIterator = info->info->structValue->find("allow-retrigger");
        if(settingsIterator != info->info->structValue->end()) _allowRetrigger = settingsIterator->second->booleanValue;

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
        _stopped = false;

        _lastInputState = getNodeData("lastInputState")->booleanValue;

        int64_t delayTo = getNodeData("delayTo")->integerValue64;
        _delayTo.store(delayTo, std::memory_order_release);
        if(delayTo > 0)
        {
            std::lock_guard<std::mutex> timerGuard(_timerThreadMutex);
            _stopThread = true;
            if(_timerThread.joinable())_timerThread.join();
            _stopThread = false;
            _timerThread = std::thread(&MyNode::timer, this, false);
        }

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
        _stopped = true;
        std::lock_guard<std::mutex> timerGuard(_timerThreadMutex);
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
        std::lock_guard<std::mutex> timerGuard(_timerThreadMutex);
        _stopThread = true;
        if(_timerThread.joinable()) _timerThread.join();
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

void MyNode::timer(bool outputTrue)
{
    try
    {
        int64_t delayTo = _delayTo.load(std::memory_order_acquire);
        int64_t restTime = delayTo - Flows::HelperFunctions::getTime();
        int64_t sleepingTime = 10;
        if(_delay >= 1000) sleepingTime = 100;
        else if(_delay >= 30000) sleepingTime = 1000;

        if(outputTrue)
        {
            Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
            outputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>(true));
            output(0, outputMessage); //true
        }

        while(restTime > 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepingTime));
            if(_stopThread) return;

            restTime = delayTo - Flows::HelperFunctions::getTime();
        }

        Flows::PVariable outputMessage2 = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
        outputMessage2->structValue->emplace("payload", std::make_shared<Flows::Variable>(false));
        output(0, outputMessage2); //false

        setNodeData("delayTo", std::make_shared<Flows::Variable>(0));
        _lastInputState = false; //to start the impulse again, if true is input when interval is complete
        setNodeData("lastInputState", std::make_shared<Flows::Variable>(false));
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

void MyNode::input(const Flows::PNodeInfo info, uint32_t index, const Flows::PVariable message)
{
    try
    {
        Flows::PVariable& input = message->structValue->at("payload");
        if(*input)
        {
            if(!_lastInputState || _allowRetrigger)
            {
                _lastInputState = true;
                setNodeData("lastInputState", std::make_shared<Flows::Variable>(true));
                int64_t delayTo = _delay + Flows::HelperFunctions::getTime();
                _delayTo.store(delayTo, std::memory_order_release);
                setNodeData("delayTo", std::make_shared<Flows::Variable>(delayTo));
                std::lock_guard<std::mutex> timerGuard(_timerThreadMutex);
                _stopThread = true;
                if(_timerThread.joinable())_timerThread.join();
                if(_stopped) return;
                _stopThread = false;
                _timerThread = std::thread(&MyNode::timer, this, true);
            }
        }
        else
        {
            _lastInputState = false;
            setNodeData("lastInputState", std::make_shared<Flows::Variable>(false));
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

