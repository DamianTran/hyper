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

#ifndef TOOLKIT_REFERENCE_VECTOR
#define TOOLKIT_REFERENCE_VECTOR

#include <vector>
#include <functional>
#include <iostream>

/** ////////////////////////////////////////////////////////////////

    *** Reference vector class for more efficient random access

    *** MAKE SURE YOU UNDERSTAND HOW REFERENCES WORK:

    *** ENSURE THAT REFERENCE SUBSTRATES ARE NOT OTHERWISE MODIFIED
    *** (IE. DELETED OR MOVED) WHILE REFERENCE VECTOR IS VALID OR
    *** BEHAVIOUR WILL BE UNDEFINED

////////////////////////////////////////////////////////////////  **/

/////////////////////////////////////////////////////////////////////////////

/* Iterators */

/////////////////////////////////////////////////////////////////////////////

namespace hyperC
{

template<typename T>
class ref_iterator : public std::iterator<std::random_access_iterator_tag,
                                            std::reference_wrapper<T>,
                                            ptrdiff_t,
                                            std::reference_wrapper<T>*,
                                            std::reference_wrapper<T>&>
{
public:

    ref_iterator(std::reference_wrapper<T>* o_ptr = NULL):
        ptr(o_ptr){ }

    ref_iterator<T>&        operator=(const std::reference_wrapper<T>* o_ptr){ ptr = o_ptr; return *this; }

    operator                bool() const{ return ptr; }

    bool                    operator==(const ref_iterator<T>& other) const{ return ptr == other.ptr; }
    bool                    operator!=(const ref_iterator<T>& other) const{ return ptr != other.ptr; }

    bool                    operator==(const T* other) const{ return &ptr->get() == other; }
    bool                    operator!=(const T* other) const{ return &ptr->get() != other; }

    ref_iterator<T>&        operator+=(const ptrdiff_t& movement){ ptr += movement; return *this; }
    ref_iterator<T>&        operator-=(const ptrdiff_t& movement){ ptr -= movement; return *this; }
    ref_iterator<T>&        operator++(){ ++ptr; return *this; }
    ref_iterator<T>&        operator--(){ --ptr; return *this; }
    ref_iterator<T>         operator++(int){ auto cpy(*this); ++ptr; return cpy; }
    ref_iterator<T>         operator--(int){ auto cpy(*this); --ptr; return cpy; }
    ref_iterator<T>         operator+(const ptrdiff_t& movement) const{ auto other(*this); other += movement; return other; }
    ref_iterator<T>         operator-(const ptrdiff_t& movement) const{ auto other(*this); other -= movement; return other; }

    ptrdiff_t               operator-(const ref_iterator<T>& other) const{ return std::distance(other.ptr, ptr); }

    T&                      operator*(){ return ptr->get(); }
    const T&                operator*() const{ return ptr->get(); }

    std::reference_wrapper<T>*                  getPtr() const{ return ptr; }
    const std::reference_wrapper<T>*            getConstPtr() const{ return ptr; }

    friend std::ostream& operator<<(std::ostream& output, const ref_iterator<T>& itr)
    {
        return output << &itr.ptr->get();
    }

protected:

    std::reference_wrapper<T>* ptr;

};

template<typename T>
class ref_reverse_iterator : public ref_iterator<T>
{
public:

    ref_reverse_iterator(std::reference_wrapper<T>* o_ptr = NULL):
        ref_iterator<T>(o_ptr){ }

    ref_reverse_iterator<T>&        operator=(const std::reference_wrapper<T>* o_ptr){ this->ptr = o_ptr; return *this; }

    ref_reverse_iterator<T>&        operator+=(const ptrdiff_t& movement){ this->ptr -= movement; return *this; }
    ref_reverse_iterator<T>&        operator-=(const ptrdiff_t& movement){ this->ptr += movement; return *this; }
    ref_reverse_iterator<T>&        operator++(){ --this->ptr; return *this; }
    ref_reverse_iterator<T>&        operator--(){ ++this->ptr; return *this; }
    ref_reverse_iterator<T>         operator++(int){ auto cpy(*this); ++this->ptr; return cpy; }
    ref_reverse_iterator<T>         operator--(int){ auto cpy(*this); --this->ptr; return cpy; }
    ref_reverse_iterator<T>         operator+(const ptrdiff_t& movement) const{ auto other(*this); other += movement; return other; }
    ref_reverse_iterator<T>         operator-(const ptrdiff_t& movement) const{ auto other(*this); other -= movement; return other; }

    ptrdiff_t                       operator-(const ref_reverse_iterator<T>& other) const{ return std::distance(this->ptr, other.ptr); }

};

/////////////////////////////////////////////////////////////////////////////

/* Vector definition */

/////////////////////////////////////////////////////////////////////////////

template<class T> using refv_t = typename std::vector<std::reference_wrapper<T>>;

template<typename T>
class reference_vector : public refv_t<T>
{
public:

    typedef ref_iterator<T>                         iterator;
    typedef ref_reverse_iterator<T>                 reverse_iterator;

    template<template<typename> typename container_t>
    reference_vector(container_t<T>& other)
    {
        for(auto& item : other)
        {
            this->emplace_back(item);
        }
    }

    reference_vector() = default;

    iterator                        begin()         { return iterator(this->data()); }
    iterator                        end()           { return iterator(this->data() + this->size()); }

    reverse_iterator                rbegin()        { return reverse_iterator(this->data() + this->size() - 1); }
    reverse_iterator                rend()          { return reverse_iterator(this->data() - 1); }

    inline const T&                 operator[](const size_t& index) const
    {
        return refv_t<T>::operator[](index);
    }
    inline T&                       operator[](const size_t& index)
    {
        return refv_t<T>::operator[](index);
    }

    friend std::ostream& operator<<(std::ostream& output, const reference_vector<T>& input)
    {
        output << '[';
        for(size_t i = 0; i < input.size(); ++i)
        {
            output << input[i];
            if(i < input.size() - 1)
            {
                output << ',';
            }
        }
        output << ']';
        return output;
    }

    template<typename value_type>
    inline void insert(const ref_iterator<T>& position, value_type& val)
    {

        int dist = std::distance(position.getPtr(), rbegin().getPtr()) + 1;

        this->emplace_back(this->back());
        ref_reverse_iterator<T> it = rbegin();

        for(int i = 0; i < dist; ++i, ++it)
        {
            *(it.getPtr()) = *((it + 1).getPtr());
        }

        *(this->data() + this->size() - dist - 1) = std::reference_wrapper<value_type>(val);

    }

    inline void erase(const ref_iterator<T>& position)
    {
        refv_t<T>::erase(refv_t<T>::begin() + std::distance(begin(), position));
    }

    inline void push_back(T& newItem)
    {
        push_back(std::ref(newItem));
    }

    template<typename iterator_t>
    void append(iterator_t begin,
               const iterator_t& end)
    {
        while(begin != end)
        {
            this->emplace_back(*begin);
            ++begin;
        }
    }

    template<typename container_t>
    void append(container_t& other)
    {
        append(other.begin(), other.end());
    }

    template<template<typename> typename container_t, typename object_t>
    reference_vector<T>& operator=(container_t<object_t>& other)
    {
        this->clear();
        for(auto& item : other)
        {
            this->emplace_back(item);
        }
    }

    inline void swap(const size_t& first, const size_t& second)
    {
        std::reference_wrapper<T> tmp(*(this->data() + first));
        *(this->data() + first) = *(this->data() + second);
        *(this->data() + second) = tmp;
    }

    inline T& back()
    {
        return refv_t<T>::back().get();
    }
    inline T& back() const
    {
        return refv_t<T>::back().get();
    }

    inline T& front()
    {
        return refv_t<T>::front().get();
    }
    inline T& front() const
    {
        return refv_t<T>::front().get();
    }

};

}

#endif // TOOLKIT_REFERENCE_VECTOR
