// #include "linalg/matrix.hpp"


/*
void test_matrix_multiplication()
{
    linalg::Matrixf<2, 2> A{{1.f, 2.f, 3.f, 4.f}};
    linalg::Matrixf<2, 2> B{{1.f, 2.f, 3.f, 4.f}};

    auto C = A * B;
    std::cout << C;
}

void test_rotations_vec()
{
    auto alpha = 2 * M_PI / 4;

    linalg::Matrixf<1, 2> A{{1.f, 0.f}};

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
    linalg::Matrixf<2, 2> A{{1.f, 2.f, 3.f, 4.f}};
    std::cout << A;
}

// Not extracted as a test function because the function is static.
static void test_that_something_is_false()
{
    // code goes here.
}

*/

/*
int test_operators()
{
    const auto               A = linalg::Matrixf<2, 2>{{{1, 2}, {3, 4}}};
    const auto               B = linalg::Matrixf<2, 2>{{{1, 2}, {3, 4}}};
    const linalg::Vectorf<2> v{{1, 2}};

    // std::cout << "A: " << A;
    // std::cout << "v: " << v;
    // std::cout << "rvalue: " << linalg::Matrixf<2, 2>{{{1, 2}, {3, 4}}};

    fmt::print("A: {0}\n", A);
    fmt::print("v: {0}\n", v);
    fmt::print("rvalue: {0}", linalg::Matrixf<2, 2>{{{1, 2}, {3, 4}}});


    auto X = linalg::Matrixf<2, 2>{{{1, 2}, {3, 4}}};
    // fmt::print("rows {}  cols {}\n", X.rows(), X.cols());

    // Check can access const matrices.
    float x = A[0][0];

    //A[0] = 2; // cc error, A is const.

    {
        auto C = A * B;
        auto D = C * 2.f;
        auto E = 2.f * C;
        // E *= A; // currently undefined, ambiguous? do you point-wise or matrix mult?
        D *= 2.f;

        assert(A == A);

        // fmt::print("{0}\n", A);
    }

    {
        auto C = A + B;
        auto D = C + 2.f;
        auto E = 2.f + C;

        E += E += A;
        std::cout << C;
        std::cout << D;
        std::cout << E;
        D += 2.f;
        std::cout << D;
    }

    return 0;
}

int main()
{
    meliorate_stop_on_error = false;
    return meliorate_run<std::exception>();
}
*/
