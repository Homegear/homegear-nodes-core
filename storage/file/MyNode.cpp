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
}

MyNode::~MyNode()
{
}

bool MyNode::init(Flows::PNodeInfo info)
{
	try
	{
		auto settingsIterator = info->info->structValue->find("filename");
		if(settingsIterator != info->info->structValue->end()) _filename = settingsIterator->second->stringValue;

		settingsIterator = info->info->structValue->find("appendNewline");
		if(settingsIterator != info->info->structValue->end()) _appendNewLine = settingsIterator->second->booleanValue;

		settingsIterator = info->info->structValue->find("createDir");
		if(settingsIterator != info->info->structValue->end()) _createDirectory = settingsIterator->second->booleanValue;

		settingsIterator = info->info->structValue->find("overwriteFile");
		if(settingsIterator != info->info->structValue->end()) _operation = settingsIterator->second->stringValue;

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

void MyNode::input(Flows::PNodeInfo info, uint32_t index, Flows::PVariable message)
{
	try
	{
		std::string filename = _filename;
		if(filename.empty())
		{
			auto messageIterator = message->structValue->find("filename");
			if(messageIterator != message->structValue->end()) filename = messageIterator->second->stringValue;

			if(filename.empty())
			{
				Flows::Output::printError("Error: filename is not set.");
				return;
			}
		}
		std::string directory = BaseLib::HelperFunctions::splitLast(filename, '/').first + '/';

		if(_operation == "delete" || _operation == "overwrite")
		{
			BaseLib::Io::deleteFile(filename);
			if(_operation == "delete") return;
		}

		if(!BaseLib::Io::directoryExists(directory))
		{
			if(_createDirectory) BaseLib::Io::createDirectory(directory, S_IRWXU | S_IRWXG);
			if(!BaseLib::Io::directoryExists(directory))
			{
				Flows::Output::printError("Error: Directory " + directory + " doesn't exist. If \"create directory\" is set, it couldn't be created.");
				return;
			}
		}

		if(_appendNewLine)
		{
			if(message->type == Flows::VariableType::tBinary) message->structValue->at("payload")->binaryValue.push_back('\n');
			else message->structValue->at("payload")->stringValue.push_back('\n');
		}
		if(_operation == "overwrite")
		{
			if(message->type == Flows::VariableType::tBinary) BaseLib::Io::writeFile(filename, message->structValue->at("payload")->binaryValue, message->structValue->at("payload")->binaryValue.size());
			else BaseLib::Io::writeFile(filename, message->structValue->at("payload")->stringValue);
		}
		else
		{
			if(message->type == Flows::VariableType::tBinary) BaseLib::Io::appendToFile(filename, message->structValue->at("payload")->binaryValue, message->structValue->at("payload")->binaryValue.size());
			else BaseLib::Io::appendToFile(filename, message->structValue->at("payload")->stringValue);
		}
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

}
