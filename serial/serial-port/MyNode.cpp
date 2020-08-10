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

MyNode::MyNode(const std::string &path, const std::string &nodeNamespace, const std::string &type, const std::atomic_bool *frontendConnected) : Flows::INode(path, nodeNamespace, type, frontendConnected) {
  _bl = std::make_shared<BaseLib::SharedObjects>();

  _localRpcMethods.emplace("registerNode", std::bind(&MyNode::registerNode, this, std::placeholders::_1));
  _localRpcMethods.emplace("write", std::bind(&MyNode::write, this, std::placeholders::_1));
}

MyNode::~MyNode() = default;

bool MyNode::init(const Flows::PNodeInfo &info) {
  _nodeInfo = info;
  return true;
}

bool MyNode::start() {
  try {
    auto settingsIterator = _nodeInfo->info->structValue->find("serialport");
    if (settingsIterator != _nodeInfo->info->structValue->end()) _serialPort = settingsIterator->second->stringValue;

    if (_serialPort.empty()) {
      _out->printError("Error: No serial device specified.");
      return false;
    }

    settingsIterator = _nodeInfo->info->structValue->find("serialbaud");
    if (settingsIterator != _nodeInfo->info->structValue->end()) _baudRate = Flows::Math::getNumber(settingsIterator->second->stringValue);

    if (_baudRate <= 0) {
      _out->printError("Error: Invalid baudrate specified.");
      return false;
    }

    settingsIterator = _nodeInfo->info->structValue->find("databits");
    if (settingsIterator != _nodeInfo->info->structValue->end()) {
      int32_t bits = Flows::Math::getNumber(settingsIterator->second->stringValue);

      if (bits == 8) _dataBits = BaseLib::SerialReaderWriter::CharacterSize::Eight;
      else if (bits == 7) _dataBits = BaseLib::SerialReaderWriter::CharacterSize::Seven;
      else if (bits == 6) _dataBits = BaseLib::SerialReaderWriter::CharacterSize::Six;
      else if (bits == 5) _dataBits = BaseLib::SerialReaderWriter::CharacterSize::Five;
      else {
        _out->printError("Error: Invalid character size specified.");
        return false;
      }
    }

    settingsIterator = _nodeInfo->info->structValue->find("parity");
    if (settingsIterator != _nodeInfo->info->structValue->end()) {
      _evenParity = false;
      _oddParity = false;
      _evenParity = (settingsIterator->second->stringValue == "even");
      _oddParity = (settingsIterator->second->stringValue == "odd");
    }

    settingsIterator = _nodeInfo->info->structValue->find("stopbits");
    if (settingsIterator != _nodeInfo->info->structValue->end()) _stopBits = Flows::Math::getNumber(settingsIterator->second->stringValue);

    settingsIterator = _nodeInfo->info->structValue->find("newline");
    if (settingsIterator != _nodeInfo->info->structValue->end()) {
      if (settingsIterator->second->stringValue.empty()) {
        _newLine = '\n';
      } else {
        if (settingsIterator->second->stringValue.size() > 1 && Flows::Math::isNumber(settingsIterator->second->stringValue, true)) {
          _newLine = (char)(uint8_t)Flows::Math::getNumber(settingsIterator->second->stringValue, true);
        } else if (settingsIterator->second->stringValue.size() == 2) {
          switch (settingsIterator->second->stringValue.at(1)) {
            case 'b':_newLine = '\b';
              break;
            case 'f':_newLine = '\f';
              break;
            case 'n':_newLine = '\n';
              break;
            case 'r':_newLine = '\r';
              break;
            case 't':_newLine = '\t';
              break;
            default:_newLine = settingsIterator->second->stringValue.front();
          }
        } else _newLine = settingsIterator->second->stringValue.front();
      }
      _timeout = Flows::Math::getNumber(settingsIterator->second->stringValue);
      if (_timeout < 0) _timeout = 1;
      else if (_timeout > 5000) _timeout = 5000;
      _fixedCount = Flows::Math::getNumber(settingsIterator->second->stringValue);
      if (_fixedCount < 1) _fixedCount = 1;
    }

    settingsIterator = _nodeInfo->info->structValue->find("bin");
    if (settingsIterator != _nodeInfo->info->structValue->end()) _binaryOutput = settingsIterator->second->booleanValue;

    settingsIterator = _nodeInfo->info->structValue->find("out");
    if (settingsIterator != _nodeInfo->info->structValue->end()) {
      std::string &splitType = settingsIterator->second->stringValue;
      if (splitType == "no") _splitType = SplitType::no;
      else if (splitType == "char") _splitType = SplitType::character;
      else if (splitType == "time") _splitType = SplitType::timeout;
      else if (splitType == "count") _splitType = SplitType::fixedLength;
    }

    settingsIterator = _nodeInfo->info->structValue->find("addchar");
    if (settingsIterator != _nodeInfo->info->structValue->end()) _addCharacter = settingsIterator->second->booleanValue;

    _serial = std::make_shared<BaseLib::SerialReaderWriter>(_bl.get(), _serialPort, _baudRate, 0, true, -1);
    reopen();

    _stopThread = false;
    _bl->threadManager.start(_readThread, true, &MyNode::listenThread, this);

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

void MyNode::stop() {
  _stopThread = true;
}

void MyNode::waitForStop() {
  try {
    _stopThread = true;
    _bl->threadManager.join(_readThread);
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

Flows::PVariable MyNode::getConfigParameterIncoming(const std::string &name) {
  try {
    auto settingsIterator = _nodeInfo->info->structValue->find(name);
    if (settingsIterator != _nodeInfo->info->structValue->end()) return settingsIterator->second;
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
  return std::make_shared<Flows::Variable>();
}

void MyNode::reopen() {
  try {
    _serial->closeDevice();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    _out->printInfo("Opening serial device " + _serialPort);
    _serial->openDevice(_evenParity, _oddParity, false, _dataBits, _stopBits == 2);
    _out->printInfo("Serial device opened.");
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void MyNode::packetReceived(const Flows::PVariable &data) {
  try {
    Flows::PArray parameters = std::make_shared<Flows::Array>();
    parameters->push_back(data);
    std::lock_guard<std::mutex> nodesGuard(_nodesMutex);
    for (const auto &node : _nodes) {
      invokeNodeMethod(node, "packetReceived", parameters, false);
    }
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void MyNode::listenThread() {
  std::vector<char> buffer;
  buffer.reserve(1024);
  char charData = 0;
  std::string stringData;
  int32_t readBytes = 0;
  while (!_stopThread) {
    try {
      if (_splitType == SplitType::character) {
        readBytes = _serial->readLine(stringData, 500000, _newLine);

        if (readBytes == 0) {
          if (_binaryOutput) {
            packetReceived(std::make_shared<Flows::Variable>(std::vector<char>(stringData.begin(), stringData.end())));
          } else packetReceived(std::make_shared<Flows::Variable>(BaseLib::HelperFunctions::getHexString(stringData)));
        }
      } else if (_splitType == SplitType::no) {
        readBytes = _serial->readChar(charData);

        if (readBytes == 0) {
          if (_binaryOutput) {
            packetReceived(std::make_shared<Flows::Variable>(std::vector<char>{charData}));
          } else packetReceived(std::make_shared<Flows::Variable>(BaseLib::HelperFunctions::getHexString(&charData, 1)));
        }
      } else if (_splitType == SplitType::timeout) {
        readBytes = 0;
        while (!_stopThread && readBytes == 0) {
          readBytes = _serial->readChar(charData, _timeout * 1000);
          if (readBytes == 0) {
            if (buffer.size() + 1 > buffer.capacity()) buffer.reserve(buffer.capacity() + 1024);
            buffer.push_back(charData);
          }
        }

        if (!_stopThread && !buffer.empty()) {
          if (_binaryOutput) packetReceived(std::make_shared<Flows::Variable>(buffer));
          else packetReceived(std::make_shared<Flows::Variable>(BaseLib::HelperFunctions::getHexString(buffer)));
          buffer.clear();
        }
      } else if (_splitType == SplitType::fixedLength) {
        readBytes = _serial->readChar(charData);
        if (readBytes == 0) {
          buffer.push_back(charData);

          if (buffer.size() >= (unsigned)_fixedCount) {
            if (_binaryOutput) packetReceived(std::make_shared<Flows::Variable>(buffer));
            else packetReceived(std::make_shared<Flows::Variable>(BaseLib::HelperFunctions::getHexString(buffer)));

            buffer.clear();
          }
        }
      }

      if (readBytes == -1) reopen();
    }
    catch (const std::exception &ex) {
      _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch (...) {
      _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
  }
}

//{{{ RPC methods
Flows::PVariable MyNode::registerNode(const Flows::PArray &parameters) {
  try {
    if (parameters->size() != 1) return Flows::Variable::createError(-1, "Method expects exactly one parameter. " + std::to_string(parameters->size()) + " given.");
    if (parameters->at(0)->type != Flows::VariableType::tString || parameters->at(0)->stringValue.empty()) return Flows::Variable::createError(-1, "Parameter is not of type string.");

    std::lock_guard<std::mutex> nodesGuard(_nodesMutex);
    _nodes.emplace(parameters->at(0)->stringValue);

    return std::make_shared<Flows::Variable>();
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
  return Flows::Variable::createError(-32500, "Unknown application error.");
}

Flows::PVariable MyNode::write(const Flows::PArray &parameters) {
  try {
    if (parameters->size() != 1) return Flows::Variable::createError(-1, "Method expects exactly one parameter.");
    if (parameters->at(0)->type != Flows::VariableType::tString && parameters->at(0)->type != Flows::VariableType::tBinary) return Flows::Variable::createError(-1, "Parameter is not of type Binary or String.");
    if (parameters->at(0)->binaryValue.empty() && parameters->at(0)->stringValue.empty()) return Flows::Variable::createError(-1, "No data given.");

    if (parameters->at(0)->type == Flows::VariableType::tString) parameters->at(0)->binaryValue.insert(parameters->at(0)->binaryValue.end(), parameters->at(0)->stringValue.begin(), parameters->at(0)->stringValue.end());
    if (_addCharacter && _splitType == SplitType::character) parameters->at(0)->binaryValue.push_back(_newLine);
    _serial->writeData(parameters->at(0)->binaryValue);

    return std::make_shared<Flows::Variable>();
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
  return Flows::Variable::createError(-32500, "Unknown application error.");
}
//}}}

}
