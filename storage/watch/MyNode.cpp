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
      std::string inputPath = settingsIterator->second->stringValue;
      std::vector<std::string> path = BaseLib::HelperFunctions::splitAll(inputPath, ',');
      for (auto &p : path) {
        if (!p.empty()) {
          if (!(BaseLib::Io::directoryExists(p) || BaseLib::Io::fileExists(p))) {
            p = BaseLib::HelperFunctions::ltrim(p);
            if (!(BaseLib::Io::directoryExists(p) || BaseLib::Io::fileExists(p))) {
              _out->printError("Path does not exist: " + p);
            } else {
              _path.emplace_back(p);
            }
          } else {
            _path.emplace_back(p);
          }
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
        for (auto &path : _path) {
          if (BaseLib::Io::directoryExists(path)){
            std::vector<std::string> directories = BaseLib::Io::getDirectories(path, true);
            for (auto &directory : directories) {
              _path.emplace_back(path + "/" + directory);
            }
          }
        }
      }
    }

    settingsIterator = info->info->structValue->find("allEvents");
    if (settingsIterator != info->info->structValue->end()) {
      if (settingsIterator->second->booleanValue) {
        mask = IN_ALL_EVENTS;
      } else {
        settingsIterator = info->info->structValue->find("accessEvent");
        if (settingsIterator != info->info->structValue->end()) {
          if (settingsIterator->second->booleanValue) {
            mask = mask | IN_ACCESS;
          }
        }
        settingsIterator = info->info->structValue->find("attributeEvent");
        if (settingsIterator != info->info->structValue->end()) {
          if (settingsIterator->second->booleanValue) {
            mask = mask | IN_ATTRIB;
          }
        }
        settingsIterator = info->info->structValue->find("closeWriteEvent");
        if (settingsIterator != info->info->structValue->end()) {
          if (settingsIterator->second->booleanValue) {
            mask = mask | IN_CLOSE_WRITE;
          }
        }
        settingsIterator = info->info->structValue->find("closeNoWriteEvent");
        if (settingsIterator != info->info->structValue->end()) {
          if (settingsIterator->second->booleanValue) {
            mask = mask | IN_CLOSE_NOWRITE;
          }
        }
        settingsIterator = info->info->structValue->find("createEvent");
        if (settingsIterator != info->info->structValue->end()) {
          if (settingsIterator->second->booleanValue) {
            mask = mask | IN_CREATE;
          }
        }
        settingsIterator = info->info->structValue->find("deleteEvent");
        if (settingsIterator != info->info->structValue->end()) {
          if (settingsIterator->second->booleanValue) {
            mask = mask | IN_DELETE;
          }
        }
        settingsIterator = info->info->structValue->find("selfDeleteEvent");
        if (settingsIterator != info->info->structValue->end()) {
          if (settingsIterator->second->booleanValue) {
            mask = mask | IN_DELETE_SELF;
          }
        }
        settingsIterator = info->info->structValue->find("modifyEvent");
        if (settingsIterator != info->info->structValue->end()) {
          if (settingsIterator->second->booleanValue) {
            mask = mask | IN_MODIFY;
          }
        }
        settingsIterator = info->info->structValue->find("selfMoveEvent");
        if (settingsIterator != info->info->structValue->end()) {
          if (settingsIterator->second->booleanValue) {
            mask = mask | IN_MOVE_SELF;
          }
        }
        settingsIterator = info->info->structValue->find("moveFromEvent");
        if (settingsIterator != info->info->structValue->end()) {
          if (settingsIterator->second->booleanValue) {
            mask = mask | IN_MOVED_FROM;
          }
        }
        settingsIterator = info->info->structValue->find("moveToEvent");
        if (settingsIterator != info->info->structValue->end()) {
          if (settingsIterator->second->booleanValue) {
            mask = mask | IN_MOVED_TO;
          }
        }
        settingsIterator = info->info->structValue->find("openEvent");
        if (settingsIterator != info->info->structValue->end()) {
          if (settingsIterator->second->booleanValue) {
            mask = mask | IN_OPEN;
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
    int w = inotify_add_watch(fd, path, mask);
    if (w == -1) {
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
      std::string payload, topic, type, filePath = "";
      bool del = false;
      bool create = false;
      auto it = wd.find(event_ptr->wd);

      if (event_ptr->mask & IN_ACCESS) {
        payload = "IN_ACCESS";
      }
      if (event_ptr->mask & IN_ATTRIB) {
        payload = "IN_ATTRIB";
      }
      if (event_ptr->mask & IN_CLOSE_WRITE) {
        payload = "IN_CLOSE_WRITE";
      }
      if (event_ptr->mask & IN_CLOSE_NOWRITE) {
        payload = "IN_CLOSE_NOWRITE";
      }
      if (event_ptr->mask & IN_CREATE) {
        payload = "IN_CREATE";
        create = true;
      }
      if (event_ptr->mask & IN_DELETE) {
        payload = "IN_DELETE";
        del = true;
      }
      if (event_ptr->mask & IN_DELETE_SELF) {
        payload = "IN_DELETE_SELF";
      }
      if (event_ptr->mask & IN_MODIFY) {
        payload = "IN_MODIFY";
      }
      if (event_ptr->mask & IN_MOVE_SELF) {
        payload = "IN_MOVE_SELF";
      }
      if (event_ptr->mask & IN_MOVED_FROM) {
        payload = "IN_MOVE_FROM";
      }
      if (event_ptr->mask & IN_MOVED_TO) {
        payload = "IN_MOVED_TO";
      }
      if (event_ptr->mask & IN_OPEN) {
        payload = "IN_OPEN";
      }
      if (event_ptr->mask & IN_ISDIR) {
        type = "directory";
        if (_recursive && create) {
          if (it != wd.end()) {
            std::string p = it->second + "/" + event_ptr->name;
            const char *path = p.c_str();
            int w = inotify_add_watch(fd, path, IN_ALL_EVENTS);
            if (w == -1) {
              throw errno;
            }
            wd.insert_or_assign(w, it->second + "/" + event_ptr->name);
          }
        }
      } else {
        type = "file";
      }

      if (it != wd.end()) {
        topic = it->second;
      }

      if (del) {
        if (it != wd.end()) {
          std::string path = it->second + "/" + event_ptr->name;
          int key = -1;
          for (auto w : wd) {
            if (path.compare(w.second) == 0) {
              key = w.first;
              break;
            }
          }
          if (key != -1) {
            wd.erase(key);
          }
        }
      }

      if (event_ptr->len) {
        filePath = topic + "/" + event_ptr->name;
      }
      _out->printInfo("payload: " + payload + ", path: " + topic + ", type: " + type + ", filepath: " + filePath);

      if (!payload.empty()) {
        Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
        message->structValue->emplace("payload", std::make_shared<Flows::Variable>(payload));
        if (!filePath.empty()) {
          message->structValue->emplace("file path", std::make_shared<Flows::Variable>(payload));
        }
        if (!topic.empty()) {
          message->structValue->emplace("path", std::make_shared<Flows::Variable>(topic));
        }
        if (!type.empty()) {
          message->structValue->emplace("type", std::make_shared<Flows::Variable>(type));
        }
        output(0, message, true); //for synchronous output set to true //TODO
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  close(fd);
}
}


