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

#include <homegear-base/HelperFunctions/HelperFunctions.h>
#include "MyNode.h"

namespace MyNode {

MyNode::MyNode(const std::string &path, const std::string &type, const std::atomic_bool *frontendConnected)
    : Flows::INode(path, type, frontendConnected) {
}

MyNode::~MyNode() {
  _stopThread = true;
}

bool MyNode::init(const Flows::PNodeInfo &info) {
  try {
    auto settingsIterator = info->info->structValue->find("path");
    if (settingsIterator != info->info->structValue->end()) {
      std::string p = settingsIterator->second->stringValue;
      std::vector<std::string> path = BaseLib::HelperFunctions::splitAll(p, ',');
      for (size_t i = 0; i < path.size(); ++i) {
        if (path[i].empty()) {
          _out->printInfo("empty");
        } else if (!BaseLib::Io::directoryExists(path[i])) {
          path[i] = BaseLib::HelperFunctions::ltrim(path[i]);
          if (!BaseLib::Io::directoryExists(path[i])) {
            _out->printError("Path does not exist: " + path[i]);
          } else {
            _path.emplace_back(path[i]);
          }
        } else {
          _path.emplace_back(path[i]);
        }
      }

      if (_path.size() == 0) {
        return false;
      }
    }

    settingsIterator = info->info->structValue->find("recursive");
    if (settingsIterator != info->info->structValue->end()) {
      _recursive = settingsIterator->second->booleanValue;
      if (_recursive) {
        for (size_t i = 0; i < _path.size(); ++i) {
          std::vector<std::string> directories = BaseLib::Io::getDirectories(_path[i], true);
          for (size_t j = 0; j < directories.size(); ++j) {
            _path.emplace_back(_path[i] + "/" + directories[j]);
          }
        }
      }
    }
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
  try {
    std::lock_guard<std::mutex> workerGuard(_workerThreadMutex);
    _stopThread = true;
    if (_workerThread.joinable()) {
      _workerThread.join();
    }

    _stopThread = false;
    _workerThread = std::thread(&MyNode::monitor, this);
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
  try {
    _stopThread = true;
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void MyNode::waitForStop() {
  try {
    std::lock_guard<std::mutex> workerGuard(_workerThreadMutex);
    _stopThread = true;
    if (_workerThread.joinable()) {
      _workerThread.join();
    }
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
}

void MyNode::monitor() {
  int fd = inotify_init1(IN_NONBLOCK);
  if (fd == -1) {
    throw errno;
  }
  std::map<int, std::string> wd;
  for (size_t i = 0; i < _path.size(); i++) {
    const char *path = _path[i].c_str();
    int w = inotify_add_watch(fd, path, IN_ALL_EVENTS);
    if (w == -1){
      throw errno;
    }
    wd.insert_or_assign(w, _path[i]);
  }

  char buffer[sizeof(struct inotify_event) + NAME_MAX + 1] __attribute__ ((aligned(alignof(struct inotify_event))));
  const struct inotify_event *event_ptr;

  while (!_stopThread) {
    ssize_t len = read(fd, buffer, sizeof(buffer));
    if (len == -1 && errno != EAGAIN) {
      throw errno;
    }
    for (char *ptr = buffer; ptr < buffer + len; ptr += sizeof(struct inotify_event) + event_ptr->len) {
      event_ptr = (const struct inotify_event *) ptr;
      std::string payload, topic, type, action;
      std::atomic_bool del = false;
      std::atomic_bool create = false;

      if (event_ptr->mask & IN_ACCESS) {
        action = "Access";
      }
      if (event_ptr->mask & IN_ATTRIB) {
        action = "File attribute change";
      }
      if (event_ptr->mask & IN_CLOSE_WRITE) {
        action = "File opened for writing closed";
      }
      if (event_ptr->mask & IN_CLOSE_NOWRITE) {
        action = "File or directory not opened for writing was closed.";
      }
      if (event_ptr->mask & IN_CREATE) {
        action = "Create";
        create = true;
      }
      if (event_ptr->mask & IN_DELETE) {
        action = "Delete";
        del = true;
      }
      if (event_ptr->mask & IN_DELETE_SELF) {
        action = "Watched file/directory deleted";
      }
      if (event_ptr->mask & IN_MODIFY) {
        action = "File modified";
      }
      if (event_ptr->mask & IN_MOVE_SELF) {
        action = "Watched directory moved";
      }
      if (event_ptr->mask & IN_MOVED_FROM) {
        action = "Renamed directory/file in old directory";
      }
      if (event_ptr->mask & IN_MOVED_TO) {
        action = "Renamed directory/file in new directory";
      }
      if (event_ptr->mask & IN_OPEN) {
        action = "Open";
      }
      if (event_ptr->mask & IN_ISDIR) {
        type = "directory";
        if (_recursive && create){
          std::string p = wd.find(event_ptr->wd)->second + "/" + event_ptr->name;
          const char *path = p.c_str();
          int w = inotify_add_watch(fd, path, IN_ALL_EVENTS);
          if (w == -1){
            throw errno;
          }
          wd.insert_or_assign(w, (wd.find(event_ptr->wd)->second + "/" + event_ptr->name));
        }
      } else {
        type = "file";
      }

      topic = wd.find(event_ptr->wd)->second;

      if(del){
        std::string path = wd.find(event_ptr->wd)->second + "/" + event_ptr->name;
        int key = -1;
        for (auto w : wd){
          if (path.compare(w.second) == 0){
            key = w.first;
            break;
          }
        }
        if (key != -1){
          wd.erase(key);
        }
      }

      if(event_ptr->len){
        payload = topic + "/" + event_ptr->name;
      }
      _out->printInfo("payload: " + payload + ", topic: " + topic + ", type: " + type + ", action: " + action);

      if (!action.empty()){
        Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
        message->structValue->emplace("payload", std::make_shared<Flows::Variable>(payload));
        message->structValue->emplace("topic", std::make_shared<Flows::Variable>(topic));
        message->structValue->emplace("type", std::make_shared<Flows::Variable>(type));
        message->structValue->emplace("action", std::make_shared<Flows::Variable>(action));
        output(0, message, true);
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  close(fd);
}
}


