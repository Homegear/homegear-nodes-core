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
	MyNode(const std::string &path, const std::string &type, const std::atomic_bool* frontendConnected);
	~MyNode() override;

	bool init(const Flows::PNodeInfo &info) override;
	bool start() override;
	void stop() override;
	void waitForStop() override;
private:
	uint32_t _timeout = 300;
	uint32_t _outputs = 2;
	std::atomic<int64_t> _inputTime{0};
	std::atomic_bool _firstPress{true};
	std::atomic_int _counter{0};
	std::atomic_bool _longPress{false};
	std::atomic_bool _state{false};

	std::atomic_bool _stopThread{true};
	std::mutex _timerThreadMutex;
	std::thread _timerThread;

	void timer();
	void input(const Flows::PNodeInfo &info, uint32_t index, const Flows::PVariable &message) override;
};

}

#endif
