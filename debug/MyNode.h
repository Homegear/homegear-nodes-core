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

namespace MyNode
{

class MyNode: public Flows::INode
{
public:
	MyNode(std::string path, std::string name);
	virtual ~MyNode();

	virtual bool start(Flows::PNodeInfo info);
private:
	uint64_t _peerId = 0;
	int32_t _channel = -1;
	std::string _variable;
	Flows::VariableType _type = Flows::VariableType::tVoid;

	int32_t getNumber(std::string& s, bool isHex = false);
	int64_t getNumber64(std::string& s, bool isHex = false);

	virtual void variableEvent(uint64_t peerId, int32_t channel, std::string variable, Flows::PVariable value);
};

}

#endif
