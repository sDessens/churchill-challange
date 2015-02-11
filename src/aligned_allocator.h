#ifndef ALIGNED_ALLOCATOR_H
#define ALIGNED_ALLOCATOR_H

#ifdef _WIN32
#include <malloc.h>
#endif
#include <cstdint>
#include <stdexcept>
#include <limits>

template <typename T, std::size_t Alignment>
class aligned_allocator
{
public:
    typedef T       *pointer;
    typedef const T *const_pointer;
    typedef T       &reference;
    typedef const T &const_reference;
    typedef T       value_type;
    typedef std::size_t size_type;
    typedef ptrdiff_t difference_type;

    T *address(T& r) const
    {
        return &r;
    }

    const T *address(const T &s) const
    {
        return &s;
    }

    std::size_t max_size() const
    {
        return std::numeric_limits<size_type>::max() / sizeof(T);
    }

    template <typename U>
    struct rebind
    {
        typedef aligned_allocator<U, Alignment> other;
    };

    void construct(T *const p, const T &t) const
    {
        new (static_cast<void *const>(p)) T(t);
    }

    void destroy(T *const p) const
    {
        p->~T();
    }

    bool operator==(const aligned_allocator &) const
    {
        return true;
    }

    // NOTE: the standard mandates that allocate throws an exception if it fails for whatever reason
    // but we compile with exceptions disabled, we ignore these cases and let the application crash
    // if malloc fails for whatever reason.
    T *allocate(const std::size_t n) const
    {
        if (n == 0) {
            return NULL;
        }

        return static_cast<T *>(_mm_malloc(n * sizeof(T), Alignment));
    }

    void deallocate(T *const p, const std::size_t) const
    {
        _mm_free(p);
    }

    template <typename U>
    T * allocate(const std::size_t n, const U * /* const hint */) const
    {
        return allocate(n);
    }
};

#endif // ALIGNED_ALLOCATOR_H
