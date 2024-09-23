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
#include <homegear-base/Variable.h>
#include "PresenceLight.h"

namespace PresenceLight {

PresenceLight::PresenceLight(const std::string &path, const std::string &type, const std::atomic_bool *frontendConnected) : Flows::INode(path, type, frontendConnected) {
}

PresenceLight::~PresenceLight() {
  _stopThread.store(true, std::memory_order_release);
}

bool PresenceLight::init(const Flows::PNodeInfo &info) {
  try {
    auto settingsIterator = info->info->structValue->find("on-time");
    if (settingsIterator != info->info->structValue->end()) _onTime = Flows::Math::getUnsignedNumber(settingsIterator->second->stringValue) * 1000;
    if (_onTime < 0) _onTime = 0;

    settingsIterator = info->info->structValue->find("always-on-time");
    if (settingsIterator != info->info->structValue->end()) _alwaysOnTime = Flows::Math::getUnsignedNumber(settingsIterator->second->stringValue) * 1000;
    if (_alwaysOnTime < 0) _alwaysOnTime = 0;

    settingsIterator = info->info->structValue->find("always-off-time");
    if (settingsIterator != info->info->structValue->end()) _alwaysOffTime = Flows::Math::getUnsignedNumber(settingsIterator->second->stringValue) * 1000;
    if (_alwaysOffTime < 0) _alwaysOffTime = 0;

    settingsIterator = info->info->structValue->find("process-false");
    if (settingsIterator != info->info->structValue->end()) _switchOffOnInFalse = settingsIterator->second->booleanValue;

    settingsIterator = info->info->structValue->find("keep-on");
    if (settingsIterator != info->info->structValue->end()) _keepOn = settingsIterator->second->booleanValue;

    settingsIterator = info->info->structValue->find("toggle-profile-0-only");
    if (settingsIterator != info->info->structValue->end()) _toggleProfile0Only = settingsIterator->second->booleanValue;

    settingsIterator = info->info->structValue->find("restore-profile");
    if (settingsIterator != info->info->structValue->end()) _restoreProfile = settingsIterator->second->booleanValue;

    settingsIterator = info->info->structValue->find("restore-profile2");
    if (settingsIterator != info->info->structValue->end()) _restoreProfile2 = settingsIterator->second->booleanValue;

    settingsIterator = info->info->structValue->find("restore-hour");
    if (settingsIterator != info->info->structValue->end()) _restoreHour = Flows::Math::getUnsignedNumber(settingsIterator->second->stringValue);

    settingsIterator = info->info->structValue->find("refraction-time");
    if (settingsIterator != info->info->structValue->end()) _refractionTime = Flows::Math::getUnsignedNumber(settingsIterator->second->stringValue);

    settingsIterator = info->info->structValue->find("changes-only");
    if (settingsIterator != info->info->structValue->end()) _outputChangesOnly = settingsIterator->second->booleanValue;

    _lastInput = BaseLib::HelperFunctions::getTime();

    return true;
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
  return false;
}

bool PresenceLight::start() {
  try {
    _stopped.store(false, std::memory_order_release);

    auto enabled = getNodeData("enabled");
    if (enabled->type == Flows::VariableType::tBoolean) _enabled.store(enabled->booleanValue, std::memory_order_release);

    auto manuallyEnabled = getNodeData("manuallyEnabled");
    if (manuallyEnabled->type == Flows::VariableType::tBoolean) _manuallyEnabled.store(manuallyEnabled->booleanValue, std::memory_order_release);

    auto onTo = getNodeData("onTo");
    if (onTo->type == Flows::VariableType::tInteger || onTo->type == Flows::VariableType::tInteger64) _onTo.store(onTo->integerValue64, std::memory_order_release);

    auto alwaysOnTo = getNodeData("alwaysOnTo");
    if (alwaysOnTo->type == Flows::VariableType::tInteger || alwaysOnTo->type == Flows::VariableType::tInteger64) {
      if (alwaysOnTo->integerValue64 > 0 && alwaysOnTo->integerValue64 <= BaseLib::HelperFunctions::getTime()) alwaysOnTo->integerValue64 = -1;
      _alwaysOnTo.store(alwaysOnTo->integerValue64, std::memory_order_release);
    }

    auto alwaysOffTo = getNodeData("alwaysOffTo");
    if (alwaysOffTo->type == Flows::VariableType::tInteger || alwaysOffTo->type == Flows::VariableType::tInteger64) {
      if (alwaysOffTo->integerValue64 > 0 && alwaysOffTo->integerValue64 <= BaseLib::HelperFunctions::getTime()) alwaysOffTo->integerValue64 = -1;
      _alwaysOffTo.store(alwaysOffTo->integerValue64, std::memory_order_release);
    }

    auto stateValue = getNodeData("stateValue");
    if (stateValue->type == Flows::VariableType::tInteger || stateValue->type == Flows::VariableType::tInteger64) {
      _booleanStateValue.store(false, std::memory_order_release);
      _stateValue.store(stateValue->integerValue64, std::memory_order_release);
    }

    auto lastNonNullStateValue = getNodeData("lastNonNullStateValue");
    if (lastNonNullStateValue->type == Flows::VariableType::tInteger || lastNonNullStateValue->type == Flows::VariableType::tInteger64) {
      _lastNonNullStateValue.store(lastNonNullStateValue->integerValue64, std::memory_order_release);
    }

    auto lastRestore = getNodeData("lastRestore");
    if (lastRestore->type == Flows::VariableType::tInteger || lastRestore->type == Flows::VariableType::tInteger64) {
      _lastRestore.store(lastRestore->integerValue64, std::memory_order_release);
    }

    _stopThread.store(false, std::memory_order_release);
    _timerThread = std::thread(&PresenceLight::timer, this);

    return true;
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
  return false;
}

void PresenceLight::startUpComplete() {
  try {
    stateOutput(getLightStateVariable());
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void PresenceLight::stop() {
  try {
    _stopped.store(true, std::memory_order_release);
    std::lock_guard<std::mutex> timerGuard(_timerThreadMutex);
    _stopThread.store(true, std::memory_order_release);
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void PresenceLight::waitForStop() {
  try {
    std::lock_guard<std::mutex> timerGuard(_timerThreadMutex);
    _stopThread.store(true, std::memory_order_release);
    if (_timerThread.joinable()) _timerThread.join();
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void PresenceLight::timer() {
  try {
    int64_t time = 0;
    int64_t onTo = 0;
    int64_t alwaysOnTo = 0;
    int64_t alwaysOffTo = 0;
    int64_t lastTimeOutput = 0;
    int64_t lastAlwaysOnTimeOutput = 0;
    int64_t lastAlwaysOffTimeOutput = 0;

    while (!_stopThread.load(std::memory_order_acquire)) {
      try {
        time = BaseLib::HelperFunctions::getTime();
        onTo = _onTo.load(std::memory_order_acquire);
        alwaysOnTo = _alwaysOnTo.load(std::memory_order_acquire);
        alwaysOffTo = _alwaysOffTo.load(std::memory_order_acquire);

        if (_lastRestore.load(std::memory_order_acquire) > 0) {
          if ((time % 86400000) < (_restoreHour * 3600000)) {
            _lastRestore.store(-1, std::memory_order_release);
          }
        }

        if (onTo != -1) {
          if (onTo <= time) {
            if (alwaysOnTo == -1 || (alwaysOnTo != 0 && (time >= alwaysOnTo))) {
              Flows::PVariable state;
              if (_booleanStateValue.load(std::memory_order_acquire)) state = std::make_shared<Flows::Variable>(false);
              else state = std::make_shared<Flows::Variable>(0);
              stateOutput(state);

              Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
              outputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>(0));
              output(2, outputMessage);
            }
            onTo = -1;
            _onTo.store(-1, std::memory_order_release);
            setNodeData("onTo", std::make_shared<Flows::Variable>(-1));

            if (_manuallyEnabled.load(std::memory_order_acquire)) {
              _manuallyEnabled.store(false, std::memory_order_release);
              setNodeData("manuallyEnabled", std::make_shared<Flows::Variable>(false));
            }
            if (_manuallyDisabled.load(std::memory_order_acquire)) {
              _manuallyDisabled.store(false, std::memory_order_release);
              setNodeData("manuallyDisabled", std::make_shared<Flows::Variable>(false));

              if (_restoreProfile.load(std::memory_order_acquire) && !_booleanStateValue.load(std::memory_order_acquire)) {
                auto lastNonNullStateValue = _lastNonNullStateValue.load(std::memory_order_acquire);
                _stateValue.store(lastNonNullStateValue, std::memory_order_release);
                setNodeData("stateValue", std::make_shared<Flows::Variable>(lastNonNullStateValue));
              }
            }
          } else if (getLightState()) {
            if (time - lastTimeOutput >= 1000) {
              lastTimeOutput = time;
              Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
              outputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>((int64_t) std::lround((double) (onTo - time) / 1000.0)));
              output(2, outputMessage);
            }
          }
        }

        if (alwaysOnTo != -1 && alwaysOnTo != 0) {
          if (alwaysOnTo <= time) {
            bool lightState = onTo > time && (_enabled.load(std::memory_order_acquire) || _manuallyEnabled.load(std::memory_order_acquire)) && !_manuallyDisabled.load(std::memory_order_acquire);
            stateOutput(std::make_shared<Flows::Variable>(_booleanStateValue.load(std::memory_order_acquire) ? lightState : (lightState ? _stateValue.load(std::memory_order_acquire) : 0)));

            Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
            outputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>(true));
            output(1, outputMessage);

            Flows::PVariable outputMessage2 = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
            outputMessage2->structValue->emplace("payload", std::make_shared<Flows::Variable>(0));
            output(3, outputMessage2);

            alwaysOnTo = -1;
            _alwaysOnTo.store(-1, std::memory_order_release);
            setNodeData("alwaysOnTo", std::make_shared<Flows::Variable>(-1));
          } else {
            if (time - lastAlwaysOnTimeOutput >= 1000) {
              lastAlwaysOnTimeOutput = time;
              Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
              outputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>((int64_t) std::lround((double) (alwaysOnTo - time) / 1000.0)));
              output(3, outputMessage);
            }
          }
        }

        if (alwaysOffTo != -1 && alwaysOffTo != 0) {
          if (alwaysOffTo <= time) {
            bool lightState = onTo > time && (_enabled.load(std::memory_order_acquire) || _manuallyEnabled.load(std::memory_order_acquire)) && !_manuallyDisabled.load(std::memory_order_acquire);
            stateOutput(std::make_shared<Flows::Variable>(_booleanStateValue.load(std::memory_order_acquire) ? lightState : (lightState ? _stateValue.load(std::memory_order_acquire) : 0)));

            Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
            outputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>(true));
            output(1, outputMessage);

            Flows::PVariable outputMessage2 = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
            outputMessage2->structValue->emplace("payload", std::make_shared<Flows::Variable>(0));
            output(4, outputMessage2);

            alwaysOffTo = -1;
            _alwaysOffTo.store(-1, std::memory_order_release);
            setNodeData("alwaysOffTo", std::make_shared<Flows::Variable>(-1));
          } else {
            if (time - lastAlwaysOffTimeOutput >= 1000) {
              lastAlwaysOffTimeOutput = time;
              Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
              outputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>((int64_t) std::lround((double) (alwaysOffTo - time) / 1000.0)));
              output(4, outputMessage);
            }
          }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }
      catch (const std::exception &ex) {
        _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
      }
      catch (...) {
        _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
      }
    }
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

bool PresenceLight::getLightState() {
  auto onTo = _onTo.load(std::memory_order_acquire);
  auto alwaysOnTo = _alwaysOnTo.load(std::memory_order_acquire);
  auto alwaysOffTo = _alwaysOffTo.load(std::memory_order_acquire);
  return ((_enabled.load(std::memory_order_acquire) || _manuallyEnabled.load(std::memory_order_acquire)) &&
      !_manuallyDisabled.load(std::memory_order_acquire) &&
      onTo != -1 && BaseLib::HelperFunctions::getTime() < onTo &&
      (alwaysOffTo == -1 || (alwaysOffTo != 0 && (BaseLib::HelperFunctions::getTime() >= alwaysOffTo)))) ||
      alwaysOnTo == 0 || (alwaysOnTo != -1 && BaseLib::HelperFunctions::getTime() < alwaysOnTo);
}

Flows::PVariable PresenceLight::getLightStateVariable() {
  if (_booleanStateValue.load(std::memory_order_acquire)) {
    return std::make_shared<Flows::Variable>(getLightState());
  } else {
    return getLightState() ? std::make_shared<Flows::Variable>(_stateValue.load(std::memory_order_acquire)) : std::make_shared<Flows::Variable>(0);
  }
}

void PresenceLight::input(const Flows::PNodeInfo &info, uint32_t index, const Flows::PVariable &message) {
  try {
    { //Rate limiter
      auto time = BaseLib::HelperFunctions::getTime();
      if (time - _lastInputRefractionTime < _refractionTime) {
        int64_t timeToSleep = _refractionTime - (time - _lastInputRefractionTime);
        std::this_thread::sleep_for(std::chrono::milliseconds(timeToSleep));
      }
      _lastInputRefractionTime = BaseLib::HelperFunctions::getTime();
    }

    { //Restore profile at specified hour
      auto time = BaseLib::HelperFunctions::getTime();
      //Is profile restoration enabled? Was the profile already restored today?
      if (_restoreProfile2.load(std::memory_order_acquire) && _lastRestore.load(std::memory_order_acquire) == -1) {
        //Is the "off" profile active? Is it later than the restore hour?
        if (_stateValue.load(std::memory_order_acquire) == 0 && (time % 86400000) >= (_restoreHour * 3600000) && (time % 86400000) < ((_restoreHour + 2) * 3600000)) {
          //Has it been at least 60 minutes after the last movement?
          if (time - _lastInput >= 3600000) {
            _stateValue.store(_lastNonNullStateValue.load(std::memory_order_acquire), std::memory_order_release);
            setNodeData("stateValue", std::make_shared<Flows::Variable>(_stateValue.load(std::memory_order_acquire)));
            _lastRestore.store(time, std::memory_order_release);
            setNodeData("lastRestore", std::make_shared<Flows::Variable>(time));
          }
        }
      }
    }

    Flows::PVariable &input = message->structValue->at("payload");
    bool inputValue = *input;

    if (index == 0) //Enabled
    {
      bool enabled = _enabled.load(std::memory_order_acquire);
      if (enabled == inputValue) return;
      if (!inputValue && _keepOn && _onTo.load(std::memory_order_acquire) != -1) {
        _manuallyEnabled.store(true, std::memory_order_release);
        //To avoid race condition, make sure onTo is still set
        if (_onTo.load(std::memory_order_acquire) == -1) _manuallyEnabled.store(false, std::memory_order_release);
      }
      _enabled.store(inputValue, std::memory_order_release);
      setNodeData("enabled", std::make_shared<Flows::Variable>(inputValue));
    } else if (index == 1) //Always on
    {
      if (inputValue) {
        auto alwaysOnTime = _alwaysOnTime;
        if (alwaysOnTime == 0) {
          auto payloadIterator = message->structValue->find("alwaysOnTime");
          if (payloadIterator != message->structValue->end())
            alwaysOnTime =
                payloadIterator->second->integerValue64
                    * 1000;
        }

        _alwaysOnTo.store(alwaysOnTime == 0 ? 0 : BaseLib::HelperFunctions::getTime() + alwaysOnTime, std::memory_order_release);
        if (_alwaysOffTo.load(std::memory_order_acquire) != -1) {
          _alwaysOffTo.store(-1, std::memory_order_release);
          Flows::PVariable resetOutputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
          resetOutputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>(0));
          output(4, resetOutputMessage);
        }
      } else {
        auto alwaysOnTo = _alwaysOnTo.load(std::memory_order_acquire);
        if (alwaysOnTo == -1) return;
        _alwaysOnTo.store(-1, std::memory_order_release);

        Flows::PVariable resetOutputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
        resetOutputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>(-1));
        output(2, resetOutputMessage);
      }

      setNodeData("alwaysOnTo", std::make_shared<Flows::Variable>(_alwaysOnTo.load(std::memory_order_acquire)));
      setNodeData("alwaysOffTo", std::make_shared<Flows::Variable>(_alwaysOffTo.load(std::memory_order_acquire)));
    } else if (index == 2) //Always off
    {
      if (inputValue) {
        auto alwaysOffTime = _alwaysOffTime;
        if (alwaysOffTime == 0) {
          auto payloadIterator = message->structValue->find("alwaysOffTime");
          if (payloadIterator != message->structValue->end())
            alwaysOffTime =
                payloadIterator->second->integerValue64
                    * 1000;
        }

        _alwaysOffTo.store(alwaysOffTime == 0 ? 0 : BaseLib::HelperFunctions::getTime() + alwaysOffTime, std::memory_order_release);
        if (_alwaysOnTo.load(std::memory_order_acquire) != -1) {
          _alwaysOnTo.store(-1, std::memory_order_release);
          Flows::PVariable resetOutputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
          resetOutputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>(0));
          output(3, resetOutputMessage);
        }
      } else {
        auto alwaysOffTo = _alwaysOffTo.load(std::memory_order_acquire);
        if (alwaysOffTo == -1) return;
        _alwaysOffTo.store(-1, std::memory_order_release);

        Flows::PVariable resetOutputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
        resetOutputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>(-1));
        output(3, resetOutputMessage);
      }

      setNodeData("alwaysOnTo", std::make_shared<Flows::Variable>(_alwaysOnTo.load(std::memory_order_acquire)));
      setNodeData("alwaysOffTo", std::make_shared<Flows::Variable>(_alwaysOffTo.load(std::memory_order_acquire)));
    } else if (index == 3) //Presence (IN)
    {
      if (inputValue) {
        _lastInput = BaseLib::HelperFunctions::getTime();
        auto onTime = _onTime;
        if (onTime == 0) {
          auto payloadIterator = message->structValue->find("onTime");
          if (payloadIterator != message->structValue->end())
            onTime = payloadIterator->second->integerValue64
                * 1000;
        }
        if (onTime <= 0) {
          _out->printError("Error: Invalid value for onTime.");
          return;
        }

        _onTo.store(BaseLib::HelperFunctions::getTime() + onTime, std::memory_order_release);

        setNodeData("onTo", std::make_shared<Flows::Variable>(_onTo.load(std::memory_order_acquire)));
      } else if (_switchOffOnInFalse) {
        auto onTo = _onTo.load(std::memory_order_acquire);
        if (onTo == -1) return;
        _onTo.store(-1, std::memory_order_release);
      } else return;
    } else if (index == 4) //Light input (IN2)
    {
      _manuallyEnabled.store(false, std::memory_order_release);
      _manuallyDisabled.store(false, std::memory_order_release);
      if (inputValue) {
        auto onTime = _onTime;
        if (onTime == 0) {
          auto payloadIterator = message->structValue->find("onTime");
          if (payloadIterator != message->structValue->end()) onTime = payloadIterator->second->integerValue64 * 1000;
        }
        if (onTime <= 0) {
          _out->printError("Error: Invalid value for onTime.");
          return;
        }

        auto onTo = _onTo.load(std::memory_order_acquire);
        _onTo.store(BaseLib::HelperFunctions::getTime() + onTime, std::memory_order_release);

        setNodeData("onTo", std::make_shared<Flows::Variable>(_onTo.load(std::memory_order_acquire)));

        if (getLightState() && onTo - BaseLib::HelperFunctions::getTime() > 1000) return;
      } else {
        auto onTo = _onTo.load(std::memory_order_acquire);
        if (onTo == -1) return;
        _onTo.store(-1, std::memory_order_release);

        setNodeData("onTo", std::make_shared<Flows::Variable>(-1));
      }
    } else if (index == 5) //State value
    {
      if (input->type == Flows::VariableType::tBoolean && input->booleanValue) {
        _booleanStateValue.store(true, std::memory_order_release);
        _stateValue.store(1, std::memory_order_release);
        setNodeData("stateValue", input);
        if (input->booleanValue) {
          _lastNonNullStateValue.store(input->booleanValue, std::memory_order_release);
          setNodeData("lastNonNullStateValue", input);
        }
      } else if (input->type == Flows::VariableType::tInteger || input->type == Flows::VariableType::tInteger64) {
        _booleanStateValue.store(false, std::memory_order_release);
        _stateValue.store(input->integerValue64, std::memory_order_release);
        setNodeData("stateValue", input);
        if (input->integerValue64 > 0) {
          _lastNonNullStateValue.store(input->integerValue64, std::memory_order_release);
          setNodeData("lastNonNullStateValue", input);
        }
      }
    } else if (index == 6) //Toggle
    {
      bool booleanStateValue = _booleanStateValue.load(std::memory_order_acquire);
      int64_t lastStateValue = -1;
      if (input->type == Flows::VariableType::tInteger || input->type == Flows::VariableType::tInteger64) {
        _booleanStateValue.store(false, std::memory_order_release);
        lastStateValue = _stateValue.load(std::memory_order_acquire);
        _stateValue.store(input->integerValue64, std::memory_order_release);
        setNodeData("stateValue", input);

        if (input->integerValue64 > 0) {
          _lastNonNullStateValue.store(input->integerValue64, std::memory_order_release);
          setNodeData("lastNonNullStateValue", input);
        }
      }

      if (!booleanStateValue && input->type == Flows::VariableType::tBoolean) {
        _out->printWarning(R"(Warning: Got boolean input on "TG", but "SVAL" is set to a light profile (i. e. to an Integer).)");
        return;
      }

      if (booleanStateValue && !inputValue) return;

      if (!getLightState() || lastStateValue == 0 || (!booleanStateValue && input->integerValue64 > 0 && ((lastStateValue != -1 && lastStateValue != input->integerValue64) || _toggleProfile0Only))) {
        _stateValue.store(_lastNonNullStateValue.load(std::memory_order_acquire), std::memory_order_release);
        setNodeData("stateValue", std::make_shared<Flows::Variable>(_stateValue.load(std::memory_order_acquire)));

        _manuallyEnabled.store(true, std::memory_order_release);
        _manuallyDisabled.store(false, std::memory_order_release);
        setNodeData("manuallyEnabled", std::make_shared<Flows::Variable>(true));
      } else {
        _manuallyEnabled.store(false, std::memory_order_release);
        _manuallyDisabled.store(true, std::memory_order_release);
        setNodeData("manuallyEnabled", std::make_shared<Flows::Variable>(false));
      }

      //{{{ Always start timer, manual off needs to timeout as well
      auto onTime = _onTime;
      if (onTime == 0) {
        auto payloadIterator = message->structValue->find("onTime");
        if (payloadIterator != message->structValue->end())
          onTime = payloadIterator->second->integerValue64
              * 1000;
      }
      if (onTime <= 0) {
        _out->printError("Error: Invalid value for onTime.");
        return;
      }

      _onTo.store(BaseLib::HelperFunctions::getTime() + onTime, std::memory_order_release);
      setNodeData("onTo", std::make_shared<Flows::Variable>(BaseLib::HelperFunctions::getTime() + onTime));
      //}}}
    }

    stateOutput(getLightStateVariable());
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void PresenceLight::stateOutput(const Flows::PVariable &value) {
  try {
    std::lock_guard<std::mutex> stateOutputGuard(_stateOutputMutex);
    if (_outputChangesOnly && *value == *_lastOutput) return;
    _lastOutput = value;

    Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
    outputMessage->structValue->emplace("payload", value);
    output(0, outputMessage);
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

}
