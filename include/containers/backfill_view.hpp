#pragma once

#include <stdexcept>


template <typename _Tp, std::size_t _Nm>
struct backfill_view {
    typedef _Tp                     value_type;
    typedef value_type*             pointer;
    typedef value_type const*       const_pointer;
    typedef value_type const* const const_cpointer;
    typedef value_type&             reference;
    typedef const value_type&       const_reference;
    // typedef value_type*                           iterator;
    // typedef const value_type*                     const_iterator;
    typedef std::size_t size_type;
    // typedef std::ptrdiff_t                        difference_type;
    // typedef std::reverse_iterator<iterator>       reverse_iterator;
    // typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    backfill_view(const_cpointer begin)
        : a(begin)
        , b(begin + _Nm)
    {
        last = a;
    }

    //
    // Iterators.
    constexpr auto
    begin() noexcept
    {
        return a;
    }

    constexpr auto
    end() noexcept
    {
        return last;
    }

    // Capacity
    constexpr size_type
    size() const noexcept { return last - a; }

    constexpr size_type
    max_size() const noexcept { return _Nm; }

    [[nodiscard]] constexpr bool
    empty() const noexcept { return size() == 0; }

    //
    // Accessors.
    pointer at(std::size_t pos)
    {
        pointer x = a + pos;
        if (x >= b)
        {
            throw std::out_of_range("Index is out of bounds.");
        }

        return x;
    }

    pointer increase()
    {
        pointer x = ++last;
        if (x >= b)
        {
            throw std::out_of_range("Index is out of bounds.");
        }

        last = x;
        return last;
    }

    pointer reduce()
    {
        pointer x = --last;
        if (x >= b)
        {
            throw std::out_of_range("Index is out of bounds.");
        }

        last = x;
    }

    void remove(size_type pos)
    {
        pointer back = last;
        pointer x    = at(pos);

        x = std::swap(back);

        reduce();
    }

private:
    const_cpointer a;
    const_cpointer b;
    const_pointer  last;
};
