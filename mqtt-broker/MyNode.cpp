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

MyNode::MyNode(std::string path, std::string name, const std::atomic_bool* nodeEventsEnabled) : Flows::INode(path, name, nodeEventsEnabled)
{
	_localRpcMethods.emplace("publish", std::bind(&MyNode::publish, this, std::placeholders::_1));
	_localRpcMethods.emplace("registerTopic", std::bind(&MyNode::registerTopic, this, std::placeholders::_1));
	_localRpcMethods.emplace("unregisterTopic", std::bind(&MyNode::unregisterTopic, this, std::placeholders::_1));
}

MyNode::~MyNode()
{
}

bool MyNode::start(Flows::PNodeInfo info)
{
	try
	{
		std::shared_ptr<Mqtt::MqttSettings> mqttSettings = std::make_shared<Mqtt::MqttSettings>();
		//Todo: Set settings

		std::shared_ptr<BaseLib::SharedObjects> bl = std::make_shared<BaseLib::SharedObjects>();
		_mqtt.reset(new Mqtt(bl));
		_mqtt->setInvoke(std::function<Flows::PVariable(std::string, std::string, Flows::PArray&)>(std::bind(&MyNode::invokeNodeMethod, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)));

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

void MyNode::stop()
{
	try
	{
		if(_mqtt) _mqtt->stop();
		_mqtt.reset();
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

//{{{ RPC methods
Flows::PVariable MyNode::publish(Flows::PArray& parameters)
{
	try
	{
		if(parameters->size() != 3) return BaseLib::Variable::createError(-1, "Method expects exactly two parameters. " + std::to_string(parameters->size()) + " given.");
		if(parameters->at(0)->type != BaseLib::VariableType::tString) return BaseLib::Variable::createError(-1, "Parameter 1 is not of type string.");
		if(parameters->at(1)->type != BaseLib::VariableType::tString) return BaseLib::Variable::createError(-1, "Parameter 2 is not of type string.");
		if(parameters->at(2)->type != BaseLib::VariableType::tBoolean) return BaseLib::Variable::createError(-1, "Parameter 3 is not of type boolean.");

		if(_mqtt) _mqtt->queueMessage(parameters->at(0)->stringValue, parameters->at(1)->stringValue, parameters->at(2)->booleanValue);

		return std::make_shared<BaseLib::Variable>();
	}
	catch(const std::exception& ex)
	{
		Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
	return Flows::Variable::createError(-32500, "Unknown application error.");
}

Flows::PVariable MyNode::registerTopic(Flows::PArray& parameters)
{
	try
	{
		if(parameters->size() != 2) return BaseLib::Variable::createError(-1, "Method expects exactly two parameters. " + std::to_string(parameters->size()) + " given.");
		if(parameters->at(0)->type != BaseLib::VariableType::tString) return BaseLib::Variable::createError(-1, "Parameter 1 is not of type string.");
		if(parameters->at(1)->type != BaseLib::VariableType::tString) return BaseLib::Variable::createError(-1, "Parameter 2 is not of type string.");

		if(_mqtt) _mqtt->registerTopic(parameters->at(0)->stringValue, parameters->at(1)->stringValue);

		return std::make_shared<BaseLib::Variable>();
	}
	catch(const std::exception& ex)
	{
		Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
	return Flows::Variable::createError(-32500, "Unknown application error.");
}

Flows::PVariable MyNode::unregisterTopic(Flows::PArray& parameters)
{
	try
	{
		if(parameters->size() != 2) return BaseLib::Variable::createError(-1, "Method expects exactly two parameters. " + std::to_string(parameters->size()) + " given.");
		if(parameters->at(0)->type != BaseLib::VariableType::tString) return BaseLib::Variable::createError(-1, "Parameter 1 is not of type string.");
		if(parameters->at(1)->type != BaseLib::VariableType::tString) return BaseLib::Variable::createError(-1, "Parameter 2 is not of type string.");

		if(_mqtt) _mqtt->unregisterTopic(parameters->at(0)->stringValue, parameters->at(1)->stringValue);

		return std::make_shared<BaseLib::Variable>();
	}
	catch(const std::exception& ex)
	{
		Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	catch(...)
	{
		Flows::Output::printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
	}
	return Flows::Variable::createError(-32500, "Unknown application error.");
}
//}}}

}
