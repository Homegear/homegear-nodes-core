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

#include "Python.h"
#include "homegear-base/Managers/ProcessManager.h"
#include <sys/resource.h>
#include <sys/stat.h>

namespace Python {

Python::Python(const std::string &path, const std::string &type, const std::atomic_bool *frontendConnected) : Flows::INode(path, type, frontendConnected) {
}

Python::~Python() {
  _stopThread = true;
  if (_pid != -1) kill(_pid, 9);
  if (_execThread.joinable()) _execThread.join();
  if (_errorThread.joinable()) _errorThread.join();
  if (_callbackHandlerId != -1) BaseLib::ProcessManager::unregisterCallbackHandler(_callbackHandlerId);
}

bool Python::init(const Flows::PNodeInfo &info) {
  try {
    auto settingsIterator = info->info->structValue->find("func");
    std::string code;
    if (settingsIterator != info->info->structValue->end()) code = settingsIterator->second->stringValue;

    _codeFile = "/tmp/node-blue-node-" + _id + ".py";
    BaseLib::Io::writeFile(_codeFile, code);
    if (chmod(_codeFile.c_str(), S_IRUSR | S_IWUSR) == -1) _out->printError("Error: Could not set permissions on " + _codeFile);

    return true;
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return false;
}

bool Python::start() {
  try {
    _callbackHandlerId = BaseLib::ProcessManager::registerCallbackHandler(std::function<void(pid_t pid, int exitCode, int signal, bool coreDumped)>(std::bind(&Python::sigchildHandler,
                                                                                                                                                              this,
                                                                                                                                                              std::placeholders::_1,
                                                                                                                                                              std::placeholders::_2,
                                                                                                                                                              std::placeholders::_3,
                                                                                                                                                              std::placeholders::_4)));

    startProgram();

    {
      while (!_startUpError) {
        if (_processStartUpComplete) break;

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
    }

    if (_startUpError) return false;

    return true;
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
  return false;
}

void Python::startUpComplete() {
  try {
    if (_pid == -1) return;

    callStartUpComplete();

    _startUpComplete = true;
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Python::stop() {
  try {
    _processStartUpComplete = false;
    _stopThread = true;
    if (_pid != -1) kill(_pid, 15);
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Python::waitForStop() {
  try {
    if (_pid != -1) kill(_pid, 15);
    for (int32_t i = 0; i < 600; i++) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      if (_pid == -1) break;
    }
    if (_pid != -1) {
      _out->printError("Error: Process did not finish within 60 seconds. Killing it.");
      kill(_pid, 9);
      close(_stdIn);
      close(_stdOut);
      close(_stdErr);
      _stdIn = -1;
      _stdOut = -1;
      _stdErr = -1;
    }
    if (_execThread.joinable()) _execThread.join();
    if (_errorThread.joinable()) _errorThread.join();
    BaseLib::ProcessManager::unregisterCallbackHandler(_callbackHandlerId);
    _callbackHandlerId = -1;

    BaseLib::Io::deleteFile(_codeFile);
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Python::input(const Flows::PNodeInfo &info, uint32_t index, const Flows::PVariable &message) {
  try {
    if (index == 0) {
      if (_pid != -1) kill(_pid, message->structValue->at("payload")->integerValue64);
    } else if (index == 1) {
      if (_stdIn != -1) {
        size_t totalBytesWritten = 0;
        while (totalBytesWritten < message->structValue->at("payload")->stringValue.size()) {
          int bytesWritten = -1;
          do {
            bytesWritten = write(_stdIn, message->structValue->at("payload")->stringValue.data(), message->structValue->at("payload")->stringValue.size());
          } while (bytesWritten == -1 && (errno == EAGAIN || errno == EINTR));
          if (bytesWritten <= 0) {
            _out->printWarning("Warning: Could not write to STDIN: " + std::string(strerror(errno)));
            break;
          }
          totalBytesWritten += bytesWritten;
        }
      }
    } else {
      auto nodeInputParameters = std::make_shared<Flows::Array>();
      nodeInputParameters->reserve(3);
      nodeInputParameters->emplace_back(info->serialize());
      nodeInputParameters->emplace_back(std::make_shared<Flows::Variable>(index));
      nodeInputParameters->emplace_back(message);

      auto parameters = std::make_shared<Flows::Array>();
      parameters->reserve(3);
      parameters->emplace_back(std::make_shared<Flows::Variable>(_pid));
      parameters->emplace_back(std::make_shared<Flows::Variable>("nodeInput"));
      parameters->emplace_back(std::make_shared<Flows::Variable>(nodeInputParameters));
      auto result = invoke("invokeIpcProcessMethod", parameters);
      if (result->errorStruct) _out->printError("Error calling nodeInput: " + result->structValue->at("faultString")->stringValue);
    }
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Python::startProgram() {
  try {
    if (_execThread.joinable()) _execThread.join();
    if (_errorThread.joinable()) _errorThread.join();
    _execThread = std::thread(&Python::execThread, this);
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

int32_t Python::getMaxFd() {
  struct rlimit limits{};
  if (getrlimit(RLIMIT_NOFILE, &limits) == -1 || limits.rlim_cur >= INT32_MAX) {
    return 1024;
  }
  return limits.rlim_cur;
}

void Python::execThread() {
  try {
    do {
      if (_stdErr != -1) {
        close(_stdErr);
        _stdErr = -1;
      }
      if (_errorThread.joinable()) _errorThread.join();

      _processStartUpComplete = false;

      std::string pythonExecutablePath;

      { //Get Python path
        std::string pathEnv = getenv("PATH");
        auto paths = BaseLib::HelperFunctions::splitAll(pathEnv, ':');
        for (auto &path : paths) {
          if (path.empty()) continue;
          if (path.back() != '/') path.push_back('/');
          if (BaseLib::Io::fileExists(path + "python3")) {
            pythonExecutablePath = path + "python3";
            break;
          }
        }

        if (pythonExecutablePath.empty()) {
          for (auto &path : paths) {
            if (path.empty()) continue;
            if (path.back() != '/') path.push_back('/');
            if (BaseLib::Io::fileExists(path + "python")) {
              pythonExecutablePath = path + "python";
              break;
            }
          }
        }
      }

      if (pythonExecutablePath.empty()) {
        if (BaseLib::Io::fileExists("/usr/bin/python3")) pythonExecutablePath = "/usr/bin/python3";
        else if (BaseLib::Io::fileExists("/usr/local/bin/python3")) pythonExecutablePath = "/usr/local/bin/python3";
        else {
          _startUpError = true;
          _out->printError("Error: No Python executable found. Please install Python.");
          return;
        }
      }

      std::string socketPath;
      {
        auto socketPathParameters = std::make_shared<Flows::Array>();
        socketPathParameters->emplace_back(std::make_shared<Flows::Variable>("socketPath"));
        auto result = invoke("getSystemVariable", socketPathParameters);
        socketPath = result->stringValue;
      }
      if (socketPath.empty()) {
        _startUpError = true;
        _out->printError("Error: Could not get socket path.");
        return;
      }
      BaseLib::HelperFunctions::stringReplace(socketPath, "\"", "\\\"");
      socketPath = "\"" + socketPath + "homegearIPC.sock" + "\"";

      std::vector<std::string> arguments;
      arguments.emplace_back(_codeFile);
      arguments.emplace_back(socketPath);
      arguments.emplace_back(_id);

      int stdIn = -1;
      int stdOut = -1;
      int stdErr = -1;
      _pid = BaseLib::ProcessManager::systemp(pythonExecutablePath, arguments, getMaxFd(), stdIn, stdOut, stdErr);
      _stdIn = stdIn;
      _stdOut = stdOut;
      _stdErr = stdErr;

      _errorThread = std::thread(&Python::errorThread, this);

      while (_pid != -1) //While process is running
      {
        auto parameters = std::make_shared<Flows::Array>();
        parameters->reserve(3);
        parameters->emplace_back(std::make_shared<Flows::Variable>(_pid));
        parameters->emplace_back(std::make_shared<Flows::Variable>("ping"));
        parameters->emplace_back(std::make_shared<Flows::Variable>(Flows::VariableType::tArray));
        auto result = invoke("invokeIpcProcessMethod", parameters);
        if (!result->errorStruct) break; //Process started and responding

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
      if (_pid == -1) _startUpError = true;
      else _processStartUpComplete = true;
      if (_startUpComplete) {
        //Delay a little for slow systems
        std::this_thread::sleep_for(std::chrono::milliseconds(10000));
        callStartUpComplete();
      }

      std::array<uint8_t, 4096> buffer{};
      std::string bufferOut;
      while (_stdOut != -1) {
        auto bytesRead = 0;
        bufferOut.clear();
        do {
          bytesRead = read(_stdOut, buffer.data(), buffer.size());
          if (bytesRead > 0) {
            bufferOut.insert(bufferOut.end(), buffer.begin(), buffer.begin() + bytesRead);
          }
        } while (bytesRead > 0);

        if (!bufferOut.empty()) {
          auto outputVector = BaseLib::HelperFunctions::splitAll(bufferOut, '\n');

          Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
          Flows::PVariable outputArray = std::make_shared<Flows::Variable>(Flows::VariableType::tArray);
          outputArray->arrayValue->reserve(outputVector.size());
          for (int32_t i = 0; i < (signed)outputVector.size(); i++) {
            if (i == (signed)outputVector.size() - 1 && outputVector[i].empty()) continue;
            outputArray->arrayValue->emplace_back(std::make_shared<Flows::Variable>(std::move(outputVector[i])));
          }
          message->structValue->emplace("payload", outputArray);
          output(1, message);
        }
      }

      std::this_thread::sleep_for(std::chrono::seconds(1));
    } while (!_stopThread);
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Python::errorThread() {
  try {
    std::array<uint8_t, 4096> buffer{};
    std::string bufferErr;
    while (_stdErr != -1) {
      int32_t bytesRead = 0;
      bufferErr.clear();
      do {
        bytesRead = read(_stdErr, buffer.data(), buffer.size());
        if (bytesRead > 0) {
          bufferErr.insert(bufferErr.end(), buffer.begin(), buffer.begin() + bytesRead);
        }
      } while (bytesRead > 0);

      if (!bufferErr.empty()) {
        _out->printError("Process error output: " + bufferErr);

        auto outputVector = BaseLib::HelperFunctions::splitAll(bufferErr, '\n');

        Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
        Flows::PVariable outputArray = std::make_shared<Flows::Variable>(Flows::VariableType::tArray);
        outputArray->arrayValue->reserve(outputVector.size());
        for (int32_t i = 0; i < (signed)outputVector.size(); i++) {
          if (i == (signed)outputVector.size() - 1 && outputVector[i].empty()) continue;
          outputArray->arrayValue->emplace_back(std::make_shared<Flows::Variable>(std::move(outputVector[i])));
        }
        message->structValue->emplace("payload", outputArray);
        output(2, message);
      }
    }
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

void Python::sigchildHandler(pid_t pid, int exitCode, int signal, bool coreDumped) {
  try {
    if (pid == _pid) {
      close(_stdIn);
      close(_stdOut);
      close(_stdErr);
      _stdIn = -1;
      _stdOut = -1;
      _stdErr = -1;
      _pid = -1;
      Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
      message->structValue->emplace("coreDumped", std::make_shared<Flows::Variable>(coreDumped));
      message->structValue->emplace("signal", std::make_shared<Flows::Variable>(signal));
      message->structValue->emplace("payload", std::make_shared<Flows::Variable>(exitCode));
      output(0, message);
    }
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

int32_t Python::read(std::atomic_int &fd, uint8_t *buffer, int32_t bufferSize) {
  if (fd == -1) return 0;

  timeval timeout{};
  timeout.tv_sec = 0;
  timeout.tv_usec = 100000;
  fd_set readFileDescriptor{};
  FD_ZERO(&readFileDescriptor);
  int32_t nfds = fd + 1;
  if (nfds <= 0) {
    close(fd);
    fd = -1;
    return -1;
  }
  FD_SET(fd, &readFileDescriptor);
  auto bytesRead = select(nfds, &readFileDescriptor, nullptr, nullptr, &timeout);
  if (bytesRead == 0) {
    return 0;
  }
  if (bytesRead != 1) {
    close(fd);
    fd = -1;
    return -1;
  }
  do {
    bytesRead = ::read(fd, buffer, bufferSize);
  } while (bytesRead < 0 && (errno == EAGAIN || errno == EINTR));
  if (bytesRead <= 0) {
    if (bytesRead == -1) {
      if (errno == ETIMEDOUT) return 0;
      else {
        close(fd);
        fd = -1;
        return -1;
      }
    } else {
      close(fd);
      fd = -1;
      return -1;
    }
  }
  if (bytesRead > bufferSize) bytesRead = bufferSize;
  return bytesRead;
}

void Python::callStartUpComplete() {
  try {
    auto innerParameters = std::make_shared<Flows::Array>();
    innerParameters->reserve(5);
    innerParameters->emplace_back(std::make_shared<Flows::Variable>("nodeBlue"));
    innerParameters->emplace_back(std::make_shared<Flows::Variable>(0));
    innerParameters->emplace_back(std::make_shared<Flows::Variable>(-1));
    innerParameters->emplace_back(std::make_shared<Flows::Variable>(Flows::VariableType::tArray));
    innerParameters->emplace_back(std::make_shared<Flows::Variable>(Flows::VariableType::tArray));
    innerParameters->at(3)->arrayValue->emplace_back(std::make_shared<Flows::Variable>("startUpComplete"));
    innerParameters->at(4)->arrayValue->emplace_back(std::make_shared<Flows::Variable>(true));

    auto parameters = std::make_shared<Flows::Array>();
    parameters->reserve(3);
    parameters->emplace_back(std::make_shared<Flows::Variable>(_pid));
    parameters->emplace_back(std::make_shared<Flows::Variable>("broadcastEvent"));
    parameters->emplace_back(std::make_shared<Flows::Variable>(innerParameters));
    auto result = invoke("invokeIpcProcessMethod", parameters);
    if (result->errorStruct) _out->printError("Error calling broadcastEvent on process: " + result->structValue->at("faultString")->stringValue);
  }
  catch (const std::exception &ex) {
    _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
  }
}

}
