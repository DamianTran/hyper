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

#ifndef HYPER_RECURSIVE_VECTOR
#define HYPER_RECURSIVE_VECTOR

template<typename T>
class recursive_vector
{
public:

    recursive_vector() = default;
    recursive_vector(const std::vector<size_t>& dimensions):
        __level(dimensions.size())
    {
        if(dimensions.size() > 1)
        {
            std::vector<size_t> sub_branch(dimensions.begin() + 1, dimensions.end());
            for(size_t i = 0; i < dimensions.front(); ++i)
            {
                branches.emplace_back(sub_branch);
            }
        }
        else
        {
            for(size_t i = 0; i < dimensions.front(); ++i)
            {
                branches.emplace_back();
            }
        }
    }

    template<typename other_t>
    void operator+=(const other_t& other)
    {
        if(isBranch())
        {
            for(auto& branch : branches)
            {
                branch += other;
            }
        }
        else
        {
            value += other;
        }
    }

    inline bool isNode() const noexcept{ return branches.empty(); }
    inline bool isBranch() const noexcept{ return !branches.empty(); }

protected:

    std::vector<recursive_vector<T>> branches;
    T value;

private:

    int __level;

};

#endif // HYPER_RECURSIVE_VECTOR
