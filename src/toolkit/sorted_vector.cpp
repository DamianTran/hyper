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

#include "EZC/toolkit/sorted_vector.hpp"
#include "EZC/toolkit/string.hpp"

#include <boost/range/adaptors.hpp>

#include "EZC/algorithm.hpp"

using namespace EZC;

void SortedStringVector::update_map(){

    size_t idx = 0;
    char last_c;

    for(auto& str : *this){

        if(!str.empty()){

            if(str.front() != last_c){

                search_key[str.front()] = idx;
                last_c = str.front();

            }

        }

        ++idx;

    }

}

void SortedStringVector::get_map_idx(const char& c, size_t& start, size_t& end){

    for(auto& key_pair : boost::adaptors::reverse(search_key)){

        if(key_pair.first < c){
            start = key_pair.second;
            break;
        }

    }

    for(auto& key_pair : search_key){

        if(key_pair.first > c){

            end = key_pair.second;
            break;

        }

    }

}

unsigned int SortedStringVector::match(const std::string& search_query){

    if(search_query.empty()) return UINT_MAX;

    size_t start_idx = 0, end_idx = size();

    get_map_idx(search_query.front(), start_idx, end_idx);

    while(start_idx != end_idx){

        if((*this)[start_idx] == search_query) return start_idx;

        ++start_idx;

    }

    return UINT_MAX;

}

bool SortedStringVector::anyEqual(const std::string& search_query){

    if(search_query.empty()) return false;

    size_t start_idx = 0, end_idx = size();

    get_map_idx(search_query.front(), start_idx, end_idx);

    while(start_idx != end_idx){

        if((*this)[start_idx] == search_query) return true;

        ++start_idx;

    }

    return false;

}

std::string& SortedStringVector::operator[](const std::string& search_query){

    if(search_query.empty()) throw std::invalid_argument("Search query is empty");

    size_t start_idx = 0, end_idx = size();

    get_map_idx(search_query.front(), start_idx, end_idx);

    while(start_idx != end_idx){

        if(cmpString((*this)[start_idx], search_query, CMP_STR_CASE_INSENSITIVE)) return (*this)[start_idx];

        ++start_idx;

    }

    if(isUpperCase(search_query.front())){

        start_idx = 0;
        end_idx = size();

        get_map_idx(search_query.front() + 32, start_idx, end_idx);

        while(start_idx != end_idx){

            if(cmpString((*this)[start_idx], search_query, CMP_STR_CASE_INSENSITIVE)) return (*this)[start_idx];

            ++start_idx;

        }

    }
    else if(isLowerCase(search_query.front())){

        start_idx = 0;
        end_idx = size();

        get_map_idx(search_query.front(), start_idx, end_idx);

        while(start_idx != end_idx){

            if(cmpString((*this)[start_idx], search_query, CMP_STR_CASE_INSENSITIVE)) return (*this)[start_idx];

            ++start_idx;

        }

    }

    throw std::invalid_argument("Search query does not exist in sorted vector");

}
