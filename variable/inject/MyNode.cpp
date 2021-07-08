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

#include "MyNode.h"
namespace MyNode {
MyNode::MyNode(const std::string &path, const std::string &type, const std::atomic_bool *frontendConnected)
    : Flows::INode(path, type, frontendConnected) {
}

MyNode::~MyNode() {
  _stopThread = true;
}

bool MyNode::init(const Flows::PNodeInfo &info) {
  try {
    //TODO
    Flows::PArray messages;
    auto settingsIterator = info->info->structValue->find("props");
    if (settingsIterator != info->info->structValue->end()) {
      messages = settingsIterator->second->arrayValue;
      _messages.clear();
      if (messages) {
        _messages.reserve(messages->size());
      }
    }

    settingsIterator = info->info->structValue->find("repeat");
    if (settingsIterator != info->info->structValue->end()) {
      if (!settingsIterator->second->stringValue.empty()){
        _mode = Interval;
        _sleepingTime = settingsIterator->second->integerValue;
      }
    }

    settingsIterator = info->info->structValue->find("crontab");
    if (settingsIterator != info->info->structValue->end()) {
      if (!settingsIterator->second->stringValue.empty()){
        std::string crontab = settingsIterator->second->stringValue;
        if (crontab.substr(0, 2).compare("*/") == 0){
          _mode = Interval_Time;
          _sleepingTime = Flows::Math::getNumber(crontab.substr(2, 1)) * 60;

          size_t found = crontab.find("*", 1);
          if (found != std::string::npos){
            std::string timeInterval = crontab.substr(4, found - 4);
            found = timeInterval.find("-");
            if (found != std::string::npos){
              _intervalStart = Flows::Math::getNumber(timeInterval.substr(0, found));
              _intervalStop = Flows::Math::getNumber(timeInterval.substr(found + 1)) + 1;
            } else {
              _intervalStart = Flows::Math::getNumber(timeInterval);
              _intervalStop = _intervalStart + 1;
            }
          }
        } else {
          _mode = Time;
          std::string time = crontab.substr(3, 2) + "." + crontab.substr(0, 2);
          _time = Flows::Math::getDouble(time);
        }

        size_t found = crontab.find("* *");
        if (found != std::string::npos){
          std::string days = crontab.substr(found + 4);
          if (days.compare("*") == 0){
            for (auto &d : _days) {
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
    getTime();
    settingsIterator = info->info->structValue->find("once");
    if (settingsIterator != info->info->structValue->end()) {
      _once = settingsIterator->second->booleanValue;
    }

    settingsIterator = info->info->structValue->find("onceDelay");
    if (settingsIterator != info->info->structValue->end()) {
      _onceDelay = settingsIterator->second->floatValue * 1000;
    }

    settingsIterator = info->info->structValue->find("topic");
    if (settingsIterator != info->info->structValue->end()) {
      
    }

    settingsIterator = info->info->structValue->find("payload");
    if (settingsIterator != info->info->structValue->end()) {

    }

    settingsIterator = info->info->structValue->find("payloadType");
    if (settingsIterator != info->info->structValue->end()) {

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

bool MyNode::start() {
  try {
    std::lock_guard<std::mutex> workerGuard(_workerThreadMutex);
    _stopThread = true;
    if (_workerThread.joinable()){
      _workerThread.join();
    }

    _stopThread = false;
    _workerThread = std::thread(&MyNode::sleep, this);

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

void MyNode::stop() {
  std::lock_guard<std::mutex> workerGuard(_workerThreadMutex);
  _stopThread = true;
}

void MyNode::waitForStop() {
  try {
    std::lock_guard<std::mutex> workerGuard(_workerThreadMutex);
    _stopThread = true;
    if (_workerThread.joinable()){
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

void MyNode::sleep() {
  if (_mode == None){
    if (_once){
      std::this_thread::sleep_for(std::chrono::milliseconds(_onceDelay));
      sendMessage();
      _once = false;
    }
  } else {
    while(!_stopThread){
      switch (_mode) {
        case None:
          break;
        case Interval:{
          if (_once) {
            std::this_thread::sleep_for(std::chrono::milliseconds(_onceDelay));
            sendMessage();
            _once = false;
          } else {
            std::this_thread::sleep_for(std::chrono::seconds(_sleepingTime));
            sendMessage();
          }
          break;
        }
        case Interval_Time:{
          if (_once) {
            std::this_thread::sleep_for(std::chrono::seconds(_onceDelay));
            sendMessage();
            _once = false;
          } else {

          }
          break;
        }
        case Time:{

          break;
        }
      }
    }
  }
}

void MyNode::sendMessage() {
  return;
}

void MyNode::getTime() {
  std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  _out->printInfo(std::ctime(&time));
}



}

