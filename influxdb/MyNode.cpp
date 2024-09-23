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

MyNode::MyNode(const std::string &path, const std::string &type, const std::atomic_bool *frontendConnected) : Flows::INode(path, type, frontendConnected) {
}

MyNode::~MyNode() = default;

bool MyNode::init(const Flows::PNodeInfo &info) {
  try {
    auto settingsIterator = info->info->structValue->find("measurement");
    if (settingsIterator != info->info->structValue->end()) _measurement = stripNonAlphaNumeric(settingsIterator->second->stringValue);

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

bool MyNode::start() {
  return true;
}

std::string MyNode::stripNonAlphaNumeric(const std::string &s) {
  std::string strippedString;
  strippedString.reserve(s.size());
  for (char i : s) {
    if (i == ' ') strippedString.push_back('_');
    else if (isalpha(i) || isdigit(i) || (i == '_') || (i == '-') || (i == '.') || (i == ';')) strippedString.push_back(i);
  }
  return strippedString;
}

void MyNode::input(const Flows::PNodeInfo &info, uint32_t index, const Flows::PVariable &message) {
  try {
    auto measurementIterator = message->structValue->find("measurement");
    std::string measurement = _measurement;
    if (measurementIterator != message->structValue->end() && !measurementIterator->second->stringValue.empty()) measurement = stripNonAlphaNumeric(measurementIterator->second->stringValue);
    if (measurement.empty()) return;

    Flows::PVariable &input = message->structValue->at("payload");
    if (input->type != Flows::VariableType::tBoolean && input->type != Flows::VariableType::tFloat && input->type != Flows::VariableType::tInteger && input->type != Flows::VariableType::tInteger64 && input->type != Flows::VariableType::tString
        && input->type != Flows::VariableType::tBase64) {
      Flows::PVariable encodedValue = std::make_shared<Flows::Variable>(Flows::VariableType::tString);
      encodedValue->stringValue = _jsonEncoder.getString(input);
      input = encodedValue;
    }

    std::string query = measurement + " value=" + (input->type == Flows::VariableType::tString ? "\"" : "") + input->toString() + (input->type == Flows::VariableType::tString ? "\"" : "")
        + (input->type == Flows::VariableType::tInteger || input->type == Flows::VariableType::tInteger64 ? "i" : "");
    Flows::PArray parameters = std::make_shared<Flows::Array>();
    parameters->reserve(2);
    parameters->push_back(std::make_shared<Flows::Variable>(false));
    parameters->push_back(std::make_shared<Flows::Variable>(query));

    Flows::PVariable result = invoke("influxdbWrite", parameters);
    if (result && result->errorStruct) _out->printError("Error writing data to InfluxDB: " + result->structValue->at("faultString")->stringValue);

    if (input->type != Flows::VariableType::tFloat && input->type != Flows::VariableType::tInteger && input->type != Flows::VariableType::tInteger64) {
      parameters->at(0)->booleanValue = true;
      result = invoke("influxdbWrite", parameters);
      if (result && result->errorStruct) _out->printError("Error writing data to InfluxDB: " + result->structValue->at("faultString")->stringValue);
    } else if (_first) {
      parameters = std::make_shared<Flows::Array>();
      parameters->push_back(std::make_shared<Flows::Variable>(measurement));
      result = invoke("influxdbCreateContinuousQuery", parameters);
      if (!result->errorStruct) _first = false;
    }
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

}
