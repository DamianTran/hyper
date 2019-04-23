/** ////////////////////////////////////////////////////////////////

    *** EZ-C++ - A simplified C++ experience ***

        Yet (another) open source library for C++

        Original Copyright (C) Damian Tran 2019

        By aiFive Technologies, Inc. for developers

    Copying and redistribution of this code is freely permissible.
    Inclusion of the above notice is preferred but not required.

    This software is provided AS IS without any expressed or implied
    warranties.  By using this code, and any modifications and
    variants arising thereof, you are assuming all liabilities and
    risks that may be thus associated.

////////////////////////////////////////////////////////////////  **/

#pragma once

#ifndef EZ_VARIABLE
#define EZ_VARIABLE

#include <unistd.h>
#include <cstdlib>

#include "EZC/algorithm.hpp"

using namespace EZC;

template<class T1, class T2>
class VariableMonitor{
protected:
    BYTE monitorState; // State of monitoring of response
    std::thread* monitorThread;
    double updateLatency,
            spikeMagnitude,
            spikeFreq,
            spikeDuration,
            spikeLatency,
            currentSpikeAmplitude,
            lastSpikeAmplitude;

    unsigned int recordLength;
    std::vector<float> variableHistory;

    bool monitoring,
        bSpike;

    vMatrix<float>* recordOutput;

    T1* variable;
    const T2* response;

    T1 lastVariableValue,
        variableIncrement,
        lastVariableIncrement,
        rangeMin, rangeMax;

    T2 lastResponseValue,
        lastResponseRate;

public:

    inline void resetVariableStates(){
        if(variable != nullptr){
            lastVariableValue = variable;
            variableIncrement = *variable*0.001;
        }
        else{
            lastVariableValue = 0.0;
            variableIncrement = 0.0;
        }

        if(response != nullptr){
            lastResponseValue = *response;
        }
        else{
            lastResponseValue = 0.0;
        }

        lastResponseRate = 0.0;
    }

    inline void setMonitorState(const BYTE& newState){ monitorState = newState; }
    inline void setMonitor(T1& variable, const T2& response, const BYTE& monitorState){
        this->variable = &variable;
        this->response = &response;
        this->monitorState = monitorState;
        resetVariableStates();
    }

    inline void setMonitorVariable(T1& newVariable){
        this->variable = &newVariable;
        resetVariableStates();
    }
    inline void setMonitorVariableRange(const T1& newMin, const T1& newMax);
    inline void setResponseVariable(T2& newResponse){
        this->response = &newResponse;
        resetVariableStates();
    }

    inline void setVariableIncrement(const T1& newIncrement){ variableIncrement = newIncrement; }
    inline void setRecordLength(const unsigned int& newLength){
        variableHistory.resize(newLength, NAN);
        recordLength = newLength;
    }

    inline void setSpike(const bool& state, const float& magnitude = 100.0,
                         const float& frequency = 0.02, const float& duration = 10.0,
                         const float& latency = 0.0){
        if(state){

            bSpike = true;
            spikeMagnitude = magnitude;
            spikeFreq = frequency;
            spikeDuration = duration;
            spikeLatency = latency;

        }
        else{

            bSpike = false;

        }
    }

    void update(){

        for(int i = recordLength - 1; i > 0; --i){
            variableHistory[i] = variableHistory[i - 1];
        }
        variableHistory[0] = *variable;

        if(bSpike){

            if(currentSpikeAmplitude <= 0){

                bool rand_draw = draw(spikeFreq);

                if(rand_draw){
                    lastSpikeAmplitude = stdev(variableHistory) * spikeMagnitude;
                    currentSpikeAmplitude = lastSpikeAmplitude;

                    if((*variable - lastSpikeAmplitude > rangeMin) && (*variable + lastSpikeAmplitude > rangeMin) &&
                       (*variable - lastSpikeAmplitude < rangeMax) && (*variable + lastSpikeAmplitude < rangeMax)){

                        *variable += lastSpikeAmplitude;

                    }
                    else if((*variable - lastSpikeAmplitude < rangeMin) || (*variable + lastSpikeAmplitude < rangeMin)){

                        *variable = rangeMin;

                    }
                    else if((*variable - lastSpikeAmplitude > rangeMax) || (*variable + lastSpikeAmplitude > rangeMax)){

                        *variable = rangeMax;

                    }
                }

            } else {

                double decay = lastSpikeAmplitude/spikeDuration * updateLatency;
                currentSpikeAmplitude -= decay;
                if((*variable - decay > rangeMin) && (*variable - decay < rangeMax)){

                    *variable -= decay;

                }

                return;

            }

        }

        float responseRate = *response - lastResponseValue;

        if(responseRate == 0.0){ // No change in response
            lastResponseRate = 0.0;
            return;
        }

        if(*variable > rangeMax) *variable = rangeMax;
        else if(*variable < rangeMin) *variable = rangeMin;

        if(monitorState & MONITOR_STATE_MAX){
            if(*variable > lastVariableValue){
                lastVariableValue = *variable;
                if(responseRate > lastResponseRate){
                    if(variableIncrement > lastVariableIncrement){ // Update increment by 1% if different than last
                        lastVariableIncrement = variableIncrement;
                        variableIncrement += variableIncrement*abs(responseRate)/(*response);
                    }
                    else if(variableIncrement < lastVariableIncrement){
                        lastVariableIncrement = variableIncrement;
                        variableIncrement -= variableIncrement*abs(responseRate)/(*response);
                    }
                    else if(monitorState & MONITOR_STATE_ACTIVE){
                        lastVariableIncrement = variableIncrement;
                        if(rand(0.0, 1.0) > 0.5){
                            variableIncrement += variableIncrement*abs(responseRate)/(*response);
                        }
                        else{
                            variableIncrement -= variableIncrement*abs(responseRate)/(*response);
                        }
                    }

                    if((*variable + variableIncrement < rangeMax) &&
                       (*variable + variableIncrement > rangeMin)) *variable += variableIncrement;
                }
                else if(responseRate < lastResponseRate){
                    if(variableIncrement < lastVariableIncrement){
                        lastVariableIncrement = variableIncrement;
                        variableIncrement += variableIncrement*abs(responseRate)/(*response);
                    }
                    else if(variableIncrement > lastVariableIncrement){
                        lastVariableIncrement = variableIncrement;
                        variableIncrement -= variableIncrement*abs(responseRate)/(*response);
                    }
                    else if(monitorState & MONITOR_STATE_ACTIVE){
                        lastVariableIncrement = variableIncrement;
                        if(rand(0.0, 1.0) > 0.5){
                            variableIncrement += variableIncrement*abs(responseRate)/(*response);
                        }
                        else{
                            variableIncrement -= variableIncrement*abs(responseRate)/(*response);
                        }
                    }
                    if((*variable - variableIncrement < rangeMax) &&
                       (*variable - variableIncrement > rangeMin)) *variable -= variableIncrement;
                }
            }
            else if(*variable < lastVariableValue){
                lastVariableValue = *variable;
                if(responseRate < lastResponseRate){
                    if(variableIncrement > lastVariableIncrement){
                        lastVariableIncrement = variableIncrement;
                        variableIncrement += variableIncrement*abs(responseRate)/(*response);
                    }
                    else if(variableIncrement < lastVariableIncrement){
                        lastVariableIncrement = variableIncrement;
                        variableIncrement -= variableIncrement*abs(responseRate)/(*response);
                    }
                    else if(monitorState & MONITOR_STATE_ACTIVE){
                        lastVariableIncrement = variableIncrement;
                        if(rand(0.0, 1.0) > 0.5){
                            variableIncrement += variableIncrement*abs(responseRate)/(*response);
                        }
                        else{
                            variableIncrement -= variableIncrement*abs(responseRate)/(*response);
                        }
                    }
                    if((*variable + variableIncrement < rangeMax) &&
                       (*variable + variableIncrement > rangeMin)) *variable += variableIncrement;
                }
                else if(responseRate > lastResponseRate){
                    if(variableIncrement < lastVariableIncrement){
                        lastVariableIncrement = variableIncrement;
                        variableIncrement += variableIncrement*abs(responseRate)/(*response);
                    }
                    else if(variableIncrement > lastVariableIncrement){
                        lastVariableIncrement = variableIncrement;
                        variableIncrement -= variableIncrement*abs(responseRate)/(*response);
                    }
                    else if(monitorState & MONITOR_STATE_ACTIVE){
                        lastVariableIncrement = variableIncrement;
                        if(rand(0.0, 1.0) > 0.5){
                            variableIncrement += variableIncrement*abs(responseRate)/(*response);
                        }
                        else{
                            variableIncrement -= variableIncrement*abs(responseRate)/(*response);
                        }
                    }
                    if((*variable - variableIncrement < rangeMax) &&
                       (*variable - variableIncrement > rangeMin)) *variable -= variableIncrement;
                }
            }
            else if(monitorState & MONITOR_STATE_ACTIVE){ // Randomly peturb to prevent stagnancy
                lastVariableValue = *variable;
                if(rand(0.0, 1.0) > 0.5){
                    if((*variable + variableIncrement < rangeMax) &&
                       (*variable + variableIncrement > rangeMin)) *variable += variableIncrement;
                }
                else{
                    if((*variable - variableIncrement < rangeMax) &&
                       (*variable - variableIncrement > rangeMin)) *variable -= variableIncrement;
                }

            }
        }
        if(monitorState & MONITOR_STATE_MIN){
            if(*variable < lastVariableValue){
                lastVariableValue = *variable;
                if(responseRate > lastResponseRate){
                    if(variableIncrement > lastVariableIncrement){
                        lastVariableIncrement = variableIncrement;
                        variableIncrement += variableIncrement*abs(responseRate)/(*response);
                    }
                    else if(variableIncrement < lastVariableIncrement){
                        lastVariableIncrement = variableIncrement;
                        variableIncrement -= variableIncrement*abs(responseRate)/(*response);
                    }
                    else if(monitorState & MONITOR_STATE_ACTIVE){
                        lastVariableIncrement = variableIncrement;
                        if(rand(0.0, 1.0) > 0.5){
                            variableIncrement += variableIncrement*abs(responseRate)/(*response);
                        }
                        else{
                            variableIncrement -= variableIncrement*abs(responseRate)/(*response);
                        }
                    }
                    if((*variable + variableIncrement < rangeMax) &&
                       (*variable + variableIncrement > rangeMin)) *variable += variableIncrement;
                }
                else if(responseRate < lastResponseRate){
                    if(variableIncrement < lastVariableIncrement){
                        lastVariableIncrement = variableIncrement;
                        variableIncrement += variableIncrement*abs(responseRate)/(*response);
                    }
                    else if(variableIncrement > lastVariableIncrement){
                        lastVariableIncrement = variableIncrement;
                        variableIncrement -= variableIncrement*abs(responseRate)/(*response);
                    }
                    else if(monitorState & MONITOR_STATE_ACTIVE){
                        lastVariableIncrement = variableIncrement;
                        if(rand(0.0, 1.0) > 0.5){
                            variableIncrement += variableIncrement*abs(responseRate)/(*response);
                        }
                        else{
                            variableIncrement -= variableIncrement*abs(responseRate)/(*response);
                        }
                    }
                    if((*variable - variableIncrement < rangeMax) &&
                       (*variable - variableIncrement > rangeMin)) *variable -= variableIncrement;
                }
            }
            else if(*variable > lastVariableValue){
                lastVariableValue = *variable;
                if(responseRate < lastResponseRate){
                    if(variableIncrement > lastVariableIncrement){
                        lastVariableIncrement = variableIncrement;
                        variableIncrement += variableIncrement*abs(responseRate)/(*response);
                    }
                    else if(variableIncrement < lastVariableIncrement){
                        lastVariableIncrement = variableIncrement;
                        variableIncrement -= variableIncrement*abs(responseRate)/(*response);
                    }
                    else if(monitorState & MONITOR_STATE_ACTIVE){
                        lastVariableIncrement = variableIncrement;
                        if(rand(0.0, 1.0) > 0.5){
                            variableIncrement += variableIncrement*abs(responseRate)/(*response);
                        }
                        else{
                            variableIncrement -= variableIncrement*abs(responseRate)/(*response);
                        }
                    }
                    if((*variable + variableIncrement < rangeMax) &&
                       (*variable + variableIncrement > rangeMin)) *variable += variableIncrement;
                }
                else if(responseRate > lastResponseRate){
                    if(variableIncrement < lastVariableIncrement){
                        lastVariableIncrement = variableIncrement;
                        variableIncrement += variableIncrement*abs(responseRate)/(*response);
                    }
                    else if(variableIncrement > lastVariableIncrement){
                        lastVariableIncrement = variableIncrement;
                        variableIncrement -= variableIncrement*abs(responseRate)/(*response);
                    }
                    else if(monitorState & MONITOR_STATE_ACTIVE){
                        lastVariableIncrement = variableIncrement;
                        if(rand(0.0, 1.0) > 0.5){
                            variableIncrement += variableIncrement*abs(responseRate)/(*response);
                        }
                        else{
                            variableIncrement -= variableIncrement*abs(responseRate)/(*response);
                        }
                    }
                    if((*variable - variableIncrement < rangeMax) &&
                       (*variable - variableIncrement > rangeMin)) *variable -= variableIncrement;
                }
            }
            else if(monitorState & MONITOR_STATE_ACTIVE){
                lastVariableValue = *variable;
                if(rand(0.0, 1.0) > 0.5){
                    if((*variable + variableIncrement < rangeMax) &&
                       (*variable + variableIncrement > rangeMin)) *variable += variableIncrement;
                }
                else{
                    if((*variable - variableIncrement < rangeMax) &&
                       (*variable - variableIncrement > rangeMin)) *variable -= variableIncrement;
                }
            }
        }

        if(*variable > rangeMax) *variable = rangeMax;
        else if(*variable < rangeMin) *variable = rangeMin;

        lastResponseValue = *response;
        lastResponseRate = responseRate;

    }

    bool beginMonitor(){
        if((monitorThread == nullptr)
           && (variable != nullptr)
            && (response != nullptr)){
                monitoring = true;
                monitorThread = new std::thread([&](){
                    std::chrono::duration<float> latency(updateLatency);
                    while(monitoring){
                        update();
                        std::this_thread::sleep_for(latency);
                    }
                });
                return true;
        }
        else{
            if(monitorState & MONITOR_STATE_VERBOSE)
                std::cout << ">> Could not begin variable monitoring\n";
            monitoring = false;
            return false;
        }
    }

    bool stopMonitor(){
        if(monitorThread != nullptr){
            monitorThread->join();
            if(monitorState & MONITOR_STATE_VERBOSE)
                std::cout << ">> Variable monitor stopped\n";
            monitoring = false;
            monitorThread = nullptr;
            return true;
        }
        else{
            if(monitorState & MONITOR_STATE_VERBOSE)
                std::cout << ">> Variable monitor is not active\n";
            monitoring = false;
            return false;
        }
    }

    VariableMonitor():
        monitorState(MONITOR_STATE_NONE),
        monitorThread(nullptr),
        updateLatency(1.0),
        spikeMagnitude(25.0),
        spikeFreq(0.01),
        spikeDuration(10.0),
        spikeLatency(0.0),
        currentSpikeAmplitude(0.0),
        lastSpikeAmplitude(0.0),
        recordLength(128),
        variableHistory(recordLength,NAN),
        monitoring(false),
        bSpike(false),
        variable(nullptr),
        response(nullptr),
        lastVariableValue(0),
        variableIncrement(0),
        lastVariableIncrement(0),
        lastResponseValue(0),
        lastResponseRate(0),
        rangeMin(0),
        rangeMax(0)
        { }
    VariableMonitor(T1& variable, const T2& response,
                    const T1& rangeMin = -INFINITY, const T1& rangeMax = INFINITY,
                    const BYTE& monitorState = MONITOR_STATE_NONE,
                        const float& updateLatency = 1.0):
        monitorState(monitorState),
        monitorThread(nullptr),
        updateLatency(updateLatency),
        spikeMagnitude(25.0),
        spikeFreq(0.01),
        spikeDuration(10.0),
        spikeLatency(0.0),
        currentSpikeAmplitude(0.0),
        lastSpikeAmplitude(0.0),
        recordLength(128),
        variableHistory(recordLength,NAN),
        monitoring(false),
        bSpike(false),
        variable(&variable),
        response(&response),
        lastVariableValue(variable),
        variableIncrement(abs(0.001f*variable)),
        lastVariableIncrement(this->variableIncrement),
        lastResponseValue(response),
        lastResponseRate(0),
        rangeMin(rangeMin),
        rangeMax(rangeMax){ }

    ~VariableMonitor(){ // Force stop
        monitoring = false;
        if(monitorThread != nullptr) monitorThread->join();
    }

};

#endif // EZ_VARIABLE
