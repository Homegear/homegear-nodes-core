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

#include <homegear-node/JsonDecoder.h>
#include "Xml.h"

namespace Parsers
{

Xml::Xml(std::string path, std::string nodeNamespace, std::string type, const std::atomic_bool* frontendConnected) : Flows::INode(path, nodeNamespace, type, frontendConnected)
{
}

Xml::~Xml()
{
}

bool Xml::init(Flows::PNodeInfo info)
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

void Xml::input(const Flows::PNodeInfo info, uint32_t index, const Flows::PVariable message)
{
	try
	{
		Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);

		if(message->structValue->at("payload")->type == Flows::VariableType::tStruct)
		{
		    auto& payload = message->structValue->at("payload");

		    if(payload->structValue->empty()) return;

		    Flows::PVariable rootVariable;

            xml_document<> doc;
            try
            {
                xml_node<>* rootNode = nullptr;
                if(payload->structValue->size() > 1)
                {
                    rootVariable = payload;
                    rootNode = doc.allocate_node(node_element, "root");
                }
                else
                {
                    rootVariable = payload->structValue->begin()->second;
                    rootNode = doc.allocate_node(node_element, payload->structValue->begin()->first.c_str());
                }

                doc.append_node(rootNode);

                parseVariable(&doc, rootNode, rootVariable);

                std::ostringstream stringStream;
                stringStream << doc;
                outputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>(stringStream.str()));
            }
            catch(const std::exception& ex)
            {
                _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
            }

            doc.clear();

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
}

Flows::PVariable Xml::parseXmlNode(xml_node<>* node)
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
			std::string nodeName(subNode->name());
			if(nodeName.empty()) continue;
            std::string nodeValue(subNode->value());

            hasElements = true;

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
            std::string nodeValue = std::string(node->value());
            Flows::PVariable jsonNodeValue;
            try
            {
                jsonNodeValue = Flows::JsonDecoder::decode(nodeValue);
            }
            catch(const std::exception& ex)
            {
                jsonNodeValue = std::make_shared<Flows::Variable>(nodeValue);
            }

            if(nodeVariable->structValue->empty()) return jsonNodeValue;
            else
            {
                if(!nodeValue.empty()) nodeVariable->structValue->emplace("@@value", jsonNodeValue);
                return nodeVariable;
            }
        }
        else return nodeVariable;
	}
	catch(const std::exception& ex)
	{
		_out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	return Flows::Variable::createError(-32500, "Unknown application error.");
}

void Xml::parseVariable(xml_document<>* doc, xml_node<>* parentNode, const Flows::PVariable& variable)
{
    try
    {
        std::string tempString;

        for(auto& structElement : *variable->structValue)
        {
            if(structElement.first.empty()) continue;
            else if(structElement.first == "@@value") //Value when node has attributes
            {
                tempString = structElement.second->toString();
                parentNode->value(doc->allocate_string(tempString.c_str(), tempString.size() + 1));
            }
            else if(structElement.first.front() == '@' && structElement.first.size() > 1) //Attribute
            {
                tempString = structElement.second->toString();
                auto* attr = doc->allocate_attribute(structElement.first.c_str() + 1, doc->allocate_string(tempString.c_str(), tempString.size() + 1));
                parentNode->append_attribute(attr);
            }
            else //Element
            {
                if(structElement.second->type == Flows::VariableType::tStruct)
                {
                    auto* node = doc->allocate_node(node_element, structElement.first.c_str());
                    parentNode->append_node(node);
                    parseVariable(doc, node, structElement.second);
                }
                else if(structElement.second->type == Flows::VariableType::tArray)
                {
                    auto* node = doc->allocate_node(node_element, "element");
                    parentNode->append_node(node);
                    parseVariable(doc, node, structElement.second);
                }
                else
                {
                    tempString = structElement.second->toString();
                    auto* node = doc->allocate_node(node_element, structElement.first.c_str(), doc->allocate_string(tempString.c_str(), tempString.size() + 1));
                    parentNode->append_node(node);
                }
            }
        }
    }
    catch(const std::exception& ex)
    {
        _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
}

}
