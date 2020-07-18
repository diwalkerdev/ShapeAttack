#include "meliorate/meliorate.h"
#include "linalg/matrix.h"


void test_make_matrix()
{
    // code goes here.
    linalg::Matrix<float, 3, 2> A{{1.f, 2.f, 3.f, 4.f, 5.f, 6.f}};
    std::cout << A;
    // prints:
    // 1,2
    // 3,4
    // 5,6
}

// Not extracted as a test function because the function name
// does not begin with "test".
int not_a_test_function()
{
    return 0;
}

void test_that_error_is_thrown()
{
    throw std::runtime_error("Something went wrong.");
}


void test_that_the_result_is_42()
{
    // code goes here.
}

// Not extracted as a test function because the function is static.
static void test_that_something_is_false()
{
    // code goes here.
}


int main()
{
    meliorate_stop_on_error = false;
    return meliorate_run<std::exception>();
}