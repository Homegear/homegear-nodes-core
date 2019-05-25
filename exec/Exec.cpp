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

#include "Exec.h"
#include "homegear-base/Managers/ProcessManager.h"
#include <sys/resource.h>

namespace Exec
{

Exec::Exec(std::string path, std::string nodeNamespace, std::string type, const std::atomic_bool* frontendConnected) : Flows::INode(path, nodeNamespace, type, frontendConnected)
{
}

Exec::~Exec()
{
}

bool Exec::init(Flows::PNodeInfo info)
{
	try
	{
		auto settingsIterator = info->info->structValue->find("filename");
		if(settingsIterator != info->info->structValue->end()) _filename = settingsIterator->second->stringValue;

        settingsIterator = info->info->structValue->find("arguments");
        if(settingsIterator != info->info->structValue->end()) _arguments = settingsIterator->second->stringValue;

        settingsIterator = info->info->structValue->find("autostart");
        if(settingsIterator != info->info->structValue->end()) _autostart = settingsIterator->second->booleanValue;

        settingsIterator = info->info->structValue->find("collect-output");
        if(settingsIterator != info->info->structValue->end()) _collectOutput = settingsIterator->second->booleanValue;

		return true;
	}
	catch(const std::exception& ex)
	{
		_out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	return false;
}

bool Exec::start()
{
    try
    {
        _callbackHandlerId = BaseLib::ProcessManager::registerCallbackHandler(std::function<void(pid_t pid, int exitCode, int signal, bool coreDumped)>(std::bind(&Exec::sigchildHandler, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));

        if(_autostart) startProgram();

        return true;
    }
    catch(const std::exception& ex)
    {
        _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return false;
}

void Exec::stop()
{
    try
    {
        _autostart = false;
        if(_pid != -1) kill(_pid, 15);
    }
    catch(const std::exception& ex)
    {
        _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
}

void Exec::waitForStop()
{
    if(_pid != -1) kill(_pid, 15);
    for(int32_t i = 0; i < 600; i++)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if(_pid == -1) break;
    }
    if(_pid != -1)
    {
        _out->printError("Error: Process did not finish within 60 seconds. Killing it.");
        kill(_pid, 9);
        close(_stdIn);
        close(_stdOut);
        close(_stdErr);
        _stdIn = -1;
        _stdOut = -1;
        _stdErr = -1;
    }
    if(_execThread.joinable()) _execThread.join();
    if(_errorThread.joinable()) _errorThread.join();
    BaseLib::ProcessManager::unregisterCallbackHandler(_callbackHandlerId);
}

void Exec::input(const Flows::PNodeInfo info, uint32_t index, const Flows::PVariable message)
{
	try
	{
	    if(index == 0)
        {
	        if(_pid == -1) startProgram();
        }
        else if(index == 1)
        {
            if(_pid != -1) kill(_pid, message->structValue->at("payload")->integerValue64);
        }
        else if(index == 2)
        {
            if(_stdIn != -1)
            {
                size_t totalBytesWritten = 0;
                while(totalBytesWritten < message->structValue->at("payload")->stringValue.size())
                {
                    int bytesWritten = -1;
                    do
                    {
                        bytesWritten = write(_stdIn, message->structValue->at("payload")->stringValue.data(), message->structValue->at("payload")->stringValue.size());
                    } while(bytesWritten == -1 && (errno == EAGAIN || errno == EINTR));
                    if(bytesWritten <= 0)
                    {
                        _out->printWarning("Warning: Could not write to STDIN: " + std::string(strerror(errno)));
                        break;
                    }
                    totalBytesWritten += bytesWritten;
                }
            }
        }
	}
	catch(const std::exception& ex)
	{
		_out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
}

void Exec::startProgram()
{
    try
    {
        if(_filename.empty())
        {
            _out->printError("Error: filename is not set.");
            return;
        }

        {
            std::lock_guard<std::mutex> bufferGuard(_bufferMutex);
            _bufferOut.clear();
            _bufferErr.clear();
        }

        if(_execThread.joinable()) _execThread.join();
        if(_errorThread.joinable()) _errorThread.join();
        _execThread = std::thread(&Exec::execThread, this);
    }
    catch(const std::exception& ex)
    {
        _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
}

int32_t Exec::getMaxFd()
{
    struct rlimit limits{};
    if(getrlimit(RLIMIT_NOFILE, &limits) == -1 || limits.rlim_cur >= INT32_MAX)
    {
        return 1024;
    }
    return limits.rlim_cur;
}

void Exec::execThread()
{
    try
    {
        do
        {
            if(_stdErr != -1)
            {
                close(_stdErr);
                _stdErr = -1;
            }
            if(_errorThread.joinable()) _errorThread.join();

            int stdIn = -1;
            int stdOut = -1;
            int stdErr = -1;
            _pid = BaseLib::ProcessManager::systemp(_filename, BaseLib::ProcessManager::splitArguments(_arguments), getMaxFd(), stdIn, stdOut, stdErr);
            _stdIn = stdIn;
            _stdOut = stdOut;
            _stdErr = stdErr;

            _errorThread = std::thread(&Exec::errorThread, this);

            std::array<uint8_t, 4096> buffer{};
            std::string bufferOut;
            while(_stdOut != -1)
            {
                if(!_collectOutput) bufferOut.clear();
                auto bytesRead = 0;
                do
                {
                    bytesRead = read(_stdOut, buffer.data(), buffer.size());
                    if(bytesRead > 0)
                    {
                        if(_collectOutput)
                        {
                            std::lock_guard<std::mutex> bufferGuard(_bufferMutex);
                            _bufferOut.insert(_bufferOut.end(), buffer.begin(), buffer.begin() + bytesRead);
                        }
                        else bufferOut.insert(bufferOut.end(), buffer.begin(), buffer.begin() + bytesRead);
                    }
                } while(bytesRead > 0);

                if(!_collectOutput && !bufferOut.empty())
                {
                    auto outputVector = BaseLib::HelperFunctions::splitAll(bufferOut, '\n');

                    Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
                    Flows::PVariable outputArray = std::make_shared<Flows::Variable>(Flows::VariableType::tArray);
                    outputArray->arrayValue->reserve(outputVector.size());
                    for(int32_t i = 0; i < (signed)outputVector.size(); i++)
                    {
                        if(i == (signed)outputVector.size() - 1 && outputVector[i].empty()) continue;
                        outputArray->arrayValue->emplace_back(std::make_shared<Flows::Variable>(std::move(outputVector[i])));
                    }
                    message->structValue->emplace("payload", outputArray);
                    output(1, message);
                }
            }

            if(_collectOutput)
            {
                std::lock_guard<std::mutex> bufferGuard(_bufferMutex);
                auto outputVector = BaseLib::HelperFunctions::splitAll(_bufferOut, '\n');

                Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
                Flows::PVariable outputArray = std::make_shared<Flows::Variable>(Flows::VariableType::tArray);
                outputArray->arrayValue->reserve(outputVector.size());
                for(int32_t i = 0; i < (signed)outputVector.size(); i++)
                {
                    if(i == (signed)outputVector.size() - 1 && outputVector[i].empty()) continue;
                    outputArray->arrayValue->emplace_back(std::make_shared<Flows::Variable>(std::move(outputVector[i])));
                }
                message->structValue->emplace("payload", outputArray);
                output(1, message);
            }

            if(_autostart) std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        while(_autostart);
    }
    catch(const std::exception& ex)
    {
        _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
}

void Exec::errorThread()
{
    try
    {
        std::array<uint8_t, 4096> buffer{};
        std::string bufferErr;
        while(_stdErr != -1)
        {
            int32_t bytesRead = 0;
            if(!_collectOutput) bufferErr.clear();
            do
            {
                bytesRead = read(_stdErr, buffer.data(), buffer.size());
                if(bytesRead > 0)
                {
                    if(_collectOutput)
                    {
                        std::lock_guard<std::mutex> bufferGuard(_bufferMutex);
                        _bufferErr.insert(_bufferErr.end(), buffer.begin(), buffer.begin() + bytesRead);
                    }
                    else bufferErr.insert(bufferErr.end(), buffer.begin(), buffer.begin() + bytesRead);
                }
            } while(bytesRead > 0);

            if(!_collectOutput && !bufferErr.empty())
            {
                auto outputVector = BaseLib::HelperFunctions::splitAll(bufferErr, '\n');

                Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
                Flows::PVariable outputArray = std::make_shared<Flows::Variable>(Flows::VariableType::tArray);
                outputArray->arrayValue->reserve(outputVector.size());
                for(int32_t i = 0; i < (signed)outputVector.size(); i++)
                {
                    if(i == (signed)outputVector.size() - 1 && outputVector[i].empty()) continue;
                    outputArray->arrayValue->emplace_back(std::make_shared<Flows::Variable>(std::move(outputVector[i])));
                }
                message->structValue->emplace("payload", outputArray);
                output(2, message);
            }
        }

        if(_collectOutput)
        {
            std::lock_guard<std::mutex> bufferGuard(_bufferMutex);
            if(!_bufferErr.empty())
            {
                auto outputVector = BaseLib::HelperFunctions::splitAll(_bufferErr, '\n');

                Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
                Flows::PVariable outputArray = std::make_shared<Flows::Variable>(Flows::VariableType::tArray);
                outputArray->arrayValue->reserve(outputVector.size());
                for(int32_t i = 0; i < (signed)outputVector.size(); i++)
                {
                    if(i == (signed)outputVector.size() - 1 && outputVector[i].empty()) continue;
                    outputArray->arrayValue->emplace_back(std::make_shared<Flows::Variable>(std::move(outputVector[i])));
                }
                message->structValue->emplace("payload", outputArray);
                output(2, message);
            }
        }
    }
    catch(const std::exception& ex)
    {
        _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
}

void Exec::sigchildHandler(pid_t pid, int exitCode, int signal, bool coreDumped)
{
    try
    {
        if(pid == _pid)
        {
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
            if(_collectOutput)
            {
                std::lock_guard<std::mutex> bufferGuard(_bufferMutex);

                {
                    auto outputVector = BaseLib::HelperFunctions::splitAll(_bufferOut, '\n');

                    Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
                    Flows::PVariable outputArray = std::make_shared<Flows::Variable>(Flows::VariableType::tArray);
                    outputArray->arrayValue->reserve(outputVector.size());
                    for(int32_t i = 0; i < (signed)outputVector.size(); i++)
                    {
                        if(i == (signed)outputVector.size() - 1 && outputVector[i].empty()) continue;
                        outputArray->arrayValue->emplace_back(std::make_shared<Flows::Variable>(std::move(outputVector[i])));
                    }
                    message->structValue->emplace("stdout", outputArray);
                }

                if(!_bufferErr.empty())
                {
                    auto outputVector = BaseLib::HelperFunctions::splitAll(_bufferErr, '\n');

                    Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
                    Flows::PVariable outputArray = std::make_shared<Flows::Variable>(Flows::VariableType::tArray);
                    outputArray->arrayValue->reserve(outputVector.size());
                    for(int32_t i = 0; i < (signed)outputVector.size(); i++)
                    {
                        if(i == (signed)outputVector.size() - 1 && outputVector[i].empty()) continue;
                        outputArray->arrayValue->emplace_back(std::make_shared<Flows::Variable>(std::move(outputVector[i])));
                    }
                    message->structValue->emplace("stderr", outputArray);
                }
            }
            output(0, message);
        }
    }
    catch(const std::exception& ex)
    {
        _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
}

int32_t Exec::read(std::atomic_int& fd, uint8_t* buffer, int32_t bufferSize)
{
    if(fd == -1) return 0;

    timeval timeout{};
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;
    fd_set readFileDescriptor{};
    FD_ZERO(&readFileDescriptor);
    int32_t nfds = fd + 1;
    if(nfds <= 0)
    {
        close(fd);
        fd = -1;
        return -1;
    }
    FD_SET(fd, &readFileDescriptor);
    auto bytesRead = select(nfds, &readFileDescriptor, nullptr, nullptr, &timeout);
    if(bytesRead == 0)
    {
        return 0;
    }
    if(bytesRead != 1)
    {
        close(fd);
        fd = -1;
        return -1;
    }
    do
    {
        bytesRead = ::read(fd, buffer, bufferSize);
    } while(bytesRead < 0 && (errno == EAGAIN || errno == EINTR));
    if(bytesRead <= 0)
    {
        if(bytesRead == -1)
        {
            if(errno == ETIMEDOUT) return 0;
            else
            {
                close(fd);
                fd = -1;
                return -1;
            }
        }
        else
        {
            close(fd);
            fd = -1;
            return -1;
        }
    }
    if(bytesRead > bufferSize) bytesRead = bufferSize;
    return bytesRead;
}

}
