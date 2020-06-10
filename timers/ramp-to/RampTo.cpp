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

#include "RampTo.h"

namespace RampTo
{

RampTo::RampTo(std::string path, std::string nodeNamespace, std::string type, const std::atomic_bool* frontendConnected) : Flows::INode(path, nodeNamespace, type, frontendConnected)
{
	_stopThread = true;
}

RampTo::~RampTo()
{
	_stopThread = true;
	waitForStop();
}


bool RampTo::init(Flows::PNodeInfo info)
{
	try
	{
        auto settingsIterator = info->info->structValue->find("interval-up");
        if(settingsIterator != info->info->structValue->end() && !settingsIterator->second->stringValue.empty())
        {
            _intervalUpSet = true;
            _intervalUp = Flows::Math::getNumber(settingsIterator->second->stringValue);
        }

        settingsIterator = info->info->structValue->find("interval-down");
        if(settingsIterator != info->info->structValue->end() && !settingsIterator->second->stringValue.empty())
        {
            _intervalDownSet = true;
            _intervalDown = Flows::Math::getNumber(settingsIterator->second->stringValue);
        }

		settingsIterator = info->info->structValue->find("step-interval");
		if(settingsIterator != info->info->structValue->end()) _stepInterval = Flows::Math::getNumber(settingsIterator->second->stringValue);

        settingsIterator = info->info->structValue->find("minimum");
        if(settingsIterator != info->info->structValue->end()) _minimum = Flows::Math::getNumber(settingsIterator->second->stringValue);

        settingsIterator = info->info->structValue->find("maximum");
        if(settingsIterator != info->info->structValue->end()) _maximum = Flows::Math::getNumber(settingsIterator->second->stringValue);

        if(_minimum >= _maximum)
        {
            _minimum = 0;
            _maximum = 100;
        }

		if(_intervalUp < 1) _intervalUp = 1;
        if(_intervalDown < 1) _intervalDown = 1;
        if(_stepInterval < 1) _stepInterval = 1;

		return true;
	}
	catch(const std::exception& ex)
	{
		_out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
	return false;
}

bool RampTo::start()
{
    try
    {
        _currentValue = getNodeData("currentValue")->floatValue;

        return true;
    }
    catch(const std::exception& ex)
    {
        _out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    return false;
}

void RampTo::stop()
{
    _stopThread = true;
}

void RampTo::waitForStop()
{
	try
	{
		std::lock_guard<std::mutex> timerGuard(_timerMutex);
		_stopThread = true;
		if(_timerThread.joinable()) _timerThread.join();
	}
	catch(const std::exception& ex)
	{
		_out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
}

void RampTo::timer(double startValue, double rampTo, bool isInteger)
{
    int32_t sleepingTime = _stepInterval;
    int64_t startTime = Flows::HelperFunctions::getTime();
    if(rampTo > _maximum) rampTo = _maximum;
    else if(rampTo < _minimum) rampTo = _minimum;
    if(startValue > _maximum) startValue = _maximum;
    else if(startValue < _minimum) startValue = _minimum;
    if(startValue == rampTo) return;

    double interval = 0;
    if(rampTo > startValue) interval = (std::abs(rampTo - startValue) / (_maximum - _minimum)) * _intervalUp;
    else interval = (std::abs(rampTo - startValue) / (_maximum - _minimum)) * _intervalDown;
    if(sleepingTime > interval) sleepingTime = interval;
    uint32_t stepCount = std::lround(interval / sleepingTime);
    double stepSize = std::abs(rampTo - startValue) / (double)stepCount;
    if(stepSize == 0.0) stepSize = 1.0;
    bool up = rampTo > startValue;

    int64_t lastIntegerValue = std::llround(startValue);

	for(uint32_t i = 0; i < stepCount; i++)
	{
		try
		{
            if(_stopThread) break;
            if(sleepingTime > 1000 && sleepingTime < 30000)
            {
                int32_t iterations = sleepingTime / 100;
                for(int32_t j = 0; j < iterations; j++)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    if(_stopThread) break;
                }
                if(sleepingTime % 100) std::this_thread::sleep_for(std::chrono::milliseconds(sleepingTime % 100));
            }
            else if(sleepingTime >= 30000)
            {
                int32_t iterations = sleepingTime / 1000;
                for(int32_t j = 0; j < iterations; j++)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                    if(_stopThread) break;
                }
                if(sleepingTime % 1000) std::this_thread::sleep_for(std::chrono::milliseconds(sleepingTime % 1000));
            }
            else std::this_thread::sleep_for(std::chrono::milliseconds(sleepingTime));
			if(_stopThread) break;

            startValue = (up ? startValue + stepSize : startValue - stepSize);
            if(i == stepCount - 1) startValue = rampTo; //Prevent rounding issues
			Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
			if(isInteger)
            {
			    auto currentValue = (int64_t)std::llround(startValue);
			    if(currentValue != lastIntegerValue)
                {
                    message->structValue->emplace("payload", std::make_shared<Flows::Variable>(currentValue));
                    lastIntegerValue = currentValue;
                    output(0, message);
                }
            }
            else
            {
                message->structValue->emplace("payload", std::make_shared<Flows::Variable>(startValue));
                output(0, message);
            }

			int64_t diff = Flows::HelperFunctions::getTime() - startTime;
			if(diff <= _stepInterval) sleepingTime = _stepInterval;
			else sleepingTime = _stepInterval - (diff - _stepInterval);
			if(sleepingTime < 1) sleepingTime = 1;
			startTime = Flows::HelperFunctions::getTime();
		}
		catch(const std::exception& ex)
		{
			_out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
		}
	}

    Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
    message->structValue->emplace("payload", std::make_shared<Flows::Variable>(true));
    output(1, message);

    _currentValue = startValue;
    setNodeData("currentValue", std::make_shared<Flows::Variable>(_currentValue));
}

void RampTo::input(const Flows::PNodeInfo info, uint32_t index, const Flows::PVariable message)
{
	try
	{
		if(index == 0)
        {
		    if(!_intervalUpSet)
            {
                auto messageIterator = message->structValue->find("interval-up");
                if(messageIterator != message->structValue->end()) _intervalUp = messageIterator->second->integerValue;
                if(_intervalUp < 1) _intervalUp = 1;
            }
            if(!_intervalDownSet)
            {
                auto messageIterator = message->structValue->find("interval-down");
                if(messageIterator != message->structValue->end()) _intervalDown = messageIterator->second->integerValue;
                if(_intervalDown < 1) _intervalDown = 1;
            }

            std::lock_guard<std::mutex> timerGuard(_timerMutex);
            _stopThread = true;
            if(_timerThread.joinable()) _timerThread.join();

            auto rampToValue = message->structValue->at("payload")->floatValue;

            if(_intervalUp == 1 && _intervalDown == 1)
            {
                Flows::PVariable outputMessage = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
                if(message->structValue->at("payload")->type != Flows::VariableType::tFloat)
                {
                    outputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>((int64_t)std::llround(rampToValue)));
                }
                else
                {
                    outputMessage->structValue->emplace("payload", std::make_shared<Flows::Variable>(rampToValue));
                }
                output(0, outputMessage);
            }
            else
            {
                _stopThread = false;
                _timerThread = std::thread(&RampTo::timer, this, _currentValue.load(), rampToValue, message->structValue->at("payload")->type != Flows::VariableType::tFloat);
            }
		}
		else if(index == 1)
		{
		    _currentValue = message->structValue->at("payload")->floatValue;
		    setNodeData("currentValue", std::make_shared<Flows::Variable>(_currentValue));
		}
        else if(index == 2)
        {
            {
                std::lock_guard<std::mutex> timerGuard(_timerMutex);
                _stopThread = true;
                if(_timerThread.joinable()) _timerThread.join();
            }

            Flows::PVariable message = std::make_shared<Flows::Variable>(Flows::VariableType::tStruct);
            message->structValue->emplace("payload", std::make_shared<Flows::Variable>(true));
            output(1, message);
        }
	}
	catch(const std::exception& ex)
	{
		_out->printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
	}
}

}
