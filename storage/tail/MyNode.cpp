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

MyNode::~MyNode() = default;

bool MyNode::init(const Flows::PNodeInfo &info) {
  try {
    auto settingsIterator = info->info->structValue->find("path");
    if (settingsIterator != info->info->structValue->end()) {
      std::string inputPath = settingsIterator->second->stringValue;
      std::vector<std::string> path = BaseLib::HelperFunctions::splitAll(inputPath, ',');
      for (auto &p : path) {
        if (!p.empty()) {
          if (!BaseLib::Io::fileExists(p)) {
            p = BaseLib::HelperFunctions::ltrim(p);
            if (!BaseLib::Io::fileExists(p)) {
              _out->printError("Path does not exist or is not a file: " + p);
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
    _workerThread = std::thread(&MyNode::tail, this);
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

void MyNode::tail() {
  int fd = inotify_init1(IN_NONBLOCK);
  if (fd == -1) {
    throw errno;
  }

  std::map<int, std::string> wd;
  for (size_t i = 0; i < _path.size(); i++) {
    const char *path = _path[i].c_str();
    int w = inotify_add_watch(fd, path, IN_MODIFY | IN_DELETE_SELF);
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
      auto it = wd.find(event_ptr->wd);

      if (event_ptr->mask & IN_MODIFY) {
        if (it != wd.end()) {
          std::ifstream fs(it->second, std::ifstream::in);
          if (fs) {
            fs.seekg(0, fs.end);
            int length = fs.tellg();

            int n = 2;
            while (fs.tellg() != 0 && n) {
              fs.seekg(-1, fs.cur);
              if (fs.peek() == '\n') {
                n--;
              }
            }
            if (fs.peek() == '\n') {
              fs.seekg(1, fs.cur);
            }

            length = length - fs.tellg();
            if (length > 1) {
              char line[length];
              fs.getline(line, length);


              std::string l(line);
              std::string filePath;
              if (it != wd.end()) {
                filePath = it->second;
              }
              _out->printInfo("line: " + l + ", file path: " + filePath);
              Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
              message->structValue->emplace("payload", std::make_shared<Flows::Variable>(l));
              message->structValue->emplace("file", std::make_shared<Flows::Variable>(filePath));
              output(0, message, false);
            }
            fs.close();
          }

        }
      } else if (event_ptr->mask & IN_DELETE_SELF) {
        if (it != wd.end()) {
          wd.erase(it);
        }
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  close(fd);
}
}