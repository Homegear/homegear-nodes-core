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

#include <homegear-node/INode.h>
#include <thread>
#include <mutex>

namespace MyNode
{

class MyNode: public Flows::INode
{
public:
	enum class Units
	{
		ms,
		s,
		m,
		h,
		dom,
		dow,
		doy,
		w,
		M,
		Y
	};

	MyNode(std::string path, std::string nodeNamespace, std::string type, const std::atomic_bool* frontendConnected);
	virtual ~MyNode();

	virtual bool init(Flows::PNodeInfo info);
	virtual bool start();
	virtual void startUpComplete();
	virtual void stop();
	virtual void waitForStop();
private:
	bool _timestamp = true;
	Units _unit = Units::s;
	int32_t _lastWeek = 0;
	int32_t _lastMonth = 0;
	int32_t _lastYear = 0;

	std::mutex _timerMutex;
	std::atomic_bool _stopThread;
	std::thread _timerThread;

	void timer();
	void outputMessage(int64_t time);
	std::pair<int64_t, int64_t> getLocalAndUtcTime(int64_t utcTime = 0);
	void getTimeStruct(std::tm& timeStruct, int64_t utcTime = 0);
};

}

#endif
