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

#include "Light.h"

#include <cmath>

namespace Light {

Light::Light(const std::string &path, const std::string &type, const std::atomic_bool *frontendConnected) : Flows::INode(path, type, frontendConnected) {
}

Light::~Light() = default;

bool Light::init(const Flows::PNodeInfo &info) {
  try {
    std::string variableType = "device";
    auto settingsIterator = info->info->structValue->find("variabletype");
    if (settingsIterator != info->info->structValue->end()) variableType = settingsIterator->second->stringValue;

    if (variableType == "device" || variableType == "metadata") {
      settingsIterator = info->info->structValue->find("peerid");
      if (settingsIterator != info->info->structValue->end()) _peerId = Flows::Math::getNumber64(settingsIterator->second->stringValue);
    }

    if (variableType == "device") {
      settingsIterator = info->info->structValue->find("channel");
      if (settingsIterator != info->info->structValue->end()) _channel = Flows::Math::getNumber(settingsIterator->second->stringValue);
    }

    settingsIterator = info->info->structValue->find("variable");
    if (settingsIterator != info->info->structValue->end()) _variable = settingsIterator->second->stringValue;

    settingsIterator = info->info->structValue->find("lighttype");
    if (settingsIterator != info->info->structValue->end()) {
      std::string lightType = settingsIterator->second->stringValue;
      if (lightType == "dimmerstate") _lightType = LightType::dimmerState;
      else if (lightType == "dimmer") _lightType = LightType::dimmer;
    }

    settingsIterator = info->info->structValue->find("twoinputs");
    if (settingsIterator != info->info->structValue->end()) _twoInputs = settingsIterator->second->booleanValue;

    settingsIterator = info->info->structValue->find("onmaxvalue");
    if (settingsIterator != info->info->structValue->end()) _onMaxValue = settingsIterator->second->booleanValue;

    if (_lightType == LightType::dimmerState) {
      std::string offValue = "0";
      settingsIterator = info->info->structValue->find("offvalue");
      if (settingsIterator != info->info->structValue->end()) offValue = settingsIterator->second->stringValue;

      std::string onValue = "100";
      settingsIterator = info->info->structValue->find("onvalue");
      if (settingsIterator != info->info->structValue->end()) onValue = settingsIterator->second->stringValue;

      if (onValue.find('.') != std::string::npos) _onValue = std::make_shared<Flows::Variable>(Flows::Math::getDouble(onValue));
      else _onValue = std::make_shared<Flows::Variable>(Flows::Math::getNumber(onValue));

      if (offValue.find('.') != std::string::npos) _offValue = std::make_shared<Flows::Variable>(Flows::Math::getDouble(offValue));
      else _offValue = std::make_shared<Flows::Variable>(Flows::Math::getNumber(offValue));
    } else if (_lightType == LightType::dimmer) {
      std::string offValue = "0";
      settingsIterator = info->info->structValue->find("offvalue2");
      if (settingsIterator != info->info->structValue->end()) offValue = settingsIterator->second->stringValue;

      std::string minValue = "15";
      settingsIterator = info->info->structValue->find("minvalue");
      if (settingsIterator != info->info->structValue->end()) minValue = settingsIterator->second->stringValue;

      std::string maxValue = "0";
      settingsIterator = info->info->structValue->find("maxvalue");
      if (settingsIterator != info->info->structValue->end()) maxValue = settingsIterator->second->stringValue;

      settingsIterator = info->info->structValue->find("step");
      if (settingsIterator != info->info->structValue->end()) _step = Flows::Math::getDouble(settingsIterator->second->stringValue);
      if (_step <= 0.0) _step = 0.001;

      settingsIterator = info->info->structValue->find("factor");
      if (settingsIterator != info->info->structValue->end()) _factor = Flows::Math::getDouble(settingsIterator->second->stringValue);
      if (_factor < 0) _factor = 0;

      settingsIterator = info->info->structValue->find("interval");
      if (settingsIterator != info->info->structValue->end()) _interval = Flows::Math::getDouble(settingsIterator->second->stringValue);
      if (_interval < 10) _interval = 10;

      if (offValue.find('.') != std::string::npos) _offValue = std::make_shared<Flows::Variable>(Flows::Math::getDouble(offValue));
      else _offValue = std::make_shared<Flows::Variable>(Flows::Math::getNumber(offValue));

      if (minValue.find('.') != std::string::npos) _minValue = std::make_shared<Flows::Variable>(Flows::Math::getDouble(minValue));
      else {
        _minValue = std::make_shared<Flows::Variable>(Flows::Math::getNumber(minValue));
        _minValue->floatValue = _minValue->integerValue;
      }

      if (maxValue.find('.') != std::string::npos) _maxValue = std::make_shared<Flows::Variable>(Flows::Math::getDouble(maxValue));
      else {
        _maxValue = std::make_shared<Flows::Variable>(Flows::Math::getNumber(maxValue));
        _maxValue->floatValue = _maxValue->integerValue;
      }

      if (!_onMaxValue) _onValue = getNodeData("currentvalue");
      if (!_onValue || !(*_onValue)) _onValue = _maxValue;
    } else {
      _onValue = std::make_shared<Flows::Variable>(true);
      _offValue = std::make_shared<Flows::Variable>(false);
    }

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

void Light::stop() {
  _stopThread = true;
}

void Light::waitForStop() {
  try {
    std::lock_guard<std::mutex> timerGuard(_timerMutex);
    if (_timerThread.joinable()) _timerThread.join();
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void Light::dim(bool up) {
  try {
    Flows::PArray parameters = std::make_shared<Flows::Array>();
    parameters->reserve(4);
    parameters->push_back(std::make_shared<Flows::Variable>(_peerId));
    parameters->push_back(std::make_shared<Flows::Variable>(_channel));
    parameters->push_back(std::make_shared<Flows::Variable>(_variable));
    Flows::PVariable startValue = invoke("getValue", parameters);
    if (startValue->errorStruct) {
      _out->printWarning("For the light node to work correctly the selected device variable should be readable.");
      std::lock_guard<std::mutex> currentValueGuard(_currentValueMutex);
      if (_currentValue) startValue = _currentValue;
    }

    double currentValue = NAN;
    if (startValue->type == Flows::VariableType::tInteger) currentValue = startValue->integerValue;
    else if (startValue->type == Flows::VariableType::tInteger64) currentValue = startValue->integerValue64;
    else {
      startValue->type = Flows::VariableType::tFloat;
      currentValue = startValue->floatValue;
    }

    parameters->push_back(startValue);

    while (!_stopThread) {
      if (up) currentValue += (_step + (_factor * currentValue));
      else currentValue -= (_step + (_factor * currentValue));

      if (currentValue > _maxValue->floatValue) currentValue = _maxValue->floatValue;
      else if (currentValue < _minValue->floatValue) currentValue = _minValue->floatValue;

      if (startValue->type == Flows::VariableType::tInteger) parameters->at(3)->integerValue = std::lround(currentValue);
      else if (startValue->type == Flows::VariableType::tInteger64) parameters->at(3)->integerValue64 = std::lround(currentValue);
      else if (startValue->type == Flows::VariableType::tFloat) parameters->at(3)->floatValue = currentValue;
      invoke("setValue", parameters);
      std::this_thread::sleep_for(std::chrono::milliseconds(_interval));
      if ((up && currentValue == _maxValue->floatValue) || (!up && currentValue == _minValue->floatValue)) break;
    }

    auto currentValue2 = std::make_shared<Flows::Variable>(startValue->type);
    if (startValue->type == Flows::VariableType::tInteger) currentValue2->integerValue = std::lround(currentValue);
    else if (startValue->type == Flows::VariableType::tInteger64) currentValue2->integerValue64 = std::lround(currentValue);
    else currentValue2->floatValue = currentValue;

    {
      std::lock_guard<std::mutex> currentValueGuard(_currentValueMutex);
      _currentValue = currentValue2;
    }

    if (currentValue >= _minValue->floatValue) {
      setNodeData("currentvalue", currentValue2);
      if (!_onMaxValue) _onValue = currentValue2;
    }
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Light::input(const Flows::PNodeInfo &info, uint32_t index, const Flows::PVariable &message) {
  try {
    bool value = *(message->structValue->at("payload"));

    if (_lightType == LightType::dimmer && (index == 1 || index == 2)) {
      std::lock_guard<std::mutex> timerGuard(_timerMutex);
      _stopThread = true;
      if (value) {
        if (_timerThread.joinable()) _timerThread.join();
        _stopThread = false;
        _timerThread = std::thread(&Light::dim, this, index == 1);
      } else {
        if (_timerThread.joinable()) _timerThread.join();
      }
    } else if (index == 3) {
      {
        std::lock_guard<std::mutex> currentValueGuard(_currentValueMutex);
        if (!_currentValue) _currentValue = std::make_shared<Flows::Variable>();
        *_currentValue = *(message->structValue->at("payload"));
        setNodeData("currentvalue", _currentValue);
      }

      {
        std::lock_guard<std::mutex> currentValueGuard(_onValueMutex);
        if (!_onValue) _onValue = std::make_shared<Flows::Variable>();
        if (_currentValue->integerValue64 > 0) *_onValue = *_currentValue;
      }
    } else {
      Flows::PArray parameters = std::make_shared<Flows::Array>();
      parameters->reserve(4);
      parameters->push_back(std::make_shared<Flows::Variable>(_peerId));
      parameters->push_back(std::make_shared<Flows::Variable>(_channel));
      parameters->push_back(std::make_shared<Flows::Variable>(_variable));

      if (_twoInputs && !value) return;
      bool state = _twoInputs ? index == 0 : value;
      {
        std::lock_guard<std::mutex> currentValueGuard(_onValueMutex);
        parameters->push_back(state ? _onValue : _offValue);
      }

      Flows::PVariable result = invoke("setValue", parameters);
      if (result->errorStruct) _out->printError("Error setting variable (Peer ID: " + std::to_string(_peerId) + ", channel: " + std::to_string(_channel) + ", name: " + _variable + "): " + result->structValue->at("faultString")->stringValue);
    }
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

}
