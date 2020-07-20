#include "linalg/matrix.h"
#include "linalg/util.h"
#include "meliorate/meliorate.h"


void test_make_matrix()
{
    // code goes here.
    linalg::Matrix<float, 3, 2> A {{1.f, 2.f, 3.f, 4.f, 5.f, 6.f}};
    std::cout << A;
    // prints:
    // 1,2
    // 3,4
    // 5,6
}

// Not extracted as a test function because the function name
// does not begin with "test".
void test_matrix_multiplication()
{
    linalg::Matrixf<2, 2> A {{1.f, 2.f, 3.f, 4.f}};
    linalg::Matrixf<2, 2> B {{1.f, 2.f, 3.f, 4.f}};

    auto C = A * B;
    std::cout << C;
}

void test_rotations_vec()
{
    auto alpha = 2 * M_PI / 4;

    linalg::Matrixf<1, 2> A {{1.f, 0.f}};

    auto X = A * rrotzf(alpha);
    std::cout << A;
    std::cout << X;
}

void test_rotations_mat()
{
    auto alpha = 2 * M_PI / 4;

    // clang-format off
    linalg::Matrixf<3, 3> A {{
        1.f, 0.f, 1.f,
        0.f, 1.f, 1.f,
        0.f, 1.f, 1.f,
    }};
    // clang-format on

    auto X = A * rtransf(alpha, 0, 0);

    std::cout << A;
    std::cout << X;
}

void test_row_iter()
{
    linalg::Matrixf<2, 2> A {{1.f, 2.f, 3.f, 4.f}};
    std::cout << A;
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