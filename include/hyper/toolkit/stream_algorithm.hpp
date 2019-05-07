#pragma once

#ifndef STREAMALGORITHM_HPP
#define STREAMALGORITHM_HPP

#include "hyper/toolkit/data_stream.hpp"
#include "hyper/toolkit/string.hpp"

namespace hyperC
{

void getValues(std::vector<float>& output, const _1Dstream& stream);
void getTaggedValues(std::vector<float>& output, const _1Dstream& stream, const std::string& tag);
void getValidValues(std::vector<float>& output, const _1Dstream& stream);
float average(const _1Dstream& stream);
float median(const _1Dstream& stream);
float sum(const _1Dstream& stream);
float signal(const _1Dstream& stream);
float stdev(const _1Dstream& stream);

float stdev(const _2Dstream& stream, float* avg = nullptr);

inline bool isNumeric(const _1Dstream& stream){
    std::vector<float> f;
    getValidValues(f, stream);
    return f.size() > stream.size()/2;
}

std::vector<float> SigmoidRangeDist(const _1Dstream& stream);
std::vector<float> SigmoidDeviationDist(const _1Dstream& stream);

inline unsigned int numValid(const _1Dstream& stream){

    size_t L = stream.size();
    size_t S = 0;

    float test = 0.0f;

    for(size_t i = 0; i < L; ++i){
        test = stream(i);
        if(!isnan(test) && isinf(test)) ++S;
    }
    return S;

}

inline unsigned int numFilled(const _1Dstream& stream){
    size_t L = stream.size();
    unsigned int output = 0;
    std::string test;
    for(size_t i = 0; i < L; ++i){
        test = stream[i];
        for(auto& c : test){
            if(!hyperC::isCharType(c, " ,;_\"\\/")){
                ++output;
                break;
            }
        }
    }
    return output;
}

}

#endif // STREAMALGORITHM_HPP
