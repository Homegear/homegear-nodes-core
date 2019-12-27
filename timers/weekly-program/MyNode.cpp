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

#include "MyNode.h"

namespace MyNode
{

MyNode::MyNode(std::string path, std::string nodeNamespace, std::string type, const std::atomic_bool* frontendConnected) : Flows::INode(path, nodeNamespace, type, frontendConnected)
{
    _stopThread = true;
    _stopped = true;
    _enabled = true;
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
        auto settingsIterator = info->info->structValue->find("startup");
        if(settingsIterator != info->info->structValue->end()) _outputOnStartUp = settingsIterator->second->booleanValue;

        settingsIterator = info->info->structValue->find("program");
        if(settingsIterator != info->info->structValue->end())
        {
            for(int32_t weekday = 0; weekday < (signed)settingsIterator->second->arrayValue->size(); weekday++)
            {
                auto& element = settingsIterator->second->arrayValue->at(weekday);

                for(auto& entry : *element->arrayValue)
                {
                    auto timeIterator = entry->structValue->find("time");
                    if(timeIterator == entry->structValue->end()) continue;

                    auto typeIterator = entry->structValue->find("type");
                    if(typeIterator == entry->structValue->end()) continue;

                    auto valueIterator = entry->structValue->find("value");
                    if(valueIterator == entry->structValue->end()) continue;

                    TimeEntry timeEntry{};
                    timeEntry.time = getTimestampFromString(timeIterator->second->stringValue);
                    timeEntry.value = std::make_shared<Flows::Variable>(typeIterator->second->stringValue, valueIterator->second->stringValue);

                    _program.at(weekday).emplace(timeEntry.time, std::move(timeEntry));
                }
            }
        }

        auto enabled = getNodeData("enabled");
        if(enabled->type == Flows::VariableType::tBoolean) _enabled = enabled->booleanValue;

        return true;
    }
    catch(const std::exception& ex)
    {
        _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return false;
}

bool MyNode::start()
{
    try
    {
        _stopped = false;
        return true;
    }
    catch(const std::exception& ex)
    {
        _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return false;
}

void MyNode::startUpComplete()
{
    try
    {
        std::lock_guard<std::mutex> timerGuard(_timerMutex);
        if(!_enabled) return;
        _stopThread = false;
        if(_timerThread.joinable()) _timerThread.join();
        _timerThread = std::thread(&MyNode::timer, this);
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

void MyNode::stop()
{
    try
    {
        _stopped = true;
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
        std::lock_guard<std::mutex> timerGuard(_timerMutex);
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

std::vector<std::string> MyNode::splitAll(std::string string, char delimiter)
{
    std::vector<std::string> elements;
    std::stringstream stringStream(string);
    std::string element;
    while (std::getline(stringStream, element, delimiter))
    {
        elements.push_back(element);
    }
    if(string.back() == delimiter) elements.push_back(std::string());
    return elements;
}

int64_t MyNode::getTimestampFromString(const std::string& time)
{
    try
    {
        auto timeVector = splitAll(time, ':');
        if(!timeVector.empty())
        {
            int64_t timestamp = Flows::Math::getNumber64(timeVector.at(0)) * 3600000;
            if(timeVector.size() > 1)
            {
                timestamp += Flows::Math::getNumber64(timeVector.at(1)) * 60000;
                if(timeVector.size() > 2) timestamp += Flows::Math::getNumber64(timeVector.at(2)) * 1000;
            }
            return timestamp;
        }
    }
    catch(const std::exception& ex)
    {
        _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return 0;
}

Flows::PVariable MyNode::getCurrentValue()
{
    try
    {
        int day = 0;

        {
            std::tm tm {};
            SunTime::getTimeStruct(tm);
            day = tm.tm_wday;
        }

        int otherDay = day;
        for(int32_t i = 0; i < 7; i++)
        {
            if(_program.at(otherDay).empty())
            {
                otherDay--;
                if(otherDay < 0) otherDay = 6;
                continue;
            }
            if(otherDay != day)
            {
                //Get last entry
                return _program.at(otherDay).rbegin()->second.value;
            }
            else
            {
                auto currentHour = SunTime::getLocalTime() % 86400000;

                //Get first matching entry
                auto& entries = _program.at(day);
                for(auto entryIterator = entries.rbegin(); entryIterator != entries.rend(); entryIterator++)
                {
                    if(currentHour >= entryIterator->first) return entryIterator->second.value;
                }
            }
        }
    }
    catch(const std::exception& ex)
    {
        _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return std::make_shared<Flows::Variable>();
}

std::string MyNode::getDateString(int64_t time)
{
    const char timeFormat[] = "%x";
    std::time_t t;
    if(time > 0)
    {
        t = std::time_t(time / 1000);
    }
    else
    {
        const auto timePoint = std::chrono::system_clock::now();
        t = std::chrono::system_clock::to_time_t(timePoint);
    }
    char timeString[50];
    std::tm localTime {};
    localtime_r(&t, &localTime);
    strftime(&timeString[0], 50, &timeFormat[0], &localTime);
    std::ostringstream timeStream;
    timeStream << timeString;
    return timeStream.str();
}

int64_t MyNode::getNext()
{
    try
    {
        int day = 0;

        {
            std::tm tm {};
            SunTime::getTimeStruct(tm);
            day = tm.tm_wday;
        }

        auto currentDay = SunTime::getLocalTime() / 86400000 * 86400000;
        int otherDay = day;
        for(int32_t i = 0; i < 7; i++)
        {
            if(_program.at(otherDay).empty())
            {
                otherDay++;
                if(otherDay > 6) otherDay = 0;
                continue;
            }
            if(otherDay != day)
            {
                //Get first entry
                return currentDay + (i * 86400000) + _program.at(otherDay).begin()->first;
            }
            else
            {
                auto currentHour = SunTime::getLocalTime() % 86400000;

                //Get first matching entry
                auto& entries = _program.at(day);
                for(auto& entry : entries)
                {
                    if(entry.first >= currentHour) return currentDay + entry.first;
                }

                //No matching entry this day
                otherDay++;
                if(otherDay > 6) otherDay = 0;
                continue;
            }
        }
    }
    catch(const std::exception& ex)
    {
        _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return -1;
}

void MyNode::printNext(int64_t timestamp)
{
    try
    {
        Flows::PVariable status = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
        if(timestamp == -1)
        {
            status->structValue->emplace("text", std::make_shared<Flows::Variable>("Next: Unknown"));
            nodeEvent("statusBottom/" + _id, status);
            return;
        }

        auto currentTime = SunTime::getLocalTime();

        std::ostringstream timeStream;
        if(timestamp > currentTime + 86400000)
        {
            timeStream << getDateString(timestamp);
        }
        else
        {
            timestamp = timestamp / 1000;
            timestamp = timestamp % 86400;
            int32_t hours = timestamp / 3600;
            timestamp = timestamp % 3600;
            int32_t minutes = timestamp / 60;
            int32_t seconds = timestamp % 60;
            timeStream << std::setw(2) << std::setfill('0') << hours << ':' << std::setw(2) << minutes << ':' << std::setw(2) << seconds;
        }

        status->structValue->emplace("text", std::make_shared<Flows::Variable>("Next: " + timeStream.str()));
        nodeEvent("statusBottom/" + _id, status);
    }
    catch(const std::exception& ex)
    {
        _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
}

void MyNode::timer()
{
    bool update = false;
    int64_t currentTime = 0;
    int64_t next = getNext();
    int64_t lastTime = 0;

    if(_outputOnStartUp)
    {
        Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
        message->structValue->emplace("payload", getCurrentValue());
        output(0, message);
    }

    printNext(next);

    while(!_stopThread)
    {
        try
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            if(_stopThread) break;
            currentTime = SunTime::getLocalTime();
            if(currentTime >= next)
            {
                update = true;
                Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
                message->structValue->emplace("payload", getCurrentValue());
                output(0, message);
            }

            if(update || _forceUpdate || currentTime % 3600000 < lastTime % 3600000) //New hour? Recalc in case of time changes or summer/winter time
            {
                update = false;
                _forceUpdate = false;
                next = getNext();
                printNext(next);
            }

            lastTime = currentTime;
        }
        catch(const std::exception& ex)
        {
            _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
        }
    }
}

void MyNode::input(const Flows::PNodeInfo info, uint32_t index, const Flows::PVariable message)
{
    try
    {
        if(index == 0) //Enabled
        {
            _enabled = message->structValue->at("payload")->booleanValue;
            setNodeData("enabled", std::make_shared<Flows::Variable>(_enabled));
            std::lock_guard<std::mutex> timerGuard(_timerMutex);
            if(_enabled)
            {
                if(!_stopThread)
                {
                    _stopThread = true;
                    if(_timerThread.joinable()) _timerThread.join();
                    if(_stopped) return;
                    _stopThread = false;
                    _timerThread = std::thread(&MyNode::timer, this);
                }
            }
            else
            {
                _stopThread = true;
                if(_timerThread.joinable()) _timerThread.join();
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

}
