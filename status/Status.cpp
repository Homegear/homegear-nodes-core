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

#include "Status.h"

namespace Status {

Status::Status(const std::string &path, const std::string &type, const std::atomic_bool *frontendConnected) : Flows::INode(path, type, frontendConnected) {
}

Status::~Status() = default;

bool Status::init(const Flows::PNodeInfo &info) {
  try {
    auto settingsIterator = info->info->structValue->find("scope");
    if (settingsIterator != info->info->structValue->end()) {
      for (auto &element : *settingsIterator->second->arrayValue) {
        if (!element->stringValue.empty()) _scope.emplace(element->stringValue);
      }
    }

    subscribeStatusEvents();

    return true;
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return false;
}

void Status::statusEvent(const std::string &nodeId, const Flows::PVariable &status) {
  try {
    if (_scope.empty() || _scope.find(nodeId) != _scope.end()) {
      auto message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
      message->structValue->emplace("payload", status);
      output(0, message);
    }
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

}
