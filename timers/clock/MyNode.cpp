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
		auto settingsIterator = info->info->structValue->find("units");
		std::string unit;
		if(settingsIterator != info->info->structValue->end()) unit = settingsIterator->second->stringValue;
		if(unit == "ms") _unit = Units::ms;
		else if(unit == "s") _unit = Units::s;
		else if(unit == "m") _unit = Units::m;
		else if(unit == "h") _unit = Units::h;
		else if(unit == "dom") _unit = Units::dom;
		else if(unit == "dow") _unit = Units::dow;
		else if(unit == "doy") _unit = Units::doy;
		else if(unit == "w") _unit = Units::w;
		else if(unit == "M") _unit = Units::M;
		else if(unit == "Y") _unit = Units::Y;

		settingsIterator = info->info->structValue->find("timestamp");
		if(settingsIterator != info->info->structValue->end()) _timestamp = settingsIterator->second->booleanValue;

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

void MyNode::startUpComplete()
{
	try
	{
		std::lock_guard<std::mutex> timerGuard(_timerMutex);
		_stopThread = false;
		if(_timerThread.joinable()) _timerThread.join();
		_timerThread = std::thread(&MyNode::timer, this);
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

void MyNode::stop()
{
	try
	{
		_stopThread = true;
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

void MyNode::waitForStop()
{
	try
	{
		std::lock_guard<std::mutex> timerGuard(_timerMutex);
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

void MyNode::outputMessage(int64_t time)
{
	try
	{
		Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
		std::tm tm;
		getTimeStruct(tm, time);

		int32_t weekNum = 0;
		{
			int32_t julian = tm.tm_yday;
			int32_t dow = tm.tm_wday;
			int64_t time2 = time - (julian * 86400000);
			std::tm tm2;
			getTimeStruct(tm2, time2);
			weekNum = ((julian + 6) / 7);
			if(dow < tm2.tm_wday) weekNum++;
		}

		message->structValue->emplace("timestamp", std::make_shared<Flows::Variable>(time / 1000));
		message->structValue->emplace("seconds", std::make_shared<Flows::Variable>((time / 1000) % 60));
		message->structValue->emplace("minutes", std::make_shared<Flows::Variable>((time / 60000) % 60));
		message->structValue->emplace("hours", std::make_shared<Flows::Variable>((time / 3600000) % 24));
		message->structValue->emplace("dom", std::make_shared<Flows::Variable>(tm.tm_mday));
		message->structValue->emplace("dow", std::make_shared<Flows::Variable>(tm.tm_wday == 0 ? 7 : tm.tm_wday));
		message->structValue->emplace("doy", std::make_shared<Flows::Variable>(tm.tm_yday + 1));
		message->structValue->emplace("week", std::make_shared<Flows::Variable>(weekNum));
		message->structValue->emplace("month", std::make_shared<Flows::Variable>(tm.tm_mon + 1));
		message->structValue->emplace("year", std::make_shared<Flows::Variable>(tm.tm_year + 1900));

		if(_unit == Units::ms)
		{
			if(_timestamp) message->structValue->emplace("payload", std::make_shared<Flows::Variable>(time));
			else message->structValue->emplace("payload", std::make_shared<Flows::Variable>(time % 1000));
		}
		else if(_unit == Units::s)
		{
			if(_timestamp) message->structValue->emplace("payload", std::make_shared<Flows::Variable>(time / 1000));
			else message->structValue->emplace("payload", std::make_shared<Flows::Variable>((time / 1000) % 60));
		}
		else if(_unit == Units::m)
		{
			if(_timestamp) message->structValue->emplace("payload", std::make_shared<Flows::Variable>(time / 1000));
			else message->structValue->emplace("payload", std::make_shared<Flows::Variable>((time / 60000) % 60));
		}
		else if(_unit == Units::h)
		{
			if(_timestamp) message->structValue->emplace("payload", std::make_shared<Flows::Variable>(time / 1000));
			else message->structValue->emplace("payload", std::make_shared<Flows::Variable>((time / 3600000) % 24));
		}
		else if(_unit == Units::dom)
		{
			message->structValue->emplace("payload", std::make_shared<Flows::Variable>(tm.tm_mday));
		}
		else if(_unit == Units::dow)
		{
			message->structValue->emplace("payload", std::make_shared<Flows::Variable>(tm.tm_wday == 0 ? 7 : tm.tm_wday));
		}
		else if(_unit == Units::doy)
		{
			message->structValue->emplace("payload", std::make_shared<Flows::Variable>(tm.tm_yday + 1));
		}
		else if(_unit == Units::w)
		{
			if(weekNum != _lastWeek)
			{
				_lastWeek = weekNum;
				message->structValue->emplace("payload", std::make_shared<Flows::Variable>(weekNum));
			}
		}
		else if(_unit == Units::M)
		{
			int32_t month = tm.tm_mon + 1;
			if(month != _lastMonth)
			{
				_lastMonth = month;
				message->structValue->emplace("payload", std::make_shared<Flows::Variable>(month));
			}
		}
		else if(_unit == Units::Y)
		{
			int32_t year = tm.tm_year + 1900;
			if(year != _lastYear)
			{
				_lastYear = year;
				message->structValue->emplace("payload", std::make_shared<Flows::Variable>(year));
			}
		}
		output(0, message);
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

std::pair<int64_t, int64_t> MyNode::getLocalAndUtcTime(int64_t utcTime)
{
	std::time_t t;
	if(utcTime > 0)
	{
		t = std::time_t(utcTime / 1000);
	}
	else
	{
		const auto timePoint = std::chrono::system_clock::now();
		t = std::chrono::system_clock::to_time_t(timePoint);
	}
	std::tm localTime;
	localtime_r(&t, &localTime);
	int64_t millisecondOffset = localTime.tm_gmtoff * 1000;
	if(utcTime > 0) return std::make_pair(utcTime + millisecondOffset, utcTime);
	else
	{
		utcTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		return std::make_pair(utcTime + millisecondOffset, utcTime);
	}
}

void MyNode::getTimeStruct(std::tm& timeStruct, int64_t utcTime)
{
	std::time_t t;
	if(utcTime > 0)
	{
		t = std::time_t(utcTime / 1000);
	}
	else
	{
		const auto timePoint = std::chrono::system_clock::now();
		t = std::chrono::system_clock::to_time_t(timePoint);
	}

	localtime_r(&t, &timeStruct);
}

void MyNode::timer()
{
	auto currentTimes = getLocalAndUtcTime();
	int64_t currentLocalTime = currentTimes.first;
	int64_t currentUtcTime = currentTimes.second;
	int64_t sleepTime = 0;
	if(_unit == Units::ms)
	{
		currentLocalTime = (currentLocalTime / 100) * 100;
		currentUtcTime = (currentUtcTime / 100) * 100;
	}
	else
	{
		currentLocalTime = (currentLocalTime / 1000) * 1000;
		currentUtcTime = (currentUtcTime / 1000) * 1000;
	}
	outputMessage(currentLocalTime);
	if(_unit == Units::ms)
	{
		currentLocalTime += 100;
		currentUtcTime += 100;
	}
	else
	{
		currentLocalTime += 1000;
		currentUtcTime += 1000;
	}
	while(!_stopThread)
	{
		try
		{
			currentTimes = getLocalAndUtcTime();
			if(currentLocalTime <= currentTimes.first || currentLocalTime >= currentTimes.first + 5000)
			{
				if(_unit == Units::ms)
				{
					currentLocalTime = (currentTimes.first / 100) * 100 + 100;
					currentUtcTime = (currentTimes.second / 100) * 100 + 100;
				}
				else
				{
					currentLocalTime = (currentTimes.first / 1000) * 1000 + 1000;
					currentUtcTime = (currentTimes.second / 1000) * 1000 + 1000;
				}
				outputMessage(currentLocalTime);
			}
			std::tm timeStruct;
			getTimeStruct(timeStruct);
			sleepTime = currentUtcTime - currentTimes.second;
			if(sleepTime < 10) sleepTime = 10;
			else
			{
				if(_unit == Units::ms && sleepTime > 100) sleepTime = 100;
				else if(sleepTime > 1000) sleepTime = 1000;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
			currentLocalTime += (_unit == Units::ms) ? 100 : 1000;
			currentUtcTime += (_unit == Units::ms) ? 100 : 1000;
			if(_stopThread) break;
			switch(_unit)
			{
			case Units::ms:
				break;
			case Units::s:
				break;
			case Units::m:
				if(currentLocalTime % 60000 != 0) continue;
				break;
			case Units::h:
				if(currentLocalTime % 3600000 != 0) continue;
				break;
			case Units::dom:
			case Units::dow:
			case Units::doy:
			case Units::w:
			case Units::M:
			case Units::Y:
				if(currentLocalTime % 86400000 != 0) continue;
				break;
			}
			outputMessage(currentLocalTime);
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

}
