#pragma once

#include <array>
#include <stdexcept>

template <typename _Tp, std::size_t _Nm>
struct backfill_vector {
    typedef _Tp               value_type;
    typedef value_type*       pointer;
    typedef const value_type* const_pointer;
    typedef value_type&       reference;
    typedef const value_type& const_reference;
    // typedef value_type*                           iterator;
    // typedef const value_type*                     const_iterator;
    typedef std::size_t size_type;
    // typedef std::ptrdiff_t                        difference_type;
    // typedef std::reverse_iterator<iterator>       reverse_iterator;
    // typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

    // Iterators.
    constexpr auto
    begin() noexcept
    {
        return vector.begin();
    }

    constexpr auto
    begin() const noexcept
    {
        return vector.cbegin();
    }

    constexpr auto
    end() noexcept
    {
        return vector.begin() + last;
    }

    constexpr auto
    end() const noexcept
    {
        return vector.begin() + last;
    }

    constexpr auto
    back() noexcept
    {
        return vector.end();
    }

    constexpr auto
    back() const noexcept
    {
        return vector.end();
    }

    constexpr auto
    rbegin() noexcept
    {
        return vector.rbegin() + (_Nm - last);
    }

    constexpr auto
    rbegin() const noexcept
    {
        return vector.rbegin() + (_Nm - last);
    }

    constexpr auto
    rend() noexcept
    {
        return vector.rend();
    }

    constexpr auto
    rend() const noexcept
    {
        return vector.rend();
    }

    // constexpr auto
    // cbegin() const noexcept
    // {
    //     return vector.cbegin();
    // }

    // constexpr auto
    // cend() const noexcept
    // {
    //     return vector.cend();
    // }

    // constexpr auto
    // crbegin() const noexcept
    // {
    //     return vector.crbegin();
    // }

    // constexpr auto
    // crend() const noexcept
    // {
    //     return vector.crend();
    // }

    // Capacity.
    constexpr size_type
    size() const noexcept { return last; }

    constexpr size_type
    max_size() const noexcept { return _Nm; }

    [[nodiscard]] constexpr bool
    empty() const noexcept { return size() == 0; }

    // Constructors.
    // backfill_vector()
    // {
    // }

    // Accessors.
    reference at(std::size_t pos)
    {
        return vector.at(pos);
    }

    // Modifiers.
    // void push_back(value_type const& value) noexcept(false)
    // {

    //     if (vector.size() >= _Nm)
    //     {
    //         throw std::out_of_range("Push back exceeds size of backfill_vector.");
    //     }
    //     vector.push_back(value);
    // }

    // void push_back(value_type&& value) noexcept(false)
    // {
    //     if (vector.size() >= _Nm)
    //     {
    //         throw std::out_of_range("Push back exceeds size of backfill_vector.");
    //     }
    //     vector.push_back(std::move(value));
    // }
    reference increase()
    {
        auto next = last + 1;
        if (next > vector.size())
        {
            throw std::out_of_range("increase exceeds size of backfill_vector.");
        }
        reference x = at(last);
        last = next;
        return x;
    }

    void remove(size_type pos) noexcept(false)
    {
        using std::swap;
        auto& back = vector.at(last - 1);
        auto& x    = vector.at(pos);

        swap(x, back);
        last -= 1;
    }

private:
    std::array<_Tp, _Nm> vector;
    std::size_t          last{};
};

