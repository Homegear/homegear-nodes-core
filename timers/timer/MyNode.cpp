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
	_enabled = true;
}

MyNode::~MyNode()
{
	stop();
}


bool MyNode::init(Flows::PNodeInfo info)
{
	try
	{
		auto settingsIterator = info->info->structValue->find("ontime");
		if(settingsIterator != info->info->structValue->end()) _onTime = settingsIterator->second->stringValue;

		settingsIterator = info->info->structValue->find("ontimetype");
		if(settingsIterator != info->info->structValue->end()) _onTimeType = settingsIterator->second->stringValue;

		settingsIterator = info->info->structValue->find("offtime");
		if(settingsIterator != info->info->structValue->end()) _offTime = settingsIterator->second->stringValue;

		settingsIterator = info->info->structValue->find("offtimetype");
		if(settingsIterator != info->info->structValue->end()) _offTimeType = settingsIterator->second->stringValue;

		settingsIterator = info->info->structValue->find("lat");
		if(settingsIterator != info->info->structValue->end()) _latitude = Flows::Math::getDouble(settingsIterator->second->stringValue);

		settingsIterator = info->info->structValue->find("lon");
		if(settingsIterator != info->info->structValue->end()) _longitude = Flows::Math::getDouble(settingsIterator->second->stringValue);

		auto enabled = getNodeData("enabled");
		if(enabled->type == Flows::VariableType::tBoolean) _enabled = enabled->booleanValue;
		_lastOnTime = getNodeData("lastOnTime")->integerValue64;
		_lastOffTime = getNodeData("lastOffTime")->integerValue64;

		return true;
	}
	catch(const std::exception& ex)
	{
		Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
	return false;
}

bool MyNode::start()
{
	try
	{
		if(!_enabled) return true;
		std::lock_guard<std::mutex> timerGuard(_timerMutex);
		_stopThread = false;
		if(_timerThread.joinable()) _timerThread.join();
		_timerThread = std::thread(&MyNode::timer, this);

		return true;
	}
	catch(const std::exception& ex)
	{
		Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
	return false;
}

void MyNode::stop()
{
	try
	{
		std::lock_guard<std::mutex> timerGuard(_timerMutex);
		_stopThread = true;
		if(_timerThread.joinable()) _timerThread.join();
	}
	catch(const std::exception& ex)
	{
		Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
}

std::pair<std::string, std::string> MyNode::splitFirst(std::string string, char delimiter)
{
	int32_t pos = string.find_first_of(delimiter);
	if(pos == -1) return std::pair<std::string, std::string>(string, "");
	if((unsigned)pos + 1 >= string.size()) return std::pair<std::string, std::string>(string.substr(0, pos), "");
	return std::pair<std::string, std::string>(string.substr(0, pos), string.substr(pos + 1));
}

int64_t MyNode::getTime(std::string time, std::string timeType)
{
	try
	{
		if(timeType == "suntime")
		{
			auto sunTimes = _sunTime.getTimesLocal(_sunTime.getLocalTime(), _latitude, _longitude);
			if(time == "sunrise") return sunTimes.times.at(SunTime::SunTimeTypes::sunrise);
			else if(time == "sunset") return sunTimes.times.at(SunTime::SunTimeTypes::sunset);
			else if(time == "sunriseEnd") return sunTimes.times.at(SunTime::SunTimeTypes::sunriseEnd);
			else if(time == "sunsetStart") return sunTimes.times.at(SunTime::SunTimeTypes::sunsetStart);
			else if(time == "dawn") return sunTimes.times.at(SunTime::SunTimeTypes::dawn);
			else if(time == "dusk") return sunTimes.times.at(SunTime::SunTimeTypes::dusk);
			else if(time == "nauticalDawn") return sunTimes.times.at(SunTime::SunTimeTypes::nauticalDawn);
			else if(time == "nauticalDusk") return sunTimes.times.at(SunTime::SunTimeTypes::nauticalDusk);
			else if(time == "nightEnd") return sunTimes.times.at(SunTime::SunTimeTypes::nightEnd);
			else if(time == "night") return sunTimes.times.at(SunTime::SunTimeTypes::night);
			else if(time == "goldenHourEnd") return sunTimes.times.at(SunTime::SunTimeTypes::goldenHourEnd);
			else if(time == "goldenHour") return sunTimes.times.at(SunTime::SunTimeTypes::goldenHour);
		}
		else
		{
			auto timePair = splitFirst(time, ':');
			return (_sunTime.getLocalTime() / 86400000) * 86400000 + Flows::Math::getNumber64(timePair.first) * 3600000 + Flows::Math::getNumber64(timePair.second) * 60000;
		}
	}
	catch(const std::exception& ex)
	{
		Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
	return 0;
}

void MyNode::timer()
{
	bool event = false;
	int64_t currentTime;
	int64_t onTime = getTime(_onTime, _onTimeType);
	int64_t offTime = getTime(_offTime, _offTimeType);

	Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
	message->structValue->emplace("payload", std::make_shared<Flows::Variable>(true));

	uint32_t i = 0;
	while(!_stopThread)
	{
		try
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			if(i % 15 != 0) continue;
			if(_stopThread) break;
			currentTime = _sunTime.getLocalTime();
			if(_lastOnTime < onTime && currentTime >= onTime)
			{
				event = true;
				_lastOnTime = onTime;
				setNodeData("lastOnTime", std::make_shared<Flows::Variable>(_lastOnTime));
				message->structValue->at("payload")->booleanValue = true;
			}
			else if(_lastOffTime < offTime && currentTime >= offTime)
			{
				event = true;
				_lastOffTime = offTime;
				setNodeData("lastOffTime", std::make_shared<Flows::Variable>(_lastOffTime));
				message->structValue->at("payload")->booleanValue = false;
			}
			if(event)
			{
				event = false;
				onTime = getTime(_onTime, _onTimeType);
				offTime = getTime(_offTime, _offTimeType);
				output(0, message);
			}
			i++;
		}
		catch(const std::exception& ex)
		{
			Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
		}
		catch(...)
		{
			Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
		}
	}
}

void MyNode::input(Flows::PNodeInfo info, uint32_t index, Flows::PVariable message)
{
	try
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
	catch(const std::exception& ex)
	{
		Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
}

}
