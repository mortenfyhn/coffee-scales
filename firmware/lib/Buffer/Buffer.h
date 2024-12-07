#pragma once

#include <stddef.h>  // size_t

namespace Buffer
{

// A buffer that takes care of itself so you can pass it around without worry.
// It's mutable, but it's not resizable.
// It's movable, but it's not copyable.
template <typename T> class buffer
{
  public:
    explicit buffer(size_t size);
    buffer(T const* input, size_t size);

    T const* get() const;
    T* data();
    size_t size() const;

    T& operator[](size_t pos);
    T const& operator[](size_t pos) const;

    buffer(buffer const&) = delete;
    buffer& operator=(buffer const&) = delete;

    buffer(buffer&&);
    buffer& operator=(buffer&&);

    ~buffer();

  private:
    T* data_;
    size_t const size_;
};

}  // namespace Buffer

#include "Buffer_impl.h"
