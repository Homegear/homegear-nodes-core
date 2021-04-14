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
      _path = settingsIterator->second->stringValue;
      if (!BaseLib::Io::directoryExists(_path)) {
        _out->printError("Path does not exist");
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

void MyNode::startUpComplete() {
  try {
    std::lock_guard<std::mutex> workerGuard(_workerThreadMutex);
    _stopThread = true;
    if (_workerThread.joinable()) {
      _workerThread.join();
    }

    _stopThread = false;
    _workerThread = std::thread(&MyNode::monitor, this);
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  catch (...) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
  }
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
  uint32_t mask = IN_CREATE | IN_DELETE | IN_OPEN | IN_CLOSE;
  const char *path = _path.c_str();
  int wd = inotify_add_watch(fd, path, mask);
  if (wd == -1) {
    throw errno;
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

      if (event_ptr->mask & IN_CREATE) {
        _out->printInfo("Create");
      }
      if (event_ptr->mask & IN_DELETE) {
        _out->printInfo("Delete");
      }
      if (event_ptr->mask & IN_OPEN) {
        _out->printInfo("Open");
      }
      if (event_ptr->mask & IN_CLOSE) {
        _out->printInfo("Close");
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  close(fd);
}
}

