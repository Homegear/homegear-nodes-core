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
    auto settingsIterator = info->info->structValue->find("eventtype");
    if (settingsIterator != info->info->structValue->end()) {
      auto typeString = settingsIterator->second->stringValue;
      if (typeString == "devicevariables") _eventType = EventTypes::kDeviceVariables;
      else if (typeString == "servicemessages") _eventType = EventTypes::kServiceMessages;
      else if (typeString == "metadata") _eventType = EventTypes::kMetadataVariables;
      else if (typeString == "system") _eventType = EventTypes::kSystemVariables;
      else if (typeString == "flowvariables") _eventType = EventTypes::kFlowVariables;
      else if (typeString == "globalvariables") _eventType = EventTypes::kGlobalVariables;
      else if (typeString == "variableprofileevents") _eventType = EventTypes::kVariableProfile;
      else if (typeString == "uinotificationevents") _eventType = EventTypes::kUiNotification;
      else if (typeString == "deviceevents") _eventType = EventTypes::kDevice;
      else if (typeString == "rawpacketevents") _eventType = EventTypes::kRawPacket;
    }

    subscribeHomegearEvents();

    return true;
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return false;
}

void MyNode::homegearEvent(const std::string &type, const Flows::PArray &data) {
  try {
    if (_eventType == EventTypes::kDeviceVariables && type != "deviceVariableEvent") return;
    else if (_eventType == EventTypes::kServiceMessages && type != "serviceMessage") return;
    else if (_eventType == EventTypes::kMetadataVariables && type != "metadataVariableEvent") return;
    else if (_eventType == EventTypes::kSystemVariables && type != "systemVariableEvent") return;
    else if (_eventType == EventTypes::kFlowVariables && type != "flowVariableEvent") return;
    else if (_eventType == EventTypes::kGlobalVariables && type != "globalVariableEvent") return;
    else if (_eventType == EventTypes::kVariableProfile && type != "variableProfileStateChanged") return;
    else if (_eventType == EventTypes::kUiNotification && type != "uiNotificationCreated" && type != "uiNotificationRemoved" && type != "uiNotificationAction") return;
    else if (_eventType == EventTypes::kDevice && type != "newDevices" && type != "deleteDevices" && type != "updateDevice") return;
    else if (_eventType == EventTypes::kRawPacket && type != "rawPacketEvent") return;

    auto payload = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
    payload->structValue->emplace("type", std::make_shared<Flows::Variable>(type));
    payload->structValue->emplace("data", std::make_shared<Flows::Variable>(data));

    auto message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
    message->structValue->emplace("payload", payload);

    output(0, message);
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

}
