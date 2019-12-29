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

#ifndef RUNSCRIPT_H_
#define RUNSCRIPT_H_

#include <homegear-node/INode.h>
#include <homegear-base/BaseLib.h>
#include <mutex>

namespace Exec
{

class Exec : public Flows::INode
{
public:
	Exec(std::string path, std::string nodeNamespace, std::string type, const std::atomic_bool* frontendConnected);
	virtual ~Exec();

	virtual bool init(Flows::PNodeInfo info);
	virtual bool start();
	virtual void stop();
	virtual void waitForStop();
private:
    int32_t _callbackHandlerId = -1;
	std::string _program;
    std::string _arguments;
    std::atomic_bool _autostart{false};
    bool _collectOutput = false;
	std::thread _execThread;
	std::thread _errorThread;
	std::mutex _bufferMutex;
	std::string _bufferOut;
    std::string _bufferErr;
	std::atomic_int _pid{-1};
	std::atomic_int _stdIn{-1};
    std::atomic_int _stdOut{-1};
    std::atomic_int _stdErr{-1};

	virtual void input(const Flows::PNodeInfo info, uint32_t index, const Flows::PVariable message);
	void startProgram();
	int32_t getMaxFd();
    void sigchildHandler(pid_t pid, int exitCode, int signal, bool coreDumped);
	void execThread();
	void errorThread();
	int32_t read(std::atomic_int& fd, uint8_t* buffer, int32_t bufferSize);
};

}

#endif
