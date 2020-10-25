
#include "fmt/core.h"
#include <cassert>
#include <vector>
#include "algorithms/find.hpp"

std::vector<int> make_vector_01110()
{
    std::vector v{0,1,1,1,0};
    return v;
}

void test_finds_1s()
{
    auto v = make_vector_01110();
    auto predicate = [](int x) { return x == 1; };
    auto indices   = algorithm::find_indices(v, predicate);

    assert(indices.size() == 3);
    assert(indices[0] == 1);
    assert(indices[1] == 2);
    assert(indices[2] == 3);
}

void test_finds_0s()
{
    auto v         = make_vector_01110();
    auto predicate = [](int x) { return x == 0; };
    auto indices   = algorithm::find_indices(v, predicate);

    assert(indices.size() == 2);
    assert(indices[0] == 0);
    assert(indices[1] == 4);
}

void test_rfinds_1s()
{
    auto v         = make_vector_01110();
    auto predicate = [](int x) { return x == 1; };
    auto indices   = algorithm::rfind_indices(v, predicate);

    assert(indices.size() == 3);
    assert(indices[0] == 3);
    assert(indices[1] == 2);
    assert(indices[2] == 1);
}

int main()
{
    test_finds_1s();
    test_finds_0s();
    test_rfinds_1s();
    fmt::print("Test algorithm::find_indices complete.\n");
    return 0;
}
