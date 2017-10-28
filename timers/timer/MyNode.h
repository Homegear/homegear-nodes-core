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

#ifndef MYNODE_H_
#define MYNODE_H_

#include "SunTime.h"
#include <homegear-node/INode.h>
#include <thread>
#include <mutex>

namespace MyNode
{

class MyNode: public Flows::INode
{
public:
	MyNode(std::string path, std::string nodeNamespace, std::string type, const std::atomic_bool* frontendConnected);
	virtual ~MyNode();

	virtual bool init(Flows::PNodeInfo info);
	virtual bool start();
	virtual void startUpComplete();
	virtual void stop();
	virtual void waitForStop();
private:
	SunTime _sunTime;
	std::atomic_bool _enabled;
	bool _outputOnStartUp = false;
	std::string _onTime;
	std::string _onTimeType;
	std::string _offTime;
	std::string _offTimeType;
	int64_t _onOffset = 0;
	int64_t _offOffset = 0;
	int64_t _lastOnTime = 0;
	int64_t _lastOffTime = 0;
	double _latitude = 54.32;
	double _longitude = 10.13;
	std::vector<bool> _days;
	std::vector<bool> _months;

	std::mutex _timerMutex;
	std::atomic_bool _stopThread;
	std::atomic_bool _stopped;
	std::thread _timerThread;

	std::vector<std::string> splitAll(std::string string, char delimiter);
	void timer();
	std::string getDateString(int64_t time);
	int64_t getSunTime(int64_t timeStamp, std::string time);
	int64_t getTime(int64_t currentTime, std::string time, std::string timeType, int64_t offset);
	virtual void input(const Flows::PNodeInfo info, uint32_t index, const Flows::PVariable message);
	std::pair<int64_t, bool> getNext(int64_t currentTime, int64_t onTime, int64_t offTime);
	void printNext(int64_t currentTime, int64_t onTime, int64_t offTime);
};

}

#endif
