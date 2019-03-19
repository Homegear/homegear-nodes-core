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

        settingsIterator = info->info->structValue->find("process-false");
        if(settingsIterator != info->info->structValue->end()) _switchOffOnInFalse = settingsIterator->second->booleanValue;

        settingsIterator = info->info->structValue->find("keep-on");
        if(settingsIterator != info->info->structValue->end()) _keepOn = settingsIterator->second->booleanValue;

        settingsIterator = info->info->structValue->find("refraction-time");
        if(settingsIterator != info->info->structValue->end()) _refractionTime = Flows::Math::getUnsignedNumber(settingsIterator->second->stringValue);

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

        auto manuallyEnabled = getNodeData("manuallyEnabled");
        if(manuallyEnabled->type == Flows::VariableType::tBoolean) _manuallyEnabled.store(manuallyEnabled->booleanValue, std::memory_order_release);

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

        auto stateValue = getNodeData("stateValue");
        if(stateValue->type == Flows::VariableType::tInteger64 && stateValue->integerValue64 > 0)
        {
            _booleanStateValue.store(false, std::memory_order_release);
            _stateValue.store(stateValue->integerValue64, std::memory_order_release);
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
        _lastLightEvent.store(BaseLib::HelperFunctions::getTime(), std::memory_order_release);

        Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
        outputMessage->structValue->emplace("payload", getLightStateVariable());
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

                if(onTo != -1)
                {
                    if(onTo <= time)
                    {
                        auto alwaysOnTo = _alwaysOnTo.load(std::memory_order_acquire);
                        if(alwaysOnTo == -1 || (alwaysOnTo != 0 && (time >= alwaysOnTo)))
                        {
                            _lastLightEvent.store(BaseLib::HelperFunctions::getTime(), std::memory_order_release);

                            Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
                            Flows::PVariable state;
                            if(_booleanStateValue.load(std::memory_order_acquire)) state = std::make_shared<Flows::Variable>(false);
                            else state = std::make_shared<Flows::Variable>(0);
                            outputMessage->structValue->emplace("payload", state);
                            output(0, outputMessage);

                            Flows::PVariable outputMessage2 = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
                            outputMessage2->structValue->emplace("payload", std::make_shared<Flows::Variable>(0));
                            output(2, outputMessage2);
                        }
                        onTo = -1;
                        _onTo.store(-1, std::memory_order_release);
                        setNodeData("onTo", std::make_shared<Flows::Variable>(-1));

                        _manuallyEnabled.store(false, std::memory_order_release);
                        _manuallyDisabled.store(false, std::memory_order_release);
                        setNodeData("manuallyEnabled", std::make_shared<Flows::Variable>(false));
                    }
                    else if(getLightState())
                    {
                        Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
                        outputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>((int64_t)std::lround((onTo - time) / 1000.0)));
                        output(2, outputMessage);
                    }
                }

                if(alwaysOnTo != -1 && alwaysOnTo != 0)
                {
                    if(alwaysOnTo <= time)
                    {
                        _lastLightEvent.store(BaseLib::HelperFunctions::getTime(), std::memory_order_release);

                        Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
                        bool lightState = onTo > time && (_enabled.load(std::memory_order_acquire) || _manuallyEnabled.load(std::memory_order_acquire)) && !_manuallyDisabled.load(std::memory_order_acquire);
                        outputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>(_booleanStateValue.load(std::memory_order_acquire) ? lightState : (lightState ? _stateValue.load(std::memory_order_acquire) : 0)));
                        output(0, outputMessage);

                        Flows::PVariable outputMessage2 = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
                        outputMessage2->structValue->emplace("payload", std::make_shared<Flows::Variable>(true));
                        output(1, outputMessage2);

                        Flows::PVariable outputMessage3 = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
                        outputMessage3->structValue->emplace("payload", std::make_shared<Flows::Variable>(0));
                        output(3, outputMessage3);

                        alwaysOnTo = -1;
                        _alwaysOnTo.store(-1, std::memory_order_release);
                        setNodeData("alwaysOnTo", std::make_shared<Flows::Variable>(-1));
                    }
                    else
                    {
                        Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
                        outputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>((int64_t)std::lround((alwaysOnTo - time) / 1000.0)));
                        output(3, outputMessage);
                    }
                }

                if(alwaysOffTo != -1 && alwaysOffTo != 0)
                {
                    if(alwaysOffTo <= time)
                    {
                        _lastLightEvent.store(BaseLib::HelperFunctions::getTime(), std::memory_order_release);

                        Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
                        bool lightState = onTo > time && (_enabled.load(std::memory_order_acquire) || _manuallyEnabled.load(std::memory_order_acquire)) && !_manuallyDisabled.load(std::memory_order_acquire);
                        outputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>(_booleanStateValue.load(std::memory_order_acquire) ? lightState : (lightState ? _stateValue.load(std::memory_order_acquire) : 0)));
                        output(0, outputMessage);

                        Flows::PVariable outputMessage2 = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
                        outputMessage2->structValue->emplace("payload", std::make_shared<Flows::Variable>(true));
                        output(1, outputMessage2);

                        Flows::PVariable outputMessage3 = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
                        outputMessage3->structValue->emplace("payload", std::make_shared<Flows::Variable>(0));
                        output(4, outputMessage3);

                        alwaysOffTo = -1;
                        _alwaysOffTo.store(-1, std::memory_order_release);
                        setNodeData("alwaysOffTo", std::make_shared<Flows::Variable>(-1));
                    }
                    else
                    {
                        Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
                        outputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>((int64_t)std::lround((alwaysOffTo - time) / 1000.0)));
                        output(4, outputMessage);
                    }
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
    auto onTo = _onTo.load(std::memory_order_acquire);
    auto alwaysOnTo = _alwaysOnTo.load(std::memory_order_acquire);
    auto alwaysOffTo = _alwaysOffTo.load(std::memory_order_acquire);
    return ((_enabled.load(std::memory_order_acquire) || _manuallyEnabled.load(std::memory_order_acquire)) &&
            !_manuallyDisabled.load(std::memory_order_acquire) &&
            onTo != -1 && BaseLib::HelperFunctions::getTime() < onTo &&
            (alwaysOffTo == -1 || (alwaysOffTo != 0 && (BaseLib::HelperFunctions::getTime() >= alwaysOffTo)))) ||
            alwaysOnTo == 0 || (alwaysOnTo != -1 && BaseLib::HelperFunctions::getTime() < alwaysOnTo);
}

Flows::PVariable PresenceLight::getLightStateVariable()
{
    if(_booleanStateValue.load(std::memory_order_acquire))
    {
        return std::make_shared<Flows::Variable>(getLightState());
    }
    else
    {
        return getLightState() ? std::make_shared<Flows::Variable>(_stateValue.load(std::memory_order_acquire)) : std::make_shared<Flows::Variable>(0);
    }
}

void PresenceLight::input(const Flows::PNodeInfo info, uint32_t index, const Flows::PVariable message)
{
    try
    {
        { //Rate limiter
            auto time = BaseLib::HelperFunctions::getTime();
            if(time - _lastInput < _refractionTime)
            {
                int64_t timeToSleep = _refractionTime - (time - _lastInput);
                std::this_thread::sleep_for(std::chrono::milliseconds(timeToSleep));
            }
            _lastInput = BaseLib::HelperFunctions::getTime();
        }

        Flows::PVariable& input = message->structValue->at("payload");
        bool inputValue = *input;

        if(index == 0) //Enabled
        {
            bool enabled = _enabled.load(std::memory_order_acquire);
            if(enabled == inputValue) return;
            if(!inputValue && _keepOn)
            {
                _manuallyEnabled.store(true, std::memory_order_release);
            }
            _enabled.store(inputValue, std::memory_order_release);
            setNodeData("enabled", std::make_shared<Flows::Variable>(inputValue));
        }
        else if(index == 1) //Always on
        {
            if(inputValue)
            {
                auto alwaysOnTime = _alwaysOnTime;
                auto payloadIterator = message->structValue->find("alwaysOnTime");
                if(payloadIterator != message->structValue->end()) alwaysOnTime = payloadIterator->second->integerValue64 * 1000;

                _alwaysOnTo.store(alwaysOnTime == 0 ? 0 : BaseLib::HelperFunctions::getTime() + alwaysOnTime, std::memory_order_release);
                if(_alwaysOffTo.load(std::memory_order_acquire) != -1)
                {
                    _alwaysOffTo.store(-1, std::memory_order_release);
                    Flows::PVariable resetOutputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
                    resetOutputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>(0));
                    output(4, resetOutputMessage);
                }
            }
            else
            {
                auto alwaysOnTo = _alwaysOnTo.load(std::memory_order_acquire);
                if(alwaysOnTo == -1) return;
                _alwaysOnTo.store(-1, std::memory_order_release);

                Flows::PVariable resetOutputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
                resetOutputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>(-1));
                output(2, resetOutputMessage);
            }

            setNodeData("alwaysOnTo", std::make_shared<Flows::Variable>(_alwaysOnTo.load(std::memory_order_acquire)));
            setNodeData("alwaysOffTo", std::make_shared<Flows::Variable>(_alwaysOffTo.load(std::memory_order_acquire)));
        }
        else if(index == 2) //Always off
        {
            if(inputValue)
            {
                auto alwaysOffTime = _alwaysOffTime;
                auto payloadIterator = message->structValue->find("alwaysOffTime");
                if(payloadIterator != message->structValue->end()) alwaysOffTime = payloadIterator->second->integerValue64 * 1000;

                _alwaysOffTo.store(alwaysOffTime == 0 ? 0 : BaseLib::HelperFunctions::getTime() + alwaysOffTime, std::memory_order_release);
                if(_alwaysOnTo.load(std::memory_order_acquire) != -1)
                {
                    _alwaysOnTo.store(-1, std::memory_order_release);
                    Flows::PVariable resetOutputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
                    resetOutputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>(0));
                    output(3, resetOutputMessage);
                }
            }
            else
            {
                auto alwaysOffTo = _alwaysOffTo.load(std::memory_order_acquire);
                if(alwaysOffTo == -1) return;
                _alwaysOffTo.store(-1, std::memory_order_release);

                Flows::PVariable resetOutputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
                resetOutputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>(-1));
                output(3, resetOutputMessage);
            }

            setNodeData("alwaysOnTo", std::make_shared<Flows::Variable>(_alwaysOnTo.load(std::memory_order_acquire)));
            setNodeData("alwaysOffTo", std::make_shared<Flows::Variable>(_alwaysOffTo.load(std::memory_order_acquire)));
        }
        else if(index == 3) //Presence (IN)
        {
            if(inputValue)
            {
                auto onTime = _onTime;
                auto payloadIterator = message->structValue->find("onTime");
                if(payloadIterator != message->structValue->end()) onTime = payloadIterator->second->integerValue64;

                _onTo.store(BaseLib::HelperFunctions::getTime() + onTime, std::memory_order_release);

                setNodeData("onTo", std::make_shared<Flows::Variable>(_onTo.load(std::memory_order_acquire)));
            }
            else if(_switchOffOnInFalse)
            {
                auto onTo = _onTo.load(std::memory_order_acquire);
                if(onTo == -1) return;
                _onTo.store(-1, std::memory_order_release);
            }
            else return;
        }
        else if(index == 4) //Light input (IN2)
        {
            auto lastLightEvent = _lastLightEvent.load(std::memory_order_acquire);
            if(BaseLib::HelperFunctions::getTime() - lastLightEvent < 10000) return;
            _manuallyEnabled.store(false, std::memory_order_release);
            _manuallyDisabled.store(false, std::memory_order_release);
            if(inputValue)
            {
                auto onTime = _onTime;
                auto payloadIterator = message->structValue->find("onTime");
                if(payloadIterator != message->structValue->end()) onTime = payloadIterator->second->integerValue64;

                _onTo.store(BaseLib::HelperFunctions::getTime() + onTime, std::memory_order_release);

                setNodeData("onTo", std::make_shared<Flows::Variable>(_onTo.load(std::memory_order_acquire)));
            }
            else
            {
                auto onTo = _onTo.load(std::memory_order_acquire);
                if(onTo == -1) return;
                _onTo.store(-1, std::memory_order_release);

                setNodeData("onTo", std::make_shared<Flows::Variable>(-1));
            }
        }
        else if(index == 5) //State value
        {
            if(input->type == Flows::VariableType::tBoolean && input->booleanValue)
            {
                _booleanStateValue.store(true, std::memory_order_release);
                _stateValue.store(1, std::memory_order_release);
                setNodeData("stateValue", input);
            }
            else if(input->type == Flows::VariableType::tInteger || input->type == Flows::VariableType::tInteger64)
            {
                _booleanStateValue.store(false, std::memory_order_release);
                _stateValue.store(input->integerValue64, std::memory_order_release);
                setNodeData("stateValue", input);
            }
        }
        else if(index == 6) //Toggle
        {
            if((input->type == Flows::VariableType::tInteger || input->type == Flows::VariableType::tInteger64) && input->integerValue64 > 0)
            {
                _booleanStateValue.store(false, std::memory_order_release);
                _stateValue.store(input->integerValue64, std::memory_order_release);

                setNodeData("stateValue", input);
            }

            if(!_booleanStateValue.load(std::memory_order_acquire) && input->type == Flows::VariableType::tBoolean)
            {
                _out->printWarning(R"(Warning: Got boolean input on "TG", but "SVAL" is set to a light profile (i. e. to an Integer).)");
                return;
            }

            auto onTo = _onTo.load(std::memory_order_acquire);
            if((!_booleanStateValue.load(std::memory_order_acquire) && inputValue) || onTo == -1)
            {
                _manuallyEnabled.store(true, std::memory_order_release);
                _manuallyDisabled.store(false, std::memory_order_release);
                setNodeData("manuallyEnabled", std::make_shared<Flows::Variable>(true));

                auto onTime = _onTime;
                auto payloadIterator = message->structValue->find("onTime");
                if(payloadIterator != message->structValue->end()) onTime = payloadIterator->second->integerValue64;

                _onTo.store(BaseLib::HelperFunctions::getTime() + onTime, std::memory_order_release);

                setNodeData("onTo", std::make_shared<Flows::Variable>(_onTo.load(std::memory_order_acquire)));
            }
            else
            {
                _manuallyEnabled.store(false, std::memory_order_release);
                _manuallyDisabled.store(true, std::memory_order_release);
                _onTo.store(-1, std::memory_order_release);

                setNodeData("manuallyEnabled", std::make_shared<Flows::Variable>(false));
                setNodeData("onTo", std::make_shared<Flows::Variable>(-1));
            }
        }

        _lastLightEvent.store(BaseLib::HelperFunctions::getTime(), std::memory_order_release);

        Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
        outputMessage->structValue->emplace("payload", getLightStateVariable());
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
