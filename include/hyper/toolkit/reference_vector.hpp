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

/////////////////////////////////////////////////////////////////////////////

/* Vector definition */

/////////////////////////////////////////////////////////////////////////////

template<class T> using refv_t = typename std::vector<std::reference_wrapper<T>>;

template<typename T>
class reference_vector : public refv_t<T>
{
public:

    template<typename container_t>
    reference_vector(container_t& other)
    {
        for(auto& item : other)
        {
            this->emplace_back(item);
        }
    }

    reference_vector() = default;

    class iterator_ptr
    {
    public:

        iterator_ptr(std::reference_wrapper<T>* ptr):
            _ptr(ptr){ }

        inline friend ptrdiff_t distance(const iterator_ptr& first,
                                  const iterator_ptr& second)
        {
            return std::distance(first._ptr, second._ptr);
        }

        bool operator==(const iterator_ptr& other) const{ return this->_ptr == other._ptr; }
        bool operator!=(const iterator_ptr& other) const{ return this->_ptr != other._ptr; }

    protected:
        std::reference_wrapper<T>* _ptr;
    };

    class iterator : public iterator_ptr
    {
    public:

        iterator(std::reference_wrapper<T>* ptr):
            iterator_ptr(ptr){ }

        void operator++(){ ++this->_ptr; }
        void operator--(){ --this->_ptr; }
        iterator operator++(int){ auto output(*this); ++output; return output; }
        iterator operator--(int){ auto output(*this); --output; return output; }

        void operator+=(const ptrdiff_t& movement){ this->_ptr += movement; }
        void operator-=(const ptrdiff_t& movement){ this->_ptr -= movement; }

        iterator operator+(const ptrdiff_t& movement) const{ auto output(*this); output += movement; return output; }
        iterator operator-(const ptrdiff_t& movement) const{ auto output(*this); output -= movement; return output; }

        T& operator*(){ return this->_ptr->get(); }
        std::reference_wrapper<T> operator->() const{ return *this->_ptr; }

        std::reference_wrapper<T>* getPtr() const{ return this->_ptr; }

    };

    class const_iterator : public iterator_ptr
    {
    public:

        const_iterator(std::reference_wrapper<T>* ptr):
           iterator_ptr(ptr){ }

        void operator++(){ ++this->_ptr; }
        void operator--(){ --this->_ptr; }
        const_iterator operator++(int){ auto output(*this); ++output; return output; }
        const_iterator operator--(int){ auto output(*this); --output; return output; }

        void operator+=(const ptrdiff_t& movement){ this->_ptr += movement; }
        void operator-=(const ptrdiff_t& movement){ this->_ptr -= movement; }

        const_iterator operator+(const ptrdiff_t& movement) const{ auto output(*this); output += movement; return output; }
        const_iterator operator-(const ptrdiff_t& movement) const{ auto output(*this); output -= movement; return output; }

        const T& operator*() const{ return this->_ptr->get(); }
        const std::reference_wrapper<T> operator->() const{ return *this->_ptr; }

        const std::reference_wrapper<T>* getPtr() const{ return this->_ptr; }
    };

    class reverse_iterator : public iterator_ptr
    {
    public:

        reverse_iterator(std::reference_wrapper<T>* ptr):
            iterator_ptr(ptr){ }

        void operator++(){ --this->_ptr; }
        void operator--(){ ++this->_ptr; }
        reverse_iterator operator++(int){ auto output(*this); --output; return output; }
        reverse_iterator operator--(int){ auto output(*this); ++output; return output; }

        void operator+=(const ptrdiff_t& movement){ this->_ptr -= movement; }
        void operator-=(const ptrdiff_t& movement){ this->_ptr += movement; }

        reverse_iterator operator+(const ptrdiff_t& movement) const{ auto output(*this); output += movement; return output; }
        reverse_iterator operator-(const ptrdiff_t& movement) const{ auto output(*this); output -= movement; return output; }

        T& operator*(){ return this->_ptr->get(); }
        std::reference_wrapper<T> operator->() const{ return *this->_ptr; }

        std::reference_wrapper<T>* getPtr() const{ return this->_ptr; }

    };

    class const_reverse_iterator : public iterator_ptr
    {
    public:

        const_reverse_iterator(std::reference_wrapper<T>* ptr):
            iterator_ptr(ptr){ }

        void operator++(){ --this->_ptr; }
        void operator--(){ ++this->_ptr; }
        const_reverse_iterator operator++(int){ auto output(*this); --output; return output; }
        const_reverse_iterator operator--(int){ auto output(*this); ++output; return output; }

        void operator+=(const ptrdiff_t& movement){ this->_ptr -= movement; }
        void operator-=(const ptrdiff_t& movement){ this->_ptr += movement; }

        const_reverse_iterator operator+(const ptrdiff_t& movement) const{ auto output(*this); output += movement; return output; }
        const_reverse_iterator operator-(const ptrdiff_t& movement) const{ auto output(*this); output -= movement; return output; }

        const T& operator*() const{ return this->_ptr->get(); }
        const std::reference_wrapper<T> operator->() const{ return *this->_ptr; }

        const std::reference_wrapper<T>* getPtr() const{ return this->_ptr; }

    };

    iterator                        begin()         { return iterator(this->data()); }
    iterator                        end()           { return iterator(this->data() + this->size()); }

    const_iterator                  begin()         const { return const_iterator(this->data()); }
    const_iterator                  end()           const { return const_iterator(this->data() + this->size()); }

    reverse_iterator                rbegin()        { return reverse_iterator(this->data() + this->size() - 1); }
    reverse_iterator                rend()          { return reverse_iterator(this->data() - 1); }

    const_reverse_iterator          rbegin()        const { return const_reverse_iterator(this->data() + this->size() - 1); }
    const_reverse_iterator          rend()          const { return const_reverse_iterator(this->data() - 1); }

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
    inline void insert(const iterator& position, value_type& val)
    {

        int dist = std::distance(position.getPtr(), rbegin().getPtr()) + 1;

        this->emplace_back(this->back());
        reverse_iterator it = rbegin();

        for(int i = 0; i < dist; ++i, ++it)
        {
            *(it.getPtr()) = *((it + 1).getPtr());
        }

        *(this->data() + this->size() - dist - 1) = std::reference_wrapper<value_type>(val);

    }

    inline void erase(const iterator& position)
    {

        refv_t<T>::erase(refv_t<T>::begin() + distance(begin(), position));

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
