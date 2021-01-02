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

#include "MyNode.h"

namespace MyNode {

Template::Template(const std::string &path, const std::string &type, const std::atomic_bool *frontendConnected) : Flows::INode(path, type, frontendConnected) {
}

Template::~Template() = default;

bool Template::init(const Flows::PNodeInfo &info) {
  try {
    _nodeInfo = info;

    auto settingsIterator = info->info->structValue->find("template");
    if (settingsIterator != info->info->structValue->end()) _plainTemplate = settingsIterator->second->stringValue;
    _template.reset(new kainjow::mustache::mustache(_plainTemplate));

    settingsIterator = info->info->structValue->find("syntax");
    if (settingsIterator != info->info->structValue->end()) _mustache = settingsIterator->second->stringValue == "mustache";

    settingsIterator = info->info->structValue->find("output");
    if (settingsIterator != info->info->structValue->end()) _parseJson = settingsIterator->second->stringValue == "json";

    settingsIterator = info->info->structValue->find("field");
    if (settingsIterator != info->info->structValue->end()) _field = settingsIterator->second->stringValue;

    return true;
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
  return false;
}

std::pair<std::string, std::string> Template::splitFirst(const std::string& string, char delimiter) {
  int32_t pos = string.find_first_of(delimiter);
  if (pos == -1) return std::pair<std::string, std::string>(string, "");
  if ((unsigned)pos + 1 >= string.size()) return std::pair<std::string, std::string>(string.substr(0, pos), "");
  return std::pair<std::string, std::string>(string.substr(0, pos), string.substr(pos + 1));
}

void Template::addData(mustache::DataSource dataSource, const std::string &key) {
  try {
    Flows::PVariable result;
    if (dataSource == mustache::DataSource::environment) {
      auto envIterator = _nodeInfo->info->structValue->find("env");
      if (envIterator == _nodeInfo->info->structValue->end()) return;
      auto envIterator2 = envIterator->second->structValue->find(key);
      if (envIterator2 == envIterator->second->structValue->end()) return;
      result = envIterator2->second;
    } else if (dataSource == mustache::DataSource::variable) {
      auto pair1 = splitFirst(key, '_');
      auto pair2 = splitFirst(pair1.second, '_');
      auto peerId = Flows::Math::getUnsignedNumber64(pair1.first);
      auto channel = Flows::Math::getNumber(pair2.first);
      auto &variable =  pair2.second;
      auto parameters = std::make_shared<Flows::Array>();
      parameters->reserve(3);
      parameters->push_back(std::make_shared<Flows::Variable>(peerId));
      parameters->push_back(std::make_shared<Flows::Variable>(channel));
      parameters->push_back(std::make_shared<Flows::Variable>(variable));
      result = invoke("getValue", parameters);
      if (result->errorStruct) return;
    } else {
      Flows::PArray parameters = std::make_shared<Flows::Array>();
      parameters->reserve(2);
      parameters->push_back(std::make_shared<Flows::Variable>(dataSource == mustache::DataSource::global ? "global" : _nodeInfo->info->structValue->at("z")->stringValue));
      parameters->push_back(std::make_shared<Flows::Variable>(key));
      result = invoke("getNodeData", parameters);
      if (result->errorStruct) return;
    }

    mustache::data *data = nullptr;
    //The conversion from const to non const is dirty, but works as there is no write access to data, when addData is called.
    if (dataSource == mustache::DataSource::global) data = (mustache::data *)_data.get("global");
    else if(dataSource == mustache::DataSource::flow) data = (mustache::data *)_data.get("flow");
    else if(dataSource == mustache::DataSource::environment) data = (mustache::data *)_data.get("env");
    else if(dataSource == mustache::DataSource::variable) data = (mustache::data *)_data.get("variable");
    if (!data) return;
    setData(*data, key, result);
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void Template::setData(mustache::data &data, const std::string& key, const Flows::PVariable& value) {
  try {
    if (value->type == Flows::VariableType::tArray) {
      mustache::data element = mustache::data::type::list;
      for (const auto& arrayElement : *value->arrayValue) {
        setData(element, "", arrayElement);
      }
      if (key.empty()) data.push_back(element); else data.set(key, element);
    } else if (value->type == Flows::VariableType::tStruct) {
      mustache::data element = mustache::data::type::object;
      for (const auto& pair : *value->structValue) {
        setData(element, pair.first, pair.second);
      }
      if (key.empty()) data.push_back(element); else data.set(key, element);
    } else {
      if (key.empty()) data.push_back(value->toString()); else data.set(key, value->toString());
    }
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void Template::input(const Flows::PNodeInfo &info, uint32_t index, const Flows::PVariable &message) {
  try {
    std::lock_guard<std::mutex> inputGuard(_inputMutex);
    _data = mustache::data();
    for (auto &element : *message->structValue) {
      setData(_data, element.first, element.second);
    }
    _data.set("variable", mustache::data(mustache::data::type::object));
    _data.set("flow", mustache::data(mustache::data::type::object));
    _data.set("global", mustache::data(mustache::data::type::object));
    _data.set("env", mustache::data(mustache::data::type::object));

    Flows::PVariable result;
    if (_mustache) result = std::make_shared<Flows::Variable>(_template->render(_data, std::function<void(mustache::DataSource, std::string)>(std::bind(&Template::addData, this, std::placeholders::_1, std::placeholders::_2))));
    else result = std::make_shared<Flows::Variable>(_plainTemplate);

    auto outputMessage = std::make_shared<Flows::Variable>();
    *outputMessage = *message;
    if (_parseJson) {
      Flows::PVariable json;
      result->stringValue = Flows::HelperFunctions::trim(result->stringValue);
      if (result->stringValue.empty()) json = std::make_shared<Flows::Variable>();
      json = Flows::JsonDecoder::decode(result->stringValue);
      if (_field.empty() && json->type == Flows::VariableType::tStruct) {
        outputMessage = json;
      } else {
        if (_field.empty()) _field = "payload";
        outputMessage->structValue->operator[](_field) = json;
      }
      output(0, outputMessage);
    } else {
      if (_field.empty()) _field = "payload";
      outputMessage->structValue->operator[](_field) = result;
      output(0, outputMessage);
    }
  }
  catch (const Flows::JsonDecoderException &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

}
