/** ////////////////////////////////////////////////////////////////

    *** Hyper C++ - A simplified C++ experience ***

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

#ifndef TOOLKIT_SORTED_VECTOR
#define TOOLKIT_SORTED_VECTOR

#include <vector>
#include <string>
#include <unistd.h>
#include <cstdlib>
#include <map>

template<typename key_t, typename map_t>
class SortedVector : public std::vector<key_t>{
protected:

    std::map<map_t, size_t> search_key;

    virtual void update_map() = 0;

public:

    template<typename other_t>
    SortedVector<key_t, map_t>& operator=(const std::vector<other_t>& V){

        this->clear();

        if(!V.empty()){

            size_t L = V.size();
            this->reserve(L);

            auto it = this->begin(), it_end = this->end();

            for(size_t i = 0; i < L; ++i){

                while(it != it_end){

                    if(*it > V[i]){

                        this->insert(it, V[i]);

                        goto next;

                    }

                    ++it;

                }

                this->insert(it_end, V[i]);

                next:;

                it = this->begin();
                it_end = this->end();
            }
        }

        update_map();

        return *this;

    }

    template<typename other_t>
    void add(const other_t& other){

        auto it = this->begin(), it_end = this->end();

        while(it != it_end){

            if(*it > other){

                this->insert(it, other);

                return;

            }

            ++it;

        }

        insert(it_end, other);

        update_map();

    }

    template<typename other_t>
    void add(const std::vector<other_t>& other_v){

        if(!other_v.empty()){

            size_t L = other_v.size();
            this->reserve(this->size() + L);

            auto it = this->begin(), it_end = this->end();

            for(size_t i = 0; i < L; ++i){

                while(it != it_end){

                    if(*it > other_v[i]){

                        this->insert(it, other_v[i]);

                        goto next;

                    }

                    ++it;

                }

                this->insert(it_end, other_v[i]);

                next:;

                it = this->begin();
                it_end = this->end();
            }

            update_map();
        }

    }

    template<typename other_t>
    SortedVector(const std::vector<other_t>& V){

        *this = V;

    }

    SortedVector(){ }


};

class SortedStringVector : public SortedVector<std::string, char>{
protected:

    void update_map();

    void get_map_idx(const char& c, size_t& begin_idx, size_t& end_idx);

public:

    std::string& operator[](const std::string& search_query);
    inline std::string& operator[](const size_t& idx){
        return std::vector<std::string>::operator[](idx);
    }

    unsigned int match(const std::string& search_query);
    bool anyEqual(const std::string& search_query);

    template<typename other_t>
    SortedStringVector(const std::vector<other_t>& V){

        SortedVector<std::string, char>::operator=(V);

    }

};

#endif // TOOLKIT_SORTED_VECTOR
