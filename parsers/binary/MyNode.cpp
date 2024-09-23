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

const std::array<int32_t, 23> MyNode::_asciiToBinaryTable{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0, 0, 10, 11, 12, 13, 14, 15};

MyNode::MyNode(const std::string &path, const std::string &type, const std::atomic_bool *frontendConnected) : Flows::INode(path, type, frontendConnected) {
}

MyNode::~MyNode() = default;

bool MyNode::init(const Flows::PNodeInfo &info) {
  auto settingsIterator = info->info->structValue->find("input-type");
  if (settingsIterator != info->info->structValue->end()) _inputIsBinary = settingsIterator->second->stringValue == "binary";

  return true;
}

void MyNode::input(const Flows::PNodeInfo &info, uint32_t index, const Flows::PVariable &message) {
  try {
    Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);

    if (_inputIsBinary) {
      if (message->structValue->at("payload")->type == Flows::VariableType::tBinary) {
        outputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>(Flows::HelperFunctions::getHexString(message->structValue->at("payload")->binaryValue)));
        output(0, outputMessage);
      } else if (message->structValue->at("payload")->type == Flows::VariableType::tString) {
        outputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>(Flows::HelperFunctions::getHexString(message->structValue->at("payload")->stringValue)));
        output(0, outputMessage);
      }
    } else {
      outputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>(getUBinary(message->structValue->at("payload")->stringValue)));
      output(0, outputMessage);
    }
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

std::string MyNode::getUBinary(const std::string &hexString) {
  std::string binary;
  if (hexString.empty()) return binary;
  if (hexString.size() % 2 != 0 && !std::isspace(hexString.back())) {
    std::string hexStringCopy = hexString.substr(1);
    binary.reserve(hexStringCopy.size() / 2);
    for (int32_t i = 0; i < (signed)hexStringCopy.size(); i += 2) {
      uint8_t byte = 0;
      if (i < (signed)hexStringCopy.size() && isxdigit(hexStringCopy[i])) byte = (uint8_t)(_asciiToBinaryTable[std::toupper(hexStringCopy[i]) - '0'] << 4);
      else continue;
      if (i + 1 < (signed)hexStringCopy.size() && isxdigit(hexStringCopy[i + 1])) byte += _asciiToBinaryTable[std::toupper(hexStringCopy[i + 1]) - '0'];
      else continue;
      binary.push_back(byte);
    }
  } else {
    binary.reserve(hexString.size() / 2);
    for (int32_t i = 0; i < (signed)hexString.size(); i += 2) {
      uint8_t byte = 0;
      if (i < (signed)hexString.size() && isxdigit(hexString[i])) byte = (uint8_t)(_asciiToBinaryTable[std::toupper(hexString[i]) - '0'] << 4);
      else continue;
      if (i + 1 < (signed)hexString.size() && isxdigit(hexString[i + 1])) byte += _asciiToBinaryTable[std::toupper(hexString[i + 1]) - '0'];
      else continue;
      binary.push_back(byte);
    }
  }
  return binary;
}

}
