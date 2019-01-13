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
#include <homegear-node/JsonDecoder.h>
#include "mustache.h"

using namespace kainjow;

namespace MyNode
{

class Template : public Flows::INode
{
public:
	Template(std::string path, std::string nodeNamespace, std::string type, const std::atomic_bool* frontendConnected);
	virtual ~Template();

	virtual bool init(Flows::PNodeInfo info);
private:
	Flows::PNodeInfo _nodeInfo;

	std::string _plainTemplate;
	Flows::JsonDecoder _jsonDecoder;
	std::unique_ptr<mustache::mustache> _template;
	std::string _field;
	bool _mustache = true;
	bool _parseJson = false;
	std::mutex _inputMutex;
	mustache::data _data;

	void addData(bool global, std::string key);
	void setData(mustache::data& data, std::string key, Flows::PVariable value);
	virtual void input(const Flows::PNodeInfo info, uint32_t index, const Flows::PVariable message);
};

}

#endif
