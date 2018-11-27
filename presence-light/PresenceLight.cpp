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
#include "PresenceLight.h"

namespace PresenceLight
{

PresenceLight::PresenceLight(std::string path, std::string nodeNamespace, std::string type, const std::atomic_bool* frontendConnected) : Flows::INode(path, nodeNamespace, type, frontendConnected)
{
}

PresenceLight::~PresenceLight()
{
    _stopThread.store(true, std::memory_order_release);
    waitForStop();
}


bool PresenceLight::init(Flows::PNodeInfo info)
{
    try
    {
        auto settingsIterator = info->info->structValue->find("on-time");
        if(settingsIterator != info->info->structValue->end()) _onTime = Flows::Math::getUnsignedNumber(settingsIterator->second->stringValue) * 1000;

        settingsIterator = info->info->structValue->find("always-on-time");
        if(settingsIterator != info->info->structValue->end()) _alwaysOnTime = Flows::Math::getUnsignedNumber(settingsIterator->second->stringValue) * 1000;

        settingsIterator = info->info->structValue->find("always-off-time");
        if(settingsIterator != info->info->structValue->end()) _alwaysOffTime = Flows::Math::getUnsignedNumber(settingsIterator->second->stringValue) * 1000;

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

bool PresenceLight::start()
{
    try
    {
        _stopped.store(false, std::memory_order_release);

        auto enabled = getNodeData("enabled");
        if(enabled->type == Flows::VariableType::tBoolean) _enabled.store(enabled->booleanValue, std::memory_order_release);

        auto onTo = getNodeData("onTo");
        if(onTo->type == Flows::VariableType::tInteger64) _onTo.store(onTo->integerValue64, std::memory_order_release);

        auto alwaysOnTo = getNodeData("alwaysOnTo");
        if(alwaysOnTo->type == Flows::VariableType::tInteger64)
        {
            if(alwaysOnTo->integerValue64 > 0 && alwaysOnTo->integerValue64 <= BaseLib::HelperFunctions::getTime()) alwaysOnTo->integerValue64 = -1;
            _alwaysOnTo.store(alwaysOnTo->integerValue64, std::memory_order_release);
        }

        auto alwaysOffTo = getNodeData("alwaysOffTo");
        if(alwaysOffTo->type == Flows::VariableType::tInteger64)
        {
            if(alwaysOffTo->integerValue64 > 0 && alwaysOffTo->integerValue64 <= BaseLib::HelperFunctions::getTime()) alwaysOffTo->integerValue64 = -1;
            _alwaysOffTo.store(alwaysOffTo->integerValue64, std::memory_order_release);
        }

        _stopThread.store(false, std::memory_order_release);
        _timerThread = std::thread(&PresenceLight::timer, this);

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

void PresenceLight::startUpComplete()
{
    try
    {
        Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
        outputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>(getLightState()));
        output(0, outputMessage);
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

void PresenceLight::stop()
{
    try
    {
        _stopped.store(true, std::memory_order_release);
        std::lock_guard<std::mutex> timerGuard(_timerThreadMutex);
        _stopThread.store(true, std::memory_order_release);
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

void PresenceLight::waitForStop()
{
    try
    {
        std::lock_guard<std::mutex> timerGuard(_timerThreadMutex);
        _stopThread.store(true, std::memory_order_release);
        if (_timerThread.joinable()) _timerThread.join();
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

void PresenceLight::timer()
{
    try
    {
        int64_t time = 0;
        int64_t onTo = 0;
        int64_t alwaysOnTo = 0;
        int64_t alwaysOffTo = 0;

        while(!_stopThread.load(std::memory_order_acquire))
        {
            try
            {
                time = BaseLib::HelperFunctions::getTime();
                onTo = _onTo.load(std::memory_order_acquire);
                alwaysOnTo = _alwaysOnTo.load(std::memory_order_acquire);
                alwaysOffTo = _alwaysOffTo.load(std::memory_order_acquire);

                if(onTo != -1 && onTo <= time)
                {
                    auto alwaysOnTo = _alwaysOnTo.load(std::memory_order_acquire);
                    if(alwaysOnTo == -1 || (alwaysOnTo != 0 && (time >= alwaysOnTo)))
                    {
                        Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
                        outputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>(false));
                        output(0, outputMessage);
                    }
                    onTo = -1;
                    _onTo.store(-1, std::memory_order_release);
                    setNodeData("onTo", std::make_shared<Flows::Variable>(-1));
                }

                if(alwaysOnTo != -1 && alwaysOnTo != 0 && alwaysOnTo <= time)
                {
                    Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
                    outputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>(onTo > time && _enabled.load(std::memory_order_acquire)));
                    output(0, outputMessage);

                    Flows::PVariable outputMessage2 = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
                    outputMessage2->structValue->emplace("payload", std::make_shared<Flows::Variable>(true));
                    output(1, outputMessage2);

                    alwaysOnTo = -1;
                    _alwaysOnTo.store(-1, std::memory_order_release);
                    setNodeData("alwaysOnTo", std::make_shared<Flows::Variable>(-1));
                }

                if(alwaysOffTo != -1 && alwaysOffTo != 0 && alwaysOffTo <= time)
                {
                    Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
                    outputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>(onTo > time && _enabled.load(std::memory_order_acquire)));
                    output(0, outputMessage);

                    Flows::PVariable outputMessage2 = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
                    outputMessage2->structValue->emplace("payload", std::make_shared<Flows::Variable>(true));
                    output(1, outputMessage2);

                    alwaysOffTo = -1;
                    _alwaysOffTo.store(-1, std::memory_order_release);
                    setNodeData("alwaysOffTo", std::make_shared<Flows::Variable>(-1));
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
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
    catch(const std::exception& ex)
    {
        _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
        _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
}

bool PresenceLight::getLightState()
{
    auto alwaysOnTo = _alwaysOnTo.load(std::memory_order_acquire);
    auto alwaysOffTo = _alwaysOffTo.load(std::memory_order_acquire);
    return (_enabled.load(std::memory_order_acquire) && (alwaysOffTo == -1 || (alwaysOffTo != 0 && (BaseLib::HelperFunctions::getTime() >= alwaysOffTo)))) || alwaysOnTo == 0 || (alwaysOnTo != -1 && BaseLib::HelperFunctions::getTime() >= alwaysOnTo);
}

void PresenceLight::input(const Flows::PNodeInfo info, uint32_t index, const Flows::PVariable message)
{
    try
    {
        Flows::PVariable& input = message->structValue->at("payload");

        if(index == 0) //Enabled
        {
            bool enabled = *input;
            _enabled.store(enabled, std::memory_order_release);
            setNodeData("enabled", std::make_shared<Flows::Variable>(enabled));
        }
        else if(index == 1) //Always on
        {
            if(*input)
            {
                _alwaysOnTo.store(_alwaysOnTime == 0 ? 0 : BaseLib::HelperFunctions::getTime() + _alwaysOnTime, std::memory_order_release);
                _alwaysOffTo.store(-1, std::memory_order_release);
            }
            else
            {
                _alwaysOnTo.store(-1, std::memory_order_release);
            }

            setNodeData("alwaysOnTo", std::make_shared<Flows::Variable>(_alwaysOnTo.load(std::memory_order_acquire)));
            setNodeData("alwaysOffTo", std::make_shared<Flows::Variable>(_alwaysOffTo.load(std::memory_order_acquire)));
        }
        else if(index == 2) //Always off
        {
            if(*input)
            {
                _alwaysOffTo.store(_alwaysOffTime == 0 ? 0 : BaseLib::HelperFunctions::getTime() + _alwaysOffTime, std::memory_order_release);
                _alwaysOnTo.store(-1, std::memory_order_release);
            }
            else
            {
                _alwaysOffTo.store(-1, std::memory_order_release);
            }

            setNodeData("alwaysOnTo", std::make_shared<Flows::Variable>(_alwaysOnTo.load(std::memory_order_acquire)));
            setNodeData("alwaysOffTo", std::make_shared<Flows::Variable>(_alwaysOffTo.load(std::memory_order_acquire)));
        }
        else if(index == 3 && *input) //Presence
        {
            _onTo.store(BaseLib::HelperFunctions::getTime() + _onTime, std::memory_order_release);

            setNodeData("onTo", std::make_shared<Flows::Variable>(_alwaysOnTo.load(std::memory_order_acquire)));
        }

        Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
        outputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>(getLightState()));
        output(0, outputMessage);
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
