#pragma once

#include <vector>
#include <algorithm>

namespace algorithm {


template <typename Tp, typename Fn>
auto find_indices(Tp const& container, Fn predicate) -> std::vector<std::size_t>
{
    std::vector<std::size_t> results;

    auto it = std::find_if(std::begin(container), std::end(container), predicate);
    while (it != std::end(container))
    {
        results.emplace_back(std::distance(std::begin(container), it));
        it = std::find_if(std::next(it), std::end(container), predicate);
    }
    return results;
}

template <typename Tp, typename Fn>
auto rfind_indices(Tp const& container, Fn predicate) -> std::vector<std::size_t>
{
    std::vector<std::size_t> results;

    auto it = std::find_if(std::rbegin(container), std::rend(container), predicate);
    while (it != std::rend(container))
    {
        results.emplace_back(std::distance(it, std::rend(container)) - 1);
        it = std::find_if(std::next(it), std::rend(container), predicate);
    }
    return results;
}

} // end namespace algorithm
