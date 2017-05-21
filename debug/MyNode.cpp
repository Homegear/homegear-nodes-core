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

MyNode::MyNode(std::string path, std::string name) : Flows::INode(path, name)
{
}

MyNode::~MyNode()
{
}

int32_t MyNode::getNumber(std::string& s, bool isHex)
{
	int32_t xpos = s.find('x');
	int32_t number = 0;
	if(xpos == -1 && !isHex) try { number = std::stoll(s, 0, 10); } catch(...) {}
	else try { number = std::stoll(s, 0, 16); } catch(...) {}
	return number;
}

int64_t MyNode::getNumber64(std::string& s, bool isHex)
{
	int32_t xpos = s.find('x');
	int64_t number = 0;
	if(xpos == -1 && !isHex) try { number = std::stoll(s, 0, 10); } catch(...) {}
	else try { number = std::stoll(s, 0, 16); } catch(...) {}
	return number;
}

bool MyNode::start(Flows::PNodeInfo info)
{
	try
	{
		auto peerIdIterator = info->info->structValue->find("peerid");
		if(peerIdIterator != info->info->structValue->end()) _peerId = getNumber64(peerIdIterator->second->stringValue);

		auto channelIterator = info->info->structValue->find("channel");
		if(channelIterator != info->info->structValue->end()) _channel = getNumber(channelIterator->second->stringValue);

		auto variableIterator = info->info->structValue->find("variable");
		if(variableIterator != info->info->structValue->end()) _variable = variableIterator->second->stringValue;

		Flows::PArray parameters = std::make_shared<Flows::Array>();
		parameters->reserve(3);
		parameters->push_back(std::make_shared<Flows::Variable>(_peerId));
		parameters->push_back(std::make_shared<Flows::Variable>(_channel));
		parameters->push_back(std::make_shared<Flows::Variable>(_variable));
		Flows::PVariable result = invoke("getValue", parameters);
		if(result->errorStruct) log(2, "Error: Could not get type of variable: (Peer ID: " + std::to_string(_peerId) + ", channel: " + std::to_string(_channel) + ", name: " + _variable + ").");
		else _type = result->type;

		subscribePeer(_peerId, _channel, _variable);

		return true;
	}
	catch(const std::exception& ex)
	{
		log(2, std::string("Error in file ") + __FILE__ + " in line " + std::to_string(__LINE__) + " and function " + __PRETTY_FUNCTION__ + ": " + ex.what());
	}
	catch(...)
	{
		log(2, std::string("Unknown error in file ") + __FILE__ + " in line " + std::to_string(__LINE__) + " and function " + __PRETTY_FUNCTION__ + ".");
	}
	return false;
}

void MyNode::variableEvent(uint64_t peerId, int32_t channel, std::string variable, Flows::PVariable value)
{
	try
	{
		Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
		message->structValue->emplace("peerId", std::make_shared<Flows::Variable>(peerId));
		message->structValue->emplace("channel", std::make_shared<Flows::Variable>(channel));
		message->structValue->emplace("variable", std::make_shared<Flows::Variable>(variable));
		message->structValue->emplace("payload", value);

		output(0, message);
	}
	catch(const std::exception& ex)
	{
		log(2, std::string("Error in file ") + __FILE__ + " in line " + std::to_string(__LINE__) + " and function " + __PRETTY_FUNCTION__ + ": " + ex.what());
	}
	catch(...)
	{
		log(2, std::string("Unknown error in file ") + __FILE__ + " in line " + std::to_string(__LINE__) + " and function " + __PRETTY_FUNCTION__ + ".");
	}
}

}
