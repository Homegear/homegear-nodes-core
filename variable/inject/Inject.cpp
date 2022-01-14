/* Copyright 2013-2021 Homegear GmbH
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

#include "Inject.h"
#include <homegear-node/JsonDecoder.h>

namespace Inject {

Inject::Inject(const std::string &path, const std::string &type, const std::atomic_bool *frontendConnected) : Flows::INode(path, type, frontendConnected) {
}

Inject::~Inject() {
  _stopThread = true;
}

bool Inject::init(const Flows::PNodeInfo &info) {
  try {
    auto settingsIterator = info->info->structValue->find("props");
    if (settingsIterator != info->info->structValue->end()) {
      auto &properties = settingsIterator->second->arrayValue;
      _properties.clear();
      _properties.reserve(properties->size());

      for (auto &element: *properties) {
        Flows::PVariable property = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
        auto iterator = element->structValue->find("p");
        if (iterator != element->structValue->end()) {
          property->structValue->emplace("payload", std::make_shared<Flows::Variable>(iterator->second->stringValue));
          if (iterator->second->stringValue == "payload") {
            auto payloadIt = info->info->structValue->find("payload");
            if (payloadIt != info->info->structValue->end()) {
              property->structValue->emplace("value", payloadIt->second);
            }
            payloadIt = info->info->structValue->find("payloadType");
            if (payloadIt != info->info->structValue->end()) {
              property->structValue->emplace("type", std::make_shared<Flows::Variable>(payloadIt->second->stringValue));
            }
          } else if (iterator->second->stringValue == "topic") {
            auto topicIt = info->info->structValue->find("topic");
            if (topicIt != info->info->structValue->end()) {
              property->structValue->emplace("value", topicIt->second);
            }
          }
        }
        iterator = element->structValue->find("v");
        if (iterator != element->structValue->end()) {
          property->structValue->emplace("value", iterator->second);
        }
        iterator = element->structValue->find("vt");
        if (iterator != element->structValue->end()) {
          property->structValue->emplace("type", std::make_shared<Flows::Variable>(iterator->second->stringValue));
        }
        _properties.emplace_back(property);
      }
    }

    settingsIterator = info->info->structValue->find("repeat");
    if (settingsIterator != info->info->structValue->end()) {
      if (!settingsIterator->second->stringValue.empty()) {
        _mode = Interval;
        _sleepingTime = settingsIterator->second->integerValue;
      }
    }

    settingsIterator = info->info->structValue->find("crontab");
    if (settingsIterator != info->info->structValue->end()) {
      if (!settingsIterator->second->stringValue.empty()) {
        std::string crontab = settingsIterator->second->stringValue;
        if (crontab.substr(0, 2).compare("*/") == 0) {
          _mode = Interval_Time;
          _sleepingTime = Flows::Math::getNumber(crontab.substr(2, 1)) * 60;

          size_t found = crontab.find('*', 1);
          if (found != std::string::npos) {
            std::string timeInterval = crontab.substr(4, found - 4);
            found = timeInterval.find('-');
            if (found != std::string::npos) {
              _interval.start = Flows::Math::getNumber(timeInterval.substr(0, found));
              _interval.stop = Flows::Math::getNumber(timeInterval.substr(found + 1)) + 1;
            } else {
              _interval.start = Flows::Math::getNumber(timeInterval);
              _interval.stop = _interval.start + 1;
            }
          }
        } else {
          _mode = Time;
          _startingTime.hour = Flows::Math::getNumber(crontab.substr(3, 2));
          _startingTime.minute = Flows::Math::getNumber(crontab.substr(0, 2));
        }

        size_t found = crontab.find("* *");
        if (found != std::string::npos) {
          std::string days = crontab.substr(found + 4);
          if (days == "*") {
            for (auto &d: _days) {
              d.second = true;
            }
          } else {
            const char *d = days.c_str();
            while (*d != '\0') {
              switch (*d) {
                case '0':_days.insert_or_assign(0, true);
                  break;
                case '1':_days.insert_or_assign(1, true);
                  break;
                case '2':_days.insert_or_assign(2, true);
                  break;
                case '3':_days.insert_or_assign(3, true);
                  break;
                case '4':_days.insert_or_assign(4, true);
                  break;
                case '5':_days.insert_or_assign(5, true);
                  break;
                case '6':_days.insert_or_assign(6, true);
                  break;
              }
              d++;
            }
          }
        }
      }
    }

    settingsIterator = info->info->structValue->find("once");
    if (settingsIterator != info->info->structValue->end()) {
      _once = settingsIterator->second->booleanValue;
    }

    settingsIterator = info->info->structValue->find("onceDelay");
    if (settingsIterator != info->info->structValue->end()) {
      _onceDelay = settingsIterator->second->floatValue * 1000;
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

bool Inject::start() {
  try {
    std::lock_guard<std::mutex> workerGuard(_workerThreadMutex);
    _stopThread = true;
    if (_workerThread.joinable()) {
      _workerThread.join();
    }

    _stopThread = false;
    _workerThread = std::thread(&Inject::evalMode, this);

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

void Inject::stop() {
  std::lock_guard<std::mutex> workerGuard(_workerThreadMutex);
  _stopThread = true;
}

void Inject::waitForStop() {
  try {
    std::lock_guard<std::mutex> workerGuard(_workerThreadMutex);
    _stopThread = true;
    if (_workerThread.joinable()) {
      _workerThread.join();
    }
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void Inject::setNodeVariable(const std::string &variable, const Flows::PVariable &value) {
  try {
    sendMessage();
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void Inject::evalMode() {
  if (_once) {
    int iterations = _onceDelay / 100;
    for (int i = 0; i < iterations; ++i) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      if (_stopThread) {
        return;
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(_onceDelay % 100));
    sendMessage();
  }
  while (!_stopThread) {
    switch (_mode) {
      case None:_stopThread = true;
        break;
      case Interval:intervalMode();
        break;
      case Interval_Time:intervalTimeMode();
        break;
      case Time:timeMode();
        break;
    }
  }
}

void Inject::intervalMode() {
  int iterations = (_sleepingTime * 1000) / 100;
  for (int i = 0; i < iterations; ++i) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    if (_stopThread) {
      return;
    }
  }
  std::this_thread::sleep_for(std::chrono::milliseconds((_sleepingTime * 1000) % 100));
  sendMessage();
}

void Inject::intervalTimeMode() {
  std::tm *time = getTime();
  auto it = _days.find(time->tm_wday);
  if (it != _days.end()) {
    if (it->second) {
      if (_interval.start <= time->tm_hour) {
        int iterations = (_sleepingTime * 1000) / 100;
        for (int i = 0; i < iterations; ++i) {
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
          if (_stopThread) {
            return;
          }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds((_sleepingTime * 1000) % 100));
        time = getTime();
        if (_interval.stop > time->tm_hour) {
          sendMessage();
        } else {
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
      } else {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }
    } else {
      //don't output on that day
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }
}

void Inject::timeMode() {
  std::tm *time = getTime();
  auto it = _days.find(time->tm_wday);
  if (it != _days.end()) {
    if (it->second) {
      if (_startingTime.hour == time->tm_hour && _startingTime.minute == time->tm_min) {
        sendMessage();
        int iterations = (60 * 1000) / 100;
        for (int i = 0; i < iterations; ++i) {
          std::this_thread::sleep_for(std::chrono::milliseconds(100));
          if (_stopThread) {
            return;
          }
        }
      } else {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }
    } else {
      //don't output that day
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }
}

void Inject::sendMessage() {
  Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
  for (auto &property: _properties) {
    auto propertyIterator = property->structValue->find("type");
    if (propertyIterator == property->structValue->end()) continue;
    std::string type = propertyIterator->second->stringValue;

    std::string propertyName;
    propertyIterator = property->structValue->find("payload");
    if (propertyIterator == property->structValue->end()) continue;
    propertyName = propertyIterator->second->stringValue;

    Flows::PVariable value = std::make_shared<Flows::Variable>();
    propertyIterator = property->structValue->find("value");
    if (propertyIterator != property->structValue->end()) value = propertyIterator->second;

    if (type == "flow") {
      message->structValue->emplace(propertyName, getFlowData(value->stringValue));
    } else if (type == "global") {
      message->structValue->emplace(propertyName, getGlobalData(value->stringValue));
    } else if (type == "str") {
      message->structValue->emplace(propertyName, value);
    } else if (type == "int") {
      message->structValue->emplace(propertyName, std::make_shared<Flows::Variable>(Flows::Math::getNumber64(value->stringValue)));
    } else if (type == "float") {
      message->structValue->emplace(propertyName, std::make_shared<Flows::Variable>(Flows::Math::getDouble(value->stringValue)));
    } else if (type == "bool") {
      message->structValue->emplace(propertyName, std::make_shared<Flows::Variable>(value->booleanValue));
    } else if (type == "json") {
      message->structValue->emplace(propertyName, Flows::JsonDecoder::decode(value->stringValue));
    } else if (type == "bin") {
      auto outputValue = std::make_shared<Flows::Variable>(Flows::VariableType::tBinary);
      auto buffer = Flows::JsonDecoder::decode(value->stringValue);
      if (buffer->type == Flows::VariableType::tArray) {
        outputValue->binaryValue.reserve(buffer->arrayValue->size());
        for (auto &byte: *buffer->arrayValue) {
          outputValue->binaryValue.push_back((uint8_t)byte->integerValue);
        }
      } else {
        outputValue->binaryValue = std::vector<uint8_t>(buffer->stringValue.begin(), buffer->stringValue.end());
      }
      message->structValue->emplace(propertyName, outputValue);
    } else if (type == "date") {
      message->structValue->emplace(propertyName, std::make_shared<Flows::Variable>(Flows::HelperFunctions::getTime()));
    } else {
      _out->printError("Error: Unknown type: " + type);
    }
  }
  output(0, message);
}

std::tm *Inject::getTime() {
  std::time_t time = std::time(nullptr);
  std::tm *now = std::localtime(&time);
  return now;
}

}

