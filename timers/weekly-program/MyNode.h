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

#include "SunTime.h"
#include <homegear-node/INode.h>
#include <thread>
#include <mutex>
#include <homegear-base/Variable.h>

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
    struct TimeEntry
    {
        int64_t time;
        Flows::PVariable value;
    };

    std::atomic_bool _enabled;
    bool _outputOnStartUp = false;
    std::array<std::map<int64_t, TimeEntry>, 7> _program;

    std::mutex _timerMutex;
    std::atomic_bool _stopThread;
    std::atomic_bool _stopped;
    std::atomic_bool _forceUpdate;
    std::thread _timerThread;

    Flows::PVariable _currentValue;

    std::vector<std::string> splitAll(std::string string, char delimiter);
    void timer();
    std::string getDateString(int64_t time);

    int64_t getTimestampFromString(const std::string& time);
    Flows::PVariable getCurrentValue();
    int64_t getNext();
    void printNext(int64_t timestamp);
    virtual void input(const Flows::PNodeInfo info, uint32_t index, const Flows::PVariable message);
};

}

#endif
