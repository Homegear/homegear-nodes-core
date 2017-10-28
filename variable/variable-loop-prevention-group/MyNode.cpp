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
	_localRpcMethods.emplace("event", std::bind(&MyNode::event, this, std::placeholders::_1));
	_lastEvent = 0;
}

MyNode::~MyNode()
{
}

bool MyNode::init(Flows::PNodeInfo info)
{
	try
	{
		_nodeInfo = info;

		auto settingsIterator = _nodeInfo->info->structValue->find("refractoryperiod");
		if(settingsIterator != _nodeInfo->info->structValue->end()) _refractoryPeriod = Flows::Math::getNumber(settingsIterator->second->stringValue);

		return true;
	}
	catch(const std::exception& ex)
	{
		_out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		_out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
	return false;
}

bool MyNode::start()
{
	return true;
}

void MyNode::stop()
{
	try
	{

	}
	catch(const std::exception& ex)
	{
		_out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		_out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
}

Flows::PVariable MyNode::getConfigParameterIncoming(std::string name)
{
	try
	{
		auto settingsIterator = _nodeInfo->info->structValue->find(name);
		if(settingsIterator != _nodeInfo->info->structValue->end()) return settingsIterator->second;
	}
	catch(const std::exception& ex)
	{
		_out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		_out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
	return std::make_shared<Flows::Variable>();
}

//{{{ RPC methods
Flows::PVariable MyNode::event(Flows::PArray parameters)
{
	try
	{
		if(parameters->size() != 1) return Flows::Variable::createError(-1, "Method expects exactly one parameter. " + std::to_string(parameters->size()) + " given.");
		if(parameters->at(0)->type != Flows::VariableType::tString) return Flows::Variable::createError(-1, "Parameter 1 is not of type string.");

		std::lock_guard<std::mutex> lastEventGuard(_lastEventMutex);
		if(Flows::HelperFunctions::getTime() - _lastEvent <= _refractoryPeriod && parameters->at(0)->stringValue != _lastEventNode)
		{
			return std::make_shared<Flows::Variable>(false);
		}
		_lastEvent = Flows::HelperFunctions::getTime();
		_lastEventNode = parameters->at(0)->stringValue;
		return std::make_shared<Flows::Variable>(true);
	}
	catch(const std::exception& ex)
	{
		_out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		_out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
	return Flows::Variable::createError(-32500, "Unknown application error.");
}
//}}}

}
