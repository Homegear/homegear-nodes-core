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

#include <homegear-base/Variable.h>
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

void MyNode::input(const Flows::PNodeInfo info, uint32_t index, const Flows::PVariable message)
{
	try
	{
		Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);

		if(message->structValue->at("payload")->type == Flows::VariableType::tStruct)
		{


			output(0, outputMessage);
		}
		else if(message->structValue->at("payload")->type == Flows::VariableType::tString)
		{
			std::string xmlString = message->structValue->at("payload")->stringValue;
			Flows::HelperFunctions::trim(xmlString);
			if(xmlString.empty()) return;

			xml_document<> doc;
			doc.parse<parse_no_entity_translation | parse_validate_closing_tags>((char*)xmlString.c_str());
            Flows::PVariable xml = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
            std::string nodeName(doc.first_node()->name());
            auto element = parseXmlNode(doc.first_node());
            if(!element->errorStruct) xml->structValue->emplace(nodeName, element);
            outputMessage->structValue->emplace("payload", xml);
            output(0, outputMessage);
		}
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

Flows::PVariable MyNode::parseXmlNode(xml_node<>* node)
{
	try
	{
        auto nodeVariable = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);

        bool hasElements = false;
		for(xml_attribute<>* attr = node->first_attribute(); attr; attr = attr->next_attribute())
		{
			std::string attributeName(attr->name());
			std::string attributeValue(attr->value());

            nodeVariable->structValue->emplace("@" + attributeName, std::make_shared<Flows::Variable>(attributeValue));
		}

		for(xml_node<>* subNode = node->first_node(); subNode; subNode = subNode->next_sibling())
		{
            hasElements = true;
			std::string nodeName(subNode->name());
            std::string nodeValue(subNode->value());

            auto variableIterator = nodeVariable->structValue->find(nodeName);
            if(variableIterator == nodeVariable->structValue->end())
            {
                variableIterator = nodeVariable->structValue->emplace(nodeName, std::make_shared<Flows::Variable>(Flows::VariableType::tArray)).first;
                variableIterator->second->arrayValue->reserve(100);
            }

            if(variableIterator->second->arrayValue->size() == variableIterator->second->arrayValue->capacity()) variableIterator->second->arrayValue->reserve(variableIterator->second->arrayValue->size() + 100);

            auto subNodeVariable = parseXmlNode(subNode);
            if(!subNodeVariable->errorStruct) variableIterator->second->arrayValue->emplace_back(subNodeVariable);
		}

        if(!hasElements)
        {
            if(nodeVariable->structValue->empty()) return std::make_shared<Flows::Variable>(std::string(node->value()));
            else
            {
                std::string nodeValue = std::string(node->value());
                if(!nodeValue.empty()) nodeVariable->structValue->emplace("@@value", std::make_shared<Flows::Variable>());
                return nodeVariable;
            }
        }
        else return nodeVariable;
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

}
