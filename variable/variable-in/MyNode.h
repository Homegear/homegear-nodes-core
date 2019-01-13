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

namespace MyNode
{

class MyNode: public Flows::INode
{
public:
	MyNode(std::string path, std::string nodeNamespace, std::string type, const std::atomic_bool* frontendConnected);
	virtual ~MyNode();

	virtual bool init(Flows::PNodeInfo info);
	virtual void startUpComplete();

private:
	enum class VariableType
    {
        device,
        metadata,
        system,
        flow,
        global
    };

    enum class EventSource
    {
        all,
        device,
        homegear
    };

    VariableType _variableType = VariableType::device;
	int64_t _lastInput = 0;
	uint32_t _refractionPeriod = 0;
	bool _outputOnStartup = false;
	uint64_t _peerId = 0;
	int32_t _channel = -1;
	std::string _variable;
	EventSource _eventSource = EventSource::all;
	std::string _name;

	Flows::VariableType _type = Flows::VariableType::tVoid;
	std::string _loopPreventionGroup;
	bool _loopPrevention = false;

	virtual void variableEvent(std::string source, uint64_t peerId, int32_t channel, std::string variable, Flows::PVariable value);
    virtual void flowVariableEvent(std::string flowId, std::string variable, Flows::PVariable value);
    virtual void globalVariableEvent(std::string variable, Flows::PVariable value);
};

}

#endif
