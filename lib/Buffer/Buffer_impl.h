#pragma once

#include <string.h>  // memset, memcpy

namespace Buffer
{

template <typename T> buffer<T>::buffer(size_t size) : size_(size)
{
    data_ = new T[size_];
    memset(data_, 0, size_);
}

template <typename T>
buffer<T>::buffer(T const* input, size_t size) : size_(size)
{
    data_ = new T[size_];
    memcpy(data_, input, size_);
}

template <typename T> T const* buffer<T>::get() const
{
    return data_;
}

template <typename T> T* buffer<T>::data()
{
    return data_;
}

template <typename T> size_t buffer<T>::size() const
{
    return size_;
}

template <typename T> T& buffer<T>::operator[](size_t pos)
{
    return data_[pos];
}

template <typename T> T const& buffer<T>::operator[](size_t pos) const
{
    return data_[pos];
}

template <typename T>
buffer<T>::buffer(buffer<T>&& that) : data_(that.data_), size_(that.size_)
{
    that.data_ = nullptr;
}

template <typename T> buffer<T>& buffer<T>::operator=(buffer<T>&& that)
{
    if (this != &that)
    {
        delete[] data_;
        data_ = that.data_;
        that.data_ = nullptr;
    }

    return *this;
}

template <typename T> buffer<T>::~buffer()
{
    if (data_ != nullptr)
        delete[] data_;
}

}  // namespace Buffer
