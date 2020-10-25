#include "containers/backfill_vector.hpp"
#include "fmt/core.h"
#include <cassert>


void test_starts_empty()
{
    backfill_vector<int, 5> bfv;
    assert(bfv.size() == 0);
}

void test_size_increases_when_items_pushed()
{
    backfill_vector<int, 5> bfv;

    bfv.push_back(0);
    assert(bfv.size() == 1);

    bfv.push_back(0);
    bfv.push_back(0);
    bfv.push_back(0);
    bfv.push_back(0);
    assert(bfv.size() == 5);
}

auto make_bfv_12345() -> backfill_vector<int, 5>
{
    backfill_vector<int, 5> bfv;
    bfv.push_back(1);
    bfv.push_back(2);
    bfv.push_back(3);
    bfv.push_back(4);
    bfv.push_back(5);
    return bfv;
}

void test_size_decreases_when_items_removed()
{
    auto bfv = make_bfv_12345();

    bfv.remove(4);
    assert(bfv.size() == 4);
    bfv.remove(3);
    assert(bfv.size() == 3);
    bfv.remove(2);
    assert(bfv.size() == 2);
    bfv.remove(1);
    assert(bfv.size() == 1);
    bfv.remove(0);
    assert(bfv.size() == 0);
}


void test_vector_back_fills_when_items_removed()
{
    auto bfv = make_bfv_12345();

    // Perform some basic checks on the iterators.
    assert(bfv.at(4) == 5);
    assert((*(bfv.end() - 1)) == 5);
    assert((*bfv.begin()) == 1);

    // 1234X
    // 1234
    bfv.remove(4);
    assert(bfv.at(3) == 4);

    // 1X3(4)
    // 143
    bfv.remove(1);
    assert(bfv.at(1) == 4);

    // X4(3)
    // 34
    bfv.remove(0);
    assert(bfv.at(0) == 3);

    // X(4)
    // 4
    bfv.remove(0);
    assert(bfv.at(0) == 4);
}

/*
int main()
{
    test_starts_empty();
    test_size_increases_when_items_pushed();
    test_size_decreases_when_items_removed();
    test_vector_back_fills_when_items_removed();

    fmt::print("Test backfill vector complete.\n");
    return 0;
}
*/
