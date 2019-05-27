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

#ifndef TOOLKIT_CLUSTERED_VECTOR
#define TOOLKIT_CLUSTERED_VECTOR

#include <vector>
#include <map>
#include <utility>

#include "hyper/toolkit/reference_vector.hpp"
#include "hyper/algorithm.hpp"

/////////////////////////////////////////////////////////////////////////////

/* Clustered value map by first index element */

/////////////////////////////////////////////////////////////////////////////

namespace hyperC
{

template<class key_t, class container_t, typename value_type>
class clustered_vector : public std::map<key_t, container_t>
{
public:

    clustered_vector() = default;
    clustered_vector(const container_t& V)
    {
        assemble(V);
    }

    virtual void assemble(container_t& V) = 0;

    template<typename key_vector_t>
    bool get_references(const key_vector_t& keys,
                         reference_vector<value_type>& output)
    {
        output.clear();
        size_t reserve_size = 0;

        for(auto& key : keys)
        {
            try{
                reserve_size += this->at(key).size();
            }catch(...){ }
        }

        output.reserve(reserve_size);

        for(auto& key : keys)
        {
            try
            {
                output.append(this->at(key));
            }catch(...){ }
        }

        return !output.empty();
    }

    template<typename key_vector_t>
    bool get_values(const key_vector_t& keys,
                    std::vector<value_type>& output) const
    {
        output.clear();

        for(auto& pair : *this)
        {
            if(anyEqual(pair.first, keys))
            {
                append(output, pair.second);
            }
        }

        return !output.empty();
    }

};

/////////////////////////////////////////////////////////////////////////////

/*

        Hierarchical cluster vector

        Elements are sorted based on the value of each element on
        index = level

        ie. Words are sorted on level 0 by the character at index word[0],
        level 1 by value of the character at word[1], etc.

        This vector is particularly useful for reducing the time
        complexity of large index searches (ie. conversion, word
        translation, filesystem searches)

*/

/////////////////////////////////////////////////////////////////////////////

template<class key_t, typename value_type,
            class container_t = reference_vector<value_type>>
class tree_vector : public std::map<key_t, tree_vector<key_t, value_type, container_t>>
{
public:

    tree_vector():
        key(0),
        level(0){ }

    tree_vector(key_t key,
                const int& max_level = -1,
                const int& level = 0):
        key(key),
        level(level),
        max_level(max_level){ }

    tree_vector(container_t& values,
                const int& max_level = -1,
                const int& level = 0):
        key(0),
        level(level),
        max_level(max_level)
    {
        assemble(values);
    }

    inline value_type& getValue(const size_t& index)
    {
        return vals[index];
    }

    const value_type& getValue(const size_t& index) const
    {
        return vals[index];
    }

    inline void addValue(value_type& newVal)
    {

        for(size_t i = 0; i < vals.size(); ++i)
        {
            if(newVal < vals[i])
            {
                vals.insert(vals.begin() + i, newVal);
                goto inserted;
            }
        }

        vals.emplace_back(newVal);

        inserted:;
    }

    inline const container_t& values() const
    {
        return vals;
    }

    inline container_t& values()
    {
        return vals;
    }

    inline void add_tree(value_type& value)
    {

        if(value.empty()) return;

        if(level &&
           (((value.size() == level) &&
           (value[level - 1] == key)) ||
           ((level == max_level) &&
            value[level - 1] == key)))
        {
            addValue(value);
        }
        else
        {
            try{
                this->at(value[level]).add_tree(value);
            }
            catch(...)
            {
                this->try_emplace(value[level], value[level], max_level, level + 1);
                this->at(value[level]).add_tree(value);
            }

        }
    }

    template<typename search_t>
    bool search_level(const search_t& val, int check_level = -1)
    {
        if(check_level == -1) check_level = this->level;

        if(check_level != this->level)
        {
            if(check_level < this->level)
            {
                return false;
            }
            else if(check_level > this->level)
            {
                for(auto& pair : *this)
                {
                    return pair.second.search_level(val, check_level + 1);
                }
            }
        }

        for(auto& item : values())
        {
            if(item == val)
            {
                return true;
            }
        }

        return false;

    }

    template<typename search_t>
    size_t count(const search_t& val)
    {
        size_t output = 0;

        if(level == val.size())
        {
            for(auto& item : values())
            {
                if(item == val)
                {
                    ++output;
                }
            }
        }
        else
        {
            try{
                output += this->at(val[level]).count(val);
            }catch(...){ }
        }

        return output;
    }

    template<typename search_t>
    void get_equal(const search_t& val,
                   std::vector<value_type>& output)
    {

        if(level == val.size())
        {
            for(auto& item : values())
            {
                if(item == val)
                {
                    output.push_back(item);
                }
            }
        }
        else
        {
            try{
                this->at(val[level]).get_equal(val, output);
            }catch(...){ }
        }

    }

    template<typename search_t>
    bool search(const search_t& val)
    {
        if((level == val.size()) ||
           (level == max_level))
        {
            for(auto& item : values())
            {
                if(item == val)
                {
                    return true;
                }
            }
        }
        else
        {
            try{
                return this->at(val[level]).search(val);
            }catch(...){ }
        }

        return false;
    }

    template<typename search_t>
    value_type& get(const search_t& val)
    {
        if((level == val.size()) ||
           (level == max_level))
        {
            for(auto& item : values())
            {
                if(item == val)
                {
                    return item;
                }
            }
        }
        else
        {
            try{
                return this->at(val[level]).get(val);
            }catch(...){ }
        }

        throw std::invalid_argument("Value does not exist in tree");
    }

    inline void assemble(container_t& V)
    {

        for(auto& element : V)
        {
            add_tree(element);
        }
    }

    inline friend std::ostream& operator<<(std::ostream& output,
                                    const tree_vector<key_t, value_type, container_t> input)
    {
        if(!input.level)
        {
            output << "TREE BEGIN\n";
        }

        if(!input.values().empty())
        {
            for(size_t i = 0; i < input.values().size(); ++i)
            {
                for(size_t j = 0; j < input.level; ++j)
                {
                    if(!j) output << "|--";
                    else output << "---";
                }
                output << "|+" << input.values()[i] << '\n';
            }
        }

        for(auto& pair : input)
        {
            output << "|";
            for(int i = 0; i < pair.second.level; ++i)
            {
                if(i < pair.second.level - 1) output << "---";
                else output << "--";
            }
            output << pair.first << '\n' << pair.second;
        }

        return output;
    }

    template<typename key_matrix_t>
    inline bool get_references(const key_matrix_t& key_matrix,
                         reference_vector<value_type>& output,
                         const int& min_size = -1,
                         const bool& unique_only = true)
    {

        if(!level)
        {
            output.clear();
        }
        else{

            if((min_size < 1) || (level >= min_size))
            {

                for(auto& val : values())
                {

                    if(output.empty())
                    {
                        output.emplace_back(val);
                    }
                    else
                    {
                        for(size_t i = 0; i < output.size(); ++i)
                        {
                            if(val < output[i])
                            {
                                output.insert(output.begin() + i, val);
                                goto inserted;
                            }
                            else if(unique_only && (val == output[i]))
                            {
                                goto inserted;
                            }
                        }

                        output.emplace_back(val);

                        inserted:;
                    }
                }

            }
        }

        if(level < key_matrix.size())
        {
            for(auto& c : key_matrix[level])
            {
                try{
                    this->at(c).get_references(key_matrix, output, min_size, unique_only);
                }catch(...){ }
            }
        }

        return !output.empty();
    }

    template<typename search_container_t>
    inline bool match_references(const search_container_t& query,
                                 reference_vector<value_type>& output)
    {

        size_t initSize = output.size();

        for(auto& item : query)
        {
            try
            {
                output.emplace_back(get(item));
            }catch(...){ }
        }

        return output.size() > initSize;

    }

//
//    template<typename key_vector_t>
//    get_values(const key_matrix_t& keys,
//               std::vector<value_type>& output) const
//    {
//        output.clear();
//
//        for(auto& pair : *this)
//        {
//            if(anyEqual(pair.first, keys))
//            {
//                append(output, pair.second);
//            }
//        }
//
//        return !output.empty();
//    }

protected:

    container_t vals;
    key_t key;

    int level;
    int max_level; // All terms larger than this level are binned by the last-level indexable member

};

}

#endif // TOOLKIT_CLUSTERED_VECTOR
