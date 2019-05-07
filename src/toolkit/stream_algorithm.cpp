#include "hyper/toolkit/stream_algorithm.hpp"

namespace hyperC
{

void getValues(std::vector<float>& output, const _1Dstream& stream){
    size_t L = stream.size();
    output.reserve(output.size() + L);
    for(size_t i = 0; i < L; ++i){
        output.push_back(stream(i));
    }
}

void getTaggedValues(std::vector<float>& output, const _1Dstream& stream, const std::string& tag){
    size_t L = stream.size();
    output.reserve(output.size() + L);
    for(size_t i = 0; i < L; ++i){
        std::string s = stream.getString(i);
        unsigned int fIndex = findString(s, tag);
        if(fIndex != UINT_MAX){
            unsigned int startPos = UINT_MAX,
                        endPos = 0;
            for(size_t j = fIndex; j < s.size(); ++j){
                if((s[j] == '=') || (s[j] == ':')){
                    startPos = j+1;
                }
                else if(startPos != UINT_MAX){
                    if(((s[j] == ';') || (s[j] == '\t') || (s[j] == '\n'))
                        || ((s[j] != ' ') && !isNumber(s[j]) && (s[j] != '.') && (s[j] != 'E'))){
                        endPos = j;
                        break;
                    }
                }
            }
            if((startPos != UINT_MAX) && (endPos != 0)){
                std::string sub; sub.assign(s, startPos, endPos - startPos);
                float f;
                try{
                    f = std::stof(sub);
                    output.push_back(f);
                }catch(...){
                    output.push_back(NAN);
                }
            }
        }
        else output.push_back(NAN);
    }
}

void getValidValues(std::vector<float>& output, const _1Dstream& stream){
    size_t L = stream.size();
    output.reserve(output.size() + L);
    float f;
    for(size_t i = 0; i < L; ++i){
        f = stream(i);
        if(!isnan(f)) output.push_back(f);
    }
}

float average(const _1Dstream& stream){
    float output = 0.0f, f(0.0f);
    size_t L = stream.size(), N = 0;
    if(L < 1) return NAN;
    for(size_t i = 0; i < L; ++i){
        f = stream(i);
        if(!isnan(f)){
            output += f;
            ++N;
        }
    }
    return output/N;
}

float sum(const _1Dstream& stream){
    float output = 0.0f, f(0.0f);
    size_t L = stream.size();
    if(L < 1) return NAN;
    for(size_t i = 0; i < L; ++i){
        f = stream(i);
        if(!isnan(f)) output += f;
    }
    return output;
}

float signal(const _1Dstream& stream){
    float avg = 0.0f, SD = 0.0f, f;
    size_t L = stream.size(), N = 0;
    if(L < 1) return NAN;
    if(L == 1) return stream(0);
    for(size_t i = 0; i < L; ++i){
        f = stream(i);
        if(!isnan(f)){
            avg += f;
            ++N;
        }
    }
    avg /= N;
    for(size_t i = 0; i < L; ++i){
        f = stream(i);
        if(!isnan(f)) SD += pow(f-avg, 2);
    }
    SD /= N;
    return avg/sqrt(SD);
}

float median(const _1Dstream& stream){
    size_t L = stream.size();
    if(L < 1) return NAN;

    std::vector<float> values; getValidValues(values, stream);

    float tmp = 0.0f;
    for(size_t i = 0; i < L-1; ++i){
        for(size_t j = i+1; j < L; ++j){
            if(values[j] < values[i]){
                tmp = values[i];
                values[i] = values[j];
                values[j] = tmp;
            }
        }
    }

    if((L % 2) == 0) return (values[L/2] + values[L/2 - 1])/2;
    else return values[L/2 - 1];
}

float stdev(const _1Dstream& stream){
    size_t L = stream.size();
    if(L < 1) return NAN;
    if(L == 1) return 0.0f;

    float output = 0.0f, avg = average(stream), f;
    unsigned int N = 0;
    for(size_t i = 0; i < L; ++i){
        f = stream(i);
        if(!isnan(f)){
            output += pow(f-avg, 2);
            ++N;
        }
    }

    return sqrt(output/N);
}

std::vector<float> SigmoidDeviationDist(const _1Dstream& stream){
    std::vector<float> values; getValues(values, stream);
    size_t L = values.size(), cIndex = 1;
    if(L < 2) return std::vector<float>(L, NAN);

    float valMean = 0.0f, valMax = values[0], valMin = values[0];

    while(isnan(valMax)){
        valMax = values[cIndex];
        ++cIndex;
    } cIndex = 1;
    while(isnan(valMin)){
        valMin = values[cIndex];
        ++cIndex;
    } cIndex = 0;

    for(size_t i = 0; i < L; ++i){
        if(!isnan(values[i])){
            valMean += values[i];
            if(values[i] > valMax) valMax = values[i];
            else if(values[i] < valMin) valMin = values[i];
            ++cIndex;
        }
    }
    valMean /= cIndex;

    for(size_t i = 0; i < L; ++i){ // Re-interpret values to standard deviation
        values[i] = pow(values[i] - valMean, 2)/cIndex;
    }

    float rG = 5.0f/absolute(valMax - valMin); // Fit to sigmoid
    for(size_t i = 0; i < L; ++i){
        values[i] = 2.0f/(1.0f + exp(-rG*values[i])) - 1.0f;
    }

    return values;

}

std::vector<float> SigmoidRangeDist(const _1Dstream& stream){
    std::vector<float> values; getValues(values, stream);
    size_t L = values.size();
    if(L < 2) return std::vector<float>(L, NAN);

    float valMean = 0.0f, valMax = values[0], valMin = values[0];
    unsigned int cIndex = 1;

    while(isnan(valMax)){
        valMax = values[cIndex];
        ++cIndex;
    } cIndex = 1;
    while(isnan(valMin)){
        valMin = values[cIndex];
        ++cIndex;
    }

    cIndex = 0;

    for(size_t i = 0; i < L; ++i){
        if(!isnan(values[i])){
            valMean += values[i];
            if(values[i] > valMax) valMax = values[i];
            else if(values[i] < valMin) valMin = values[i];
            ++cIndex;
        }
    }
    valMean /= cIndex;

    for(size_t i = 0; i < L; ++i){ // Redistribute around mean
        values[i] -= valMean;
    }

    float rG = 5.0f/absolute(valMax - valMin); // Fit to sigmoid
    for(size_t i = 0; i < L; ++i){
        values[i] = 2.0f/(1.0f + exp(-rG*values[i])) - 1.0f;
    }

    return values;

}

float stdev(const _2Dstream& stream, float* avg){
    float stream_avg = 0.0f, output = 0.0f, f;
    unsigned int N = 0;

    for(size_t y = 0; y < stream.nrow(); ++y){
        for(size_t x = 0; x < stream.rowSize(y); ++x){
            f = stream.getFloat(x, y);
            if(!isnan(f)){
                stream_avg += f;
                ++N;
            }
        }
    }

    stream_avg /= N;
    if(avg != nullptr) *avg = stream_avg;

    for(size_t y = 0; y < stream.nrow(); ++y){
        for(size_t x = 0; x < stream.rowSize(y); ++x){
            f = stream.getFloat(x, y);
            if(!isnan(f)){
                output += pow(f-stream_avg, 2);
            }
        }
    }

    return sqrt(output/N);

}

}
