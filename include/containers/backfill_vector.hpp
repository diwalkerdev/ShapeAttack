#ifndef TAILFILL_VECTOR_HPP
#define TAILFILL_VECTOR_HPP 

#include <vector>
#include <stdexcept>

template <typename _Tp, std::size_t _Nm>
struct backfill_vector 
{
    typedef _Tp 	    			      value_type;
    typedef value_type*			      pointer;
    typedef const value_type*                       const_pointer;
    typedef value_type&                   	      reference;
    typedef const value_type&             	      const_reference;
    typedef value_type*          		      iterator;
    typedef const value_type*			      const_iterator;
    typedef std::size_t                    	      size_type;
    typedef std::ptrdiff_t                   	      difference_type;
    typedef std::reverse_iterator<iterator>	      reverse_iterator;
    typedef std::reverse_iterator<const_iterator>   const_reverse_iterator;

    // Iterators.
    constexpr iterator
    begin() noexcept
    { return iterator(vector.begin()); }

    constexpr const_iterator
    begin() const noexcept
    { return const_iterator(vector.cbegin()); }

    constexpr iterator
    end() noexcept
    { return iterator(vector.end()); }

    constexpr const_iterator
    end() const noexcept
    { return const_iterator(vector.end()); }

    constexpr reverse_iterator
    rbegin() noexcept
    { return reverse_iterator(end()); }

    constexpr const_reverse_iterator
    rbegin() const noexcept
    { return const_reverse_iterator(end()); }

    constexpr reverse_iterator
    rend() noexcept
    { return reverse_iterator(begin()); }

    constexpr const_reverse_iterator
    rend() const noexcept
    { return const_reverse_iterator(begin()); }

    constexpr const_iterator
    cbegin() const noexcept
    { return const_iterator(vector.cbegin()); }

    constexpr const_iterator
    cend() const noexcept
    { return const_iterator(vector.cend()); }

    constexpr const_reverse_iterator
    crbegin() const noexcept
    { return const_reverse_iterator(vector.end()); }

    constexpr const_reverse_iterator
    crend() const noexcept
    { return const_reverse_iterator(vector.begin()); }

    // Capacity.
    constexpr size_type
    size() const noexcept { return _Nm; }

    constexpr size_type
    max_size() const noexcept { return _Nm; }

    [[nodiscard]] constexpr bool
    empty() const noexcept { return size() == 0; }

    // Constructors.
    backfill_vector() {
        vector.reserve(_Nm);
    }
    
    // Modifiers.
    void push_back(value_type const& value) noexcept(false)
    {
        if (vector.size() >= _Nm) {
            throw std::overflow_error();
        }
        vector.push_back(value);
    }

    void push_back(value_type && value) noexcept(false)
    {
        if (vector.size() >= _Nm) {
            throw std::overflow_error();
        }
        vector.push_back(std::move(value));
    }

    void remove(size_type pos) noexcept(false)
    {
        auto *end = vector.end();
        auto *x = vector.at(pos);
        *x = std::move(*end);
        vector.pop_back();
    }
    
private:
    std::vector<_Tp> vector;
};


#endif
