#pragma once

#ifndef HYPER_STATIC_VECTOR
#define HYPER_STATIC_VECTOR

#include <vector>
#include <iostream>

namespace hyperC
{

/** Iterator class for static vector container */

template<typename T>
class ptr_iterator : public std::iterator<std::random_access_iterator_tag,
                                            T*,
                                            ptrdiff_t,
                                            T**,
                                            T&>
{
public:

    ptr_iterator(T** o_ptr = NULL):
        ptr(o_ptr){ }

    ptr_iterator<T>&        operator=(const T* o_ptr){ ptr = o_ptr; return *this; }

    operator                bool() const{ return ptr; }

    bool                    operator==(const ptr_iterator<T>& other) const{ return ptr == other.ptr; }
    bool                    operator!=(const ptr_iterator<T>& other) const{ return ptr != other.ptr; }

    bool                    operator==(const T* other) const{ return *ptr == other; }
    bool                    operator!=(const T* other) const{ return *ptr != other; }

    ptr_iterator<T>&        operator+=(const ptrdiff_t& movement){ ptr += movement; return *this; }
    ptr_iterator<T>&        operator-=(const ptrdiff_t& movement){ ptr -= movement; return *this; }
    ptr_iterator<T>&        operator++(){ ++ptr; return *this; }
    ptr_iterator<T>&        operator--(){ --ptr; return *this; }
    ptr_iterator<T>         operator++(int){ auto cpy(*this); ++ptr; return cpy; }
    ptr_iterator<T>         operator--(int){ auto cpy(*this); --ptr; return cpy; }
    ptr_iterator<T>         operator+(const ptrdiff_t& movement) const{ auto other(*this); other += movement; return other; }
    ptr_iterator<T>         operator-(const ptrdiff_t& movement) const{ auto other(*this); other -= movement; return other; }

    ptrdiff_t               operator-(const ptr_iterator<T>& other) const{ return std::distance(other.ptr, ptr); }

    T&                      operator*(){ return **ptr; }
    const T&                operator*() const{ return **ptr; }

    T**                     getPtr() const{ return ptr; }
    const T**               getConstPtr() const{ return ptr; }

    friend std::ostream& operator<<(std::ostream& output, const ptr_iterator<T>& itr)
    {
        return output << **itr.ptr;
    }

protected:

    T** ptr;

};

/** Reverse static vector iterator */

template<typename T>
class ptr_reverse_iterator : public ptr_iterator<T>
{
public:

    ptr_reverse_iterator(T* o_ptr = NULL):
        ptr_iterator<T>(o_ptr){ }

    ptr_reverse_iterator<T>&        operator=(const T* o_ptr){ this->ptr = o_ptr; return *this; }

    ptr_reverse_iterator<T>&        operator+=(const ptrdiff_t& movement){ this->ptr -= movement; return *this; }
    ptr_reverse_iterator<T>&        operator-=(const ptrdiff_t& movement){ this->ptr += movement; return *this; }
    ptr_reverse_iterator<T>&        operator++(){ --this->ptr; return *this; }
    ptr_reverse_iterator<T>&        operator--(){ ++this->ptr; return *this; }
    ptr_reverse_iterator<T>         operator++(int){ auto cpy(*this); ++this->ptr; return cpy; }
    ptr_reverse_iterator<T>         operator--(int){ auto cpy(*this); --this->ptr; return cpy; }
    ptr_reverse_iterator<T>         operator+(const ptrdiff_t& movement) const{ auto other(*this); other += movement; return other; }
    ptr_reverse_iterator<T>         operator-(const ptrdiff_t& movement) const{ auto other(*this); other -= movement; return other; }

    ptrdiff_t                       operator-(const ptr_reverse_iterator<T>& other) const{ return std::distance(this->ptr, other.ptr); }

};

/** STL-style container for stationary memory assignment.  Container items
    will remain in place even during buffer reallocation.  This is accomplished
    by storing pointers to new items in non-contiguous memory space.  Pointers
    are automatically cleaned up after the end of the lifetime of the container. */

template<class value_type>
class static_vector : public std::vector<value_type*>
{
public:

    static_vector(){ }
    static_vector(std::initializer_list<value_type>& list)
    {
        reserve(list.size());
        for(auto& item : list)
        {
            emplace_back(new value_type(item));
        }
    }
    static_vector(const static_vector& other)
    {
        reserve(other.size());
        for(auto& item : other)
        {
            emplace_back(new value_type(other));
        }
    }

    inline static_vector& operator=(const static_vector& other)
    {
        reserve(other.size());
        for(auto& item : other)
        {
            emplace_back(new value_type(other));
        }
    }

    ~static_vector()
    {
        for(size_t i = 0; i < this->size(); ++i)
        {
            delete(*(this->data() + i));
        }
    }

    typedef ptr_iterator<value_type>                         iterator;
    typedef ptr_iterator<const value_type>                   const_iterator;
    typedef ptr_reverse_iterator<value_type>                 reverse_iterator;
    typedef ptr_reverse_iterator<const value_type>           const_reverse_iterator;

    iterator                        begin()             { return iterator(this->data()); }
    const_iterator                  begin()       const { return const_iterator(this->data()); }
    iterator                        end()               { return iterator(this->data() + this->size()); }
    const_iterator                  end()         const { return const_iterator(this->data() + this->size()); }

    reverse_iterator                rbegin()            { return reverse_iterator(this->data() + this->size() - 1); }
    const_reverse_iterator          rbegin()      const { return const_reverse_iterator(this->data() + this->size() - 1); }
    reverse_iterator                rend()              { return reverse_iterator(this->data() - 1); }
    const_reverse_iterator          rend()        const { return const_reverse_iterator(this->data() - 1); }

    inline value_type& operator[](const size_t& index)
    {
        return *std::vector<value_type*>::operator[](index);
    }

    inline const value_type& operator[](const size_t& index) const
    {
        return *std::vector<value_type*>::operator[](index);
    }

    inline value_type& front()
    {
        return *std::vector<value_type*>::front();
    }

    inline const value_type& front() const
    {
        return *std::vector<value_type*>::front();
    }

    inline value_type& back()
    {
        return *std::vector<value_type*>::back();
    }

    inline const value_type& back() const
    {
        return *std::vector<value_type*>::back();
    }

    inline void push_back(const value_type& other)
    {
        std::vector<value_type*>::push_back(new value_type(other));
    }

    template<class... Args>
    void emplace_back(Args&&... args){
        std::vector<value_type*>::push_back(new value_type(std::forward<Args>(args)...));
    }

    template<typename o_value_type>
    inline void insert(const ptr_iterator<value_type>& position, o_value_type& val)
    {

        int dist = std::distance(position.getPtr(), rbegin().getPtr()) + 1;

        this->emplace_back(this->back());
        ptr_reverse_iterator<value_type> it = rbegin();

        for(int i = 0; i < dist; ++i, ++it)
        {
            *(it.getPtr()) = *((it + 1).getPtr());
        }

        *(this->data() + this->size() - dist - 1) = new value_type(val);

    }

    inline void erase(const ptr_iterator<value_type>& position)
    {
        delete(*position.getPtr());
        std::vector<value_type*>::erase(std::vector<value_type*>::begin() + std::distance(begin(), position));
    }

    inline void clear()
    {
        for(size_t i = 0; i < this->size(); ++i)
        {
            delete(*(this->data() + i));
        }
        std::vector<value_type*>::clear();
    }

};

}

#endif // HYPER_STATIC_VECTOR
