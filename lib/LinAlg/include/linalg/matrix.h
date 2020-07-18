#include <stddef.h>
#include <memory>
#include <array>
#include <span>

namespace linalg {


template <typename Scalar, size_t Rows, size_t Cols>
struct Matrix
{
    static const size_t NumCols = Cols;
    static const size_t NumRows = Rows;

    using array2d = std::array<Scalar, Cols * Rows>; 
    array2d data;
    // std::shared_ptr<array2d> data = std::make_shared<array2d>();

    // Matrix() = default;
    // Matrix(Matrix<Scalar, Rows,Cols> const& other) = default;
    // Matrix(Matrix<Scalar, Rows,Cols> && other) = default;
    
    Matrix(array2d initializer): data(initializer)
    {
    }

    // static 
    // Matrix<Scalar, Rows, Cols> zero() 
    // {
    //     Matrix<Scalar, Rows, Cols> m;
    //     ::zero(m);
    //     return m;
    // }

    // static 
    // Matrix<Scalar, Rows, Cols> I() 
    // {
    //     static_assert(Rows == Cols, "Matrix is not square");
        
    //     Matrix<Scalar, Rows, Cols> m;
    //     ::zero(m);
    //     for (int i=0; i < Rows; ++i) {
    //         m[i][i] = 1;
    //     }

    //     return m;
    // }

    std::span<Scalar> operator[](size_t index) 
    {
        return {&data[index*Cols], Cols};
    }

    // Matrix<Scalar, Rows, Cols>& operator=(Matrix<Scalar, Rows, Cols> other)
    // {
    //     std::cout << "matrix matrix assign" << std::endl;
    //     this->data = std::make_shared<array2d>();

    //     for (size_t i = 0; i < Rows; ++i)
    //     {
    //         for (size_t j = 0; j < Cols; ++j)
    //         {
    //             (*data)[i][j] = (*other.data)[i][j];
    //         }
    //     }
    //     return *this;
    // }

    // Matrix<Scalar, Rows, Cols>& operator=(const std::array<Scalar, Rows*Cols> &other)
    // {
    //     std::cout << "matrix array assign" << std::endl;
    //     this->data = std::make_shared<array2d>();

    //     for (size_t i = 0; i < Rows; ++i)
    //     {
    //         for (size_t j = 0; j < Cols; ++j)
    //         {
    //             (*data)[i][j] = other[i*Cols + j];
    //         }
    //     }
    //     return (*this);
    // }
};

} // end namespace linalg

template <typename Scalar, size_t Rows, size_t Cols>
std::ostream& operator<<(std::ostream& os, linalg::Matrix<Scalar, Rows, Cols> &m)
{
    for (size_t r=0; r< Rows; ++r) {
        for (size_t c=0; c< Cols; ++c) {
            os << m[r][c] << " ";
        }
        os << std::endl;
    }
    return os;
}
