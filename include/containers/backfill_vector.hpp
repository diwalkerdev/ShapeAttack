#ifndef BACKFILL_VECTOR_HPP
#define BACKFILL_VECTOR_HPP

#include <stdexcept>
#include <vector>

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
        return vector.end();
    }

    constexpr auto
    end() const noexcept
    {
        return vector.cend();
    }

    // constexpr auto
    // rbegin() noexcept
    // {
    //     return reverse_iterator(end());
    // }

    // constexpr auto
    // rbegin() const noexcept
    // {
    //     return const_reverse_iterator(end());
    // }

    // constexpr auto
    // rend() noexcept
    // {
    //     return reverse_iterator(begin());
    // }

    // constexpr const_reverse_iterator
    // rend() const noexcept
    // {
    //     return const_reverse_iterator(begin());
    // }

    constexpr auto
    cbegin() const noexcept
    {
        return vector.cbegin();
    }

    constexpr auto
    cend() const noexcept
    {
        return vector.cend();
    }

    // constexpr const_reverse_iterator
    // crbegin() const noexcept
    // {
    //     return const_reverse_iterator(vector.end());
    // }

    // constexpr const_reverse_iterator
    // crend() const noexcept
    // {
    //     return const_reverse_iterator(vector.begin());
    // }

    // Capacity.
    constexpr size_type
    size() const noexcept { return vector.size(); }

    constexpr size_type
    max_size() const noexcept { return _Nm; }

    [[nodiscard]] constexpr bool
    empty() const noexcept { return size() == 0; }

    // Constructors.
    backfill_vector()
    {
        vector.reserve(_Nm);
    }

    // Accessors.
    reference at(std::size_t pos)
    {
        return vector.at(pos);
    }

    // Modifiers.
    void push_back(value_type const& value) noexcept(false)
    {
        if (vector.size() >= _Nm)
        {
            throw std::out_of_range("Push back exceeds size of backfill_vector.");
        }
        vector.push_back(value);
    }

    void push_back(value_type&& value) noexcept(false)
    {
        if (vector.size() >= _Nm)
        {
            throw std::out_of_range("Push back exceeds size of backfill_vector.");
        }
        vector.push_back(std::move(value));
    }

    void remove(size_type pos) noexcept(false)
    {
        auto& back = vector.back();
        auto& x    = vector.at(pos);

        x = std::move(back);

        vector.pop_back();
    }

private:
    std::vector<_Tp> vector;
};


#endif // BACKFILL_VECTOR_HPP
