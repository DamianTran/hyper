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

#include "hyper/toolkit/json.hpp"
#include "hyper/toolkit/string.hpp"
#include "hyper/toolkit/filesystem.hpp"
#include "hyper/toolkit/charsurf.hpp"

#include <stdio.h>
#include <unistd.h>

using namespace std;

<<<<<<< HEAD
namespace hyperC{
=======
namespace hyper{
>>>>>>> a2aca39971b964f2fe0f560682673f2341f2e7a9

void get_JSON_name_type(const string& filename,
                        const string& var_name,
                        StringVector& output){

    if(access(filename.c_str(), F_OK)) return;

    FILE* inFILE = fopen(filename.c_str(), "rb");

    size_t fileSIZE = getFileSize(inFILE);
    char data[fileSIZE+1];
    fread(data, fileSIZE, sizeof(char), inFILE);

    unsigned int foundIndex = UINT_MAX, matchIndex = 0;

    for(size_t i = 0; i < fileSIZE; ++i){
        if(check_letter(data[i], var_name[matchIndex])){
            if(foundIndex == UINT_MAX){
                foundIndex = i;
            }

            ++matchIndex;

            if(matchIndex == var_name.size()){

                ++i;
                while((i < fileSIZE) && isCharType(data[i], " :\"")){
                    ++i;
                }
                foundIndex = i;
                while((i < fileSIZE) && !isCharType(data[i], "\"}")){
                    ++i;
                }

                output.emplace_back();
                output.back().assign(data + foundIndex, data + i);

                foundIndex = UINT_MAX;
                matchIndex = 0;
            }
        }
        else{
            matchIndex = 0;
            foundIndex = UINT_MAX;
        }
    }

    fclose(inFILE);

}

JSON_object::JSON_object():
    level(0){ }

JSON_object::JSON_object(const char* str, const int& level):
    level(level)
{
    parse(str);
}

JSON_object::JSON_object(const std::string& str, const int& level):
    level(level)
{
    parse(str.c_str());
}

void JSON_object::parse(const char* str)
{

    if(!(*str))
    {
        return;
    }

    const char* init_c = str;

    std::string buf;

    bool bProperty = true;

    while(*str)
    {

        if(*str == '[')
        {

            bProperty = false;

            buf = get_between('[', ']', str);
            if(!buf.empty())
            {
                const char* vector_c = buf.c_str();
                string sub_buf;

                while(*vector_c)
                {

                    if(*vector_c == '{')
                    {
                        sub_buf = get_between('{', '}', vector_c);
                        branches.emplace_back(sub_buf, level + 1);

                        vector_c += sub_buf.size() + 1;
                    }

                    ++vector_c;
                }

                value_type = JSON_type::Vector;

                str += buf.size() + 1;
            }

        }
        else if(*str == '\"')
        {
            string label = get_between('\"', '\"', str);
            bProperty = false;

            if(!label.empty())
            {

                string sub_buf;
                str += label.size() + 2;

                while(*str && isCharType(*str, " :")) ++str;

                if(*str == '\"')
                {
                    sub_buf = get_between('\"', '\"', str);
                    str += sub_buf.size() + 1;
                }
                else if(*str == '[')
                {
                    sub_buf = get_between('[', ']', str);
                    str += sub_buf.size() + 1;
                }
                else
                {
                    const char* c = str;
                    while(*str && !isCharType(*str, ",\n}"))
                    {
                        ++str;
                    }
                    sub_buf.assign(c, distance(c, str));
                }

                if(!sub_buf.empty())
                {
                    nodes.try_emplace(label, sub_buf, level + 1);
                }
            }

        }
        else if(*str == '{')
        {

            bProperty = false;

            buf = get_between('{', '}', str);

            if(!buf.empty())
            {

                branches.emplace_back(buf, level + 1);
                str += buf.size() + 1;

            }

        }
        ++str;

    }

    if(bProperty)
    {
        property = init_c;

        // Evaluate the type

        if(isNumeric(property))
        {
            value_type = JSON_type::Numeric;
        }
        else if(cmpStringToList(property, { "true", "false" }, CMP_STR_CASE_INSENSITIVE))
        {
            value_type = JSON_type::Logical;
        }
        else
        {
            value_type = JSON_type::String;
        }

    }

    return;
}

bool JSON_object::getObjects(const string& field,
                             vector<JSON_object>& output)
{

    bool bStatus = false;

    for(auto& pair : nodes)
    {
        if(pair.first == field)
        {
            output.emplace_back(pair.second);
            bStatus |= true;
        }
        else
        {
            bStatus |= pair.second.getObjects(field, output);
        }
    }

    for(auto& branch : branches)
    {
        bStatus |= branch.getObjects(field, output);
    }


    return bStatus;
}

bool JSON_object::getValues(const string& field,
                            vector<string>& output)
{

    bool bStatus = false;

    for(auto& pair : nodes)
    {
        if(pair.first == field)
        {
            output.emplace_back(pair.second.getProperty());
            bStatus |= true;
        }
        else
        {
            bStatus |= pair.second.getValues(field, output);
        }
    }

    for(auto& branch : branches)
    {
        bStatus |= branch.getValues(field, output);
    }

    return bStatus;

}

vector<string> JSON_object::getAttributes() const
{

    vector<string> output;
    for(auto& pair : nodes)
    {
        output.emplace_back(pair.first);
    }
    return output;
}

bool JSON_object::empty() const
{
    return nodes.empty() && branches.empty() && property.empty();
}

void JSON_object::clear()
{
    nodes.clear();
    branches.clear();
    property.clear();
}

bool JSON_object::load(const std::string& filename)
{

    if(access(filename.c_str(), F_OK))
    {
        return false;
    }

    FILE* inFILE = fopen(filename.c_str(), "rb");

    if(inFILE)
    {

        clear();

        size_t L = getFileSize(inFILE);

        char* data = new char[L + 1];
        data[L] = '\0';

        fread(data, L, sizeof(char), inFILE);

        parse(data);

        delete[] data;

        return true;

    }

    return false;
}

bool JSON_object::save(const std::string& filename) const
{

    ofstream outFILE(filename, ios::out | ios::binary);

    if(outFILE)
    {

        outFILE << *this;

        return true;
    }

    return false;
}

ostream& operator<<(ostream& output, const JSON_object& input)
{

    size_t i;
    size_t j;
    size_t L;

    if(!input.nodes.empty())
    {

        j = 0;
        L = input.nodes.size();

        for(auto& pair : input.nodes)
        {

            for(i = 0; i < input.level + 1; ++i)
            {
                output << '\t';
            }
            output << '\"' << pair.first << "\": " << pair.second;

            if(j < L - 1) output << ',';

            output << '\n';

            ++j;

        }

    }

    if(!input.branches.empty())
    {

        output << "[\n";

        j = 0;
        L = input.branches.size();

        for(auto& branch : input.branches)
        {

            for(i = 0; i < input.level + 1; ++i)
            {
                output << '\t';
            }
            output << "{\n";

            output << branch;

            for(i = 0; i < input.level + 1; ++i)
            {
                output << '\t';
            }
            output << '}';

            if(j < L - 1) output << ',';

            output << '\n';

            ++j;
        }

        for(i = 0; i < input.level; ++i)
        {
            output << '\t';
        }
        output << "]";
    }

    if(!input.property.empty())
    {
        switch(input.getType())
        {
        case JSON_type::Logical:
            {
                output << input.property;
                break;
            }
        case JSON_type::Numeric:
            {
                output << input.property;
                break;
            }
        default:
            {
                output << '\"' << input.property << '\"';
                break;
            }
        }
    }

    return output;

}

JSON_object& JSON_object::operator[](const std::string& field)
{
    return nodes.at(field);
}

const JSON_object& JSON_object::operator[](const std::string& field) const
{
    return nodes.at(field);
}

}
